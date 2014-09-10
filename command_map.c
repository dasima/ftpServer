#include "command_map.h"
#include "common.h"
#include "session.h"
#include "sysutil.h"
#include "ftp_codes.h"
#include "configure.h"
#include "trans_data.h"
#include "priv_sock.h"


typedef struct ftpcmd
{
    const char *cmd; //FTP指令
    void (*cmd_handler)(Session_t *sess);//该指令所对应的执行函数
} ftpcmd_t;

static ftpcmd_t ctrl_cmds[] = {
    /* 访问控制命令 */
    {"USER", do_user },
    {"PASS", do_pass },
    {"CWD", do_cwd },
    {"XCWD", do_cwd },
    {"CDUP", do_cdup },
    {"XCUP", do_cdup },
    {"QUIT", do_quit },
    {"ACCT", NULL },
    {"SMNT", NULL },
    {"REIN", NULL },
    /* 传输参数命令 */
    {"PORT", do_port },
    {"PASV", do_pasv },
    {"TYPE", do_type },
    {"STRU", do_stru },
    {"MODE", do_mode },

    /* 服务命令 */
    {"RETR", do_retr },
    {"STOR", do_stor },
    {"APPE", do_appe },
    {"LIST", do_list },
    {"NLST", do_nlst },
    {"REST", do_rest },
    {"ABOR", do_abor },
    {"\377\364\377\362ABOR", do_abor},
    {"PWD", do_pwd },
    {"XPWD", do_pwd },
    {"MKD", do_mkd },
    {"XMKD", do_mkd },
    {"RMD", do_rmd },
    {"XRMD", do_rmd },
    {"DELE", do_dele },
    {"RNFR", do_rnfr },
    {"RNTO", do_rnto },
    {"SITE", do_site },
    {"SYST", do_syst },
    {"FEAT", do_feat },
    {"SIZE", do_size },
    {"STAT", do_stat },
    {"NOOP", do_noop },
    {"HELP", do_help },
    {"STOU", NULL },
    {"ALLO", NULL }
};

//进行命令映射
void do_command_map(Session_t *sess)
{
    int i;
    int size = sizeof(ctrl_cmds) / sizeof(ctrl_cmds[0]); //数组大小
    for (i=0; i<size; ++i)
    {
        if (strcmp(ctrl_cmds[i].cmd, sess->com) == 0)
        {
            if (ctrl_cmds[i].cmd_handler != NULL)
            {
                ctrl_cmds[i].cmd_handler(sess);
            }
            else
            {
                //该命令没有实现
                ftp_reply(sess, FTP_COMMANDNOTIMPL, "Unimplement command.");
            }

            break;
        }
    }

    if (i == size)
    {
        //未识别的命令
        ftp_reply(sess, FTP_BADCMD, "Unknown command.");
    }
}

void ftp_reply(Session_t *sess, int status, const char *text)
{
    char tmp[1024] = { 0 };
    snprintf(tmp, sizeof tmp, "%d %s\r\n", status, text);
    writen(sess->peer_fd, tmp, strlen(tmp));
}

void ftp_lreply(Session_t *sess, int status, const char *text)
{
    char tmp[1024] = { 0 };
    snprintf(tmp, sizeof tmp, "%d-%s\r\n", status, text);
    writen(sess->peer_fd, tmp, strlen(tmp));
}

void do_user(Session_t *sess)
{
    struct passwd *pw;
    if((pw = getpwnam(sess->args)) == NULL)
    {
        ftp_reply(sess, FTP_LOGINERR, "Login incorrect.");
        return;
    }

    sess->user_uid = pw->pw_uid;
    ftp_reply(sess, FTP_GIVEPWORD, "Please specify the password.");
}

void do_pass(Session_t *sess)
{
    //struct passwd *getpwuid(uid_t uid)
    struct passwd *pw;
    if((pw = getpwuid(sess->user_uid)) == NULL)
    {
        ftp_reply(sess, FTP_LOGINERR, "Login incorrect.");
        return;
    }

    //struct spwd *getspnam(const char *name);
    struct spwd *spw;
    if((spw = getspnam(pw->pw_name)) == NULL)
    {
        ftp_reply(sess, FTP_LOGINERR, "Login incorrect.");
        return;
    }

    //char *crypt(const char *key, const char *salt);
    char *encrypted_password = crypt(sess->args, spw->sp_pwdp);
    if(strcmp(encrypted_password, spw->sp_pwdp) != 0)
    {
        ftp_reply(sess, FTP_LOGINERR, "Login incorrect.");
        return;
    }

    if(setegid(pw->pw_gid) == -1)
        ERR_EXIT("setegid");
    if(seteuid(pw->pw_uid) == -1)
        ERR_EXIT("seteuid");

    //home---切换到主目录
    if(chdir(pw->pw_dir) == -1)
        ERR_EXIT("chdir");
    //umask
    umask(tunable_local_umask);

    strcpy(sess->username, pw->pw_name);

    ftp_reply(sess, FTP_LOGINOK, "Login successful.");
}


void do_cwd(Session_t *sess)
{
    if(chdir(sess->args) == -1)
    {
        //550
        ftp_reply(sess, FTP_FILEFAIL, "Failed to change directory.");
        return;
    }

    //250 Directory successfully changed.
    ftp_reply(sess, FTP_CWDOK, "Directory successfully changed.");
}

void do_cdup(Session_t *sess)
{
    if(chdir("..") == -1)
    {
        //550
        ftp_reply(sess, FTP_FILEFAIL, "Failed to change directory.");
        return;
    }

    //250 Directory successfully changed.
    ftp_reply(sess, FTP_CWDOK, "Directory successfully changed.");
}

void do_quit(Session_t *sess)
{
    ftp_reply(sess, FTP_GOODBYE, "Good Bye!");
    exit(EXIT_SUCCESS);
}

void do_port(Session_t *sess)
{
    //设置主动工作模式
    //PORT 192,168,44,1,200,174
    unsigned int v[6] = {0};
    sscanf(sess->args, "%u,%u,%u,%u,%u,%u", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5]);

    sess->p_addr = (struct sockaddr_in *)malloc(sizeof (struct sockaddr_in));
    memset(sess->p_addr, 0, sizeof(struct sockaddr_in));
    sess->p_addr->sin_family = AF_INET;

    char *p = (char*)&sess->p_addr->sin_port;
    p[0] = v[4];
    p[1] = v[5];

    p = (char*)&sess->p_addr->sin_addr.s_addr;
    p[0] = v[0];
    p[1] = v[1];
    p[2] = v[2];
    p[3] = v[3];

    ftp_reply(sess, FTP_PORTOK, "PORT command successful. Consider using PASV.");
}

void do_pasv(Session_t *sess)
{
    char ip[16] = {0};
    get_local_ip(ip);

    //给nobody发送命令
    priv_sock_send_cmd(sess->proto_fd, PRIV_SOCK_PASV_LISTEN);
    //接收nobody的应答
    char res = priv_sock_recv_result(sess->proto_fd);
    if(res == PRIV_SOCK_RESULT_BAD)
    {
        ftp_reply(sess, FTP_BADCMD, "get listenfd error");
        return;
    }
    //接收port
    uint16_t port = priv_sock_recv_int(sess->proto_fd);


    //227 Entering Passive Mode (192,168,44,136,194,6).
    unsigned int v[6];
    sscanf(ip, "%u.%u.%u.%u", &v[0], &v[1], &v[2], &v[3]);
    uint16_t net_endian_port = htons(port); //网络字节序
    unsigned char *p = (unsigned char*)&net_endian_port;
    v[4] = p[0];
    v[5] = p[1];

    char text[1024] = {0};
    snprintf(text, sizeof text, "Entering Passive Mode (%u,%u,%u,%u,%u,%u).", v[0], v[1], v[2], v[3], v[4], v[5]);

    ftp_reply(sess, FTP_PASVOK, text);
}

void do_type(Session_t *sess)
{
    //指定FTP的传输模式
    if (strcmp(sess->args, "A") == 0)
    {
        sess->ascii_mode = 1;
        ftp_reply(sess, FTP_TYPEOK, "Switching to ASCII mode.");
    }
    else if (strcmp(sess->args, "I") == 0)
    {
        sess->ascii_mode = 0;
        ftp_reply(sess, FTP_TYPEOK, "Switching to Binary mode.");
    }
    else
    {
        ftp_reply(sess, FTP_BADCMD, "Unrecognised TYPE command.");
    }
}

void do_stru(Session_t *sess)
{

}

void do_mode(Session_t *sess)
{

}

void do_retr(Session_t *sess)
{
    download_file(sess);
}

void do_stor(Session_t *sess)
{
    upload_file(sess, 0);
}

void do_appe(Session_t *sess)
{
    upload_file(sess, 1);
}

void do_list(Session_t *sess)
{
    trans_list(sess, 1);
}

//实现简单的目录传输
void do_nlst(Session_t *sess)
{
    trans_list(sess, 0);
}

void do_rest(Session_t *sess)
{
    sess->restart_pos = atoll(sess->args);
    //Restart position accepted (344545).
    char text[1024] = {0};
    snprintf(text, sizeof text, "Restart position accepted (%lld).", sess->restart_pos);
    ftp_reply(sess, FTP_RESTOK, text);
}

void do_abor(Session_t *sess)
{
    //225 No transfer to ABOR
    ftp_reply(sess, FTP_ABOR_NOCONN, "No transfer to ABOR.");
}

void do_pwd(Session_t *sess)
{
    char tmp[1024] = {0};
    if(getcwd(tmp, sizeof tmp) == NULL)
    {
        //return值为-1/0，函数进入系统内核，
        //返回值判断用perror
        //返回值为NULL,不用perror，fprintf(stderr, "a");
        fprintf(stderr, "get cwd error\n");
        ftp_reply(sess, FTP_BADMODE, "error");
        return;
    }
    char text[1024] = {0};
    snprintf(text, sizeof text, "\"%s\"", tmp);
    ftp_reply(sess, FTP_PWDOK, text);
}

void do_mkd(Session_t *sess)
{
    if(mkdir(sess->args, 0777) == -1)
    {
        ftp_reply(sess, FTP_FILEFAIL, "Create directory operation failed.");
        return;
    }

    char text[1024] = {0};
    if(sess->args[0] == '/') //绝对路径
        snprintf(text, sizeof text, "%s created.", sess->args);
    else
    {
        //char *getcwd(char *buf, size_t size);
        char tmp[1024] = {0};
        if(getcwd(tmp, sizeof tmp) == NULL)
        {
            ERR_EXIT("getcwd");
        }
        snprintf(text, sizeof text, "%s/%s created.", tmp, sess->args);
    }

    ftp_reply(sess, FTP_MKDIROK, text);
}

void do_rmd(Session_t *sess)
{
    if(rmdir(sess->args) == -1)
    {
        //550 Remove directory operation failed.
        ftp_reply(sess, FTP_FILEFAIL, "Remove directory operation failed.");
        return;
    }
    //250 Remove directory operation successful.
    ftp_reply(sess, FTP_RMDIROK, "Remove directory operation successful.");
}

void do_dele(Session_t *sess)
{
    if(unlink(sess->args) == -1)
    {
        //550 Delete operation failed.
        ftp_reply(sess, FTP_FILEFAIL, "Delete operation failed.");
        return;
    }
    //250 Delete operation successful.
    ftp_reply(sess, FTP_DELEOK, "Delete operation successful.");
}

void do_rnfr(Session_t *sess)
{
    if(sess->rnfr_name) //防止内存泄露
    {
        free(sess->rnfr_name);
        sess->rnfr_name = NULL;
    }
    sess->rnfr_name = (char*)malloc(strlen(sess->args)+1);
    strcpy(sess->rnfr_name, sess->args);
    //350 Ready for RNTO.
    ftp_reply(sess, FTP_RNFROK, "Ready for RNTO.");
}

void do_rnto(Session_t *sess)
{
    if(sess->rnfr_name == NULL)
    {
        //503 RNFR required first.
        ftp_reply(sess, FTP_NEEDRNFR, "RNFR required first.");
        return;
    }

    if(rename(sess->rnfr_name, sess->args) == -1)
    {
        ftp_reply(sess, FTP_FILEFAIL, "Rename failed.");
        return;
    }
    free(sess->rnfr_name);
    sess->rnfr_name = NULL;

    //250 Rename successful.
    ftp_reply(sess, FTP_RENAMEOK, "Rename successful.");
}

void do_site(Session_t *sess)
{

}

void do_syst(Session_t *sess)
{
    ftp_reply(sess, FTP_SYSTOK, "UNIX Type: L8");
}

void do_feat(Session_t *sess)
{
    //211-Features:
    ftp_lreply(sess, FTP_FEAT, "Features:");

    //EPRT
    writen(sess->peer_fd, " EPRT\r\n", strlen(" EPRT\r\n"));
    writen(sess->peer_fd, " EPSV\r\n", strlen(" EPSV\r\n"));
    writen(sess->peer_fd, " MDTM\r\n", strlen(" MDTM\r\n"));
    writen(sess->peer_fd, " PASV\r\n", strlen(" PASV\r\n"));
    writen(sess->peer_fd, " REST STREAM\r\n", strlen(" REST STREAM\r\n"));
    writen(sess->peer_fd, " SIZE\r\n", strlen(" SIZE\r\n"));
    writen(sess->peer_fd, " TVFS\r\n", strlen(" TVFS\r\n"));
    writen(sess->peer_fd, " UTF8\r\n", strlen(" UTF8\r\n"));

    //211 End
    ftp_reply(sess, FTP_FEAT, "End");
}

void do_size(Session_t *sess)
{
    struct stat sbuf;
    if(lstat(sess->args, &sbuf) == -1)
    {
        ftp_reply(sess, FTP_FILEFAIL, "SIZE operation failed.");
        return;
    }

    //只能求普通文件size
    if(!S_ISREG(sbuf.st_mode))
    {
        ftp_reply(sess, FTP_FILEFAIL, "SIZE operation failed.");
        return;
    }

    //213 6
    char text[1024] = {0};
    snprintf(text, sizeof text, "%lu", sbuf.st_size);
    ftp_reply(sess, FTP_SIZEOK, text);
}

void do_stat(Session_t *sess)
{
    ftp_lreply(sess, FTP_STATOK, "FTP server status:");
   
    char text[1024] = {0};
    struct in_addr in;
    in.s_addr = sess->ip;
    snprintf(text, sizeof text, " Connected to %s\r\n", inet_ntoa(in));
    writen(sess->peer_fd, text, strlen(text));
   
    snprintf(text, sizeof text, " Logged in as %s\r\n", sess->username);
    writen(sess->peer_fd, text, strlen(text));
    ftp_reply(sess, FTP_STATOK, "End of status");
}

void do_noop(Session_t *sess)
{
    ftp_reply(sess, FTP_GREET, "(FtpServer 1.0)");
}

void do_help(Session_t *sess)
{
    ftp_lreply(sess, FTP_HELP, "The following commands are recognized.");
    writen(sess->peer_fd, " ABOR ACCT ALLO APPE CDUP CWD DELE EPRT EPSV FEAT HELP LIST MDTM MKD\r\n", strlen(" ABOR ACCT ALLO APPE CDUP CWD DELE EPRT EPSV FEAT HELP LIST MDTM MKD\r\n"));
    writen(sess->peer_fd, " MODE NLST NOOP OPTS PASS PASV PORT PWD QUIT REIN REST RETR RMD RNFR\r\n", strlen(" MODE NLST NOOP OPTS PASS PASV PORT PWD QUIT REIN REST RETR RMD RNFR\r\n"));
    writen(sess->peer_fd, " RNTO SITE SIZE SMNT STAT STOR STOU STRU SYST TYPE USER XCUP XCWD XMKD\r\n", strlen(" RNTO SITE SIZE SMNT STAT STOR STOU STRU SYST TYPE USER XCUP XCWD XMKD\r\n"));
    writen(sess->peer_fd, " XPWD XRMD\r\n", strlen(" XPWD XRMD\r\n"));
    ftp_reply(sess, FTP_HELP, "Help OK.");
}
