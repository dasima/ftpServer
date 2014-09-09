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
    const char *cmd;    //FTP指令
    void (*cmd_handler)(session_t *ses);//通过指针函数实现指令所对应的执行函数
}ftpcmd_t;

//NULL为命令还未实现
static ftpcmd_t ctrl_cmds[] = {
    /* 访问控制命令 */
    {"USER",    do_user },
    {"PASS",    do_pass },
    {"CWD",     do_cwd  },
    {"XCWD",    do_cwd  },
    {"CDUP",    do_cdup },
    {"XCUP",    do_cdup },
    {"QUIT",    do_quit },
    {"ACCT",    NULL    },
    {"SMNT",    NULL    },
    {"REIN",    NULL    },
    /* 传输参数命令 */
    {"PORT",    do_port },
    {"PASV",    do_pasv },
    {"TYPE",    do_type },
    {"STRU",    do_stru },
    {"MODE",    do_mode },

    /* 服务命令 */
    {"RETR",    do_retr },
    {"STOR",    do_stor },
    {"APPE",    do_appe },
    {"LIST",    do_list },
    {"NLST",    do_nlst },
    {"REST",    do_rest },
    {"ABOR",    do_abor },
    {"\377\364\377\362ABOR", do_abor},
    {"PWD",     do_pwd  },
    {"XPWD",    do_pwd  },
    {"MKD",     do_mkd  },
    {"XMKD",    do_mkd  },
    {"RMD",     do_rmd  },
    {"XRMD",    do_rmd  },
    {"DELE",    do_dele },
    {"RNFR",    do_rnfr },
    {"RNTO",    do_rnto },
    {"SITE",    do_site },
    {"SYST",    do_syst },
    {"FEAT",    do_feat },
    {"SIZE",    do_size },
    {"STAT",    do_stat },
    {"NOOP",    do_noop },
    {"HELP",    do_help },
    {"STOU",    NULL    },
    {"ALLO",    NULL    }
};

//进行命令映射
void do_command_map(session_t *ses)
{
    int i;
    int size = sizeof(ctrl_cmds) / sizeof(ctrl_cmds[0]); //数组大小
    for (i=0; i < size; ++i)
    {
        if (strcmp(ctrl_cmds[i].cmd, ses->com) == 0)
        {
            if (ctrl_cmds[i].cmd_handler != NULL)
            {
                ctrl_cmds[i].cmd_handler(ses);
            }
            else
            {
                //该命令没有实现
                ftp_reply(ses, FTP_COMMANDNOTIMPL, "Unimplement command.");
            }

            break;
        }
    }

    if (i == size)
    {
        //未识别的命令
        ftp_reply(ses, FTP_BADCMD, "Unknown command.");
    }
}

void ftp_reply(session_t *ses, int status, const char *text)
{
    char tmp[1024] = { 0 };
    snprintf(tmp, sizeof tmp, "%d %s\r\n", status, text);
    writen(ses->peer_fd, tmp, strlen(tmp));
}

void ftp_lreply(session_t *ses, int status, const char *text)
{
    char tmp[1024] = { 0 };
    snprintf(tmp, sizeof tmp, "%d-%s\r\n", status, text);
    writen(ses->peer_fd, tmp, strlen(tmp));
}

void do_user(session_t *ses)
{
    struct passwd *pw;
    if((pw = getpwnam(ses->args)) == NULL)
    {
        ftp_reply(ses, FTP_LOGINERR, "Login incorrect.");
        return;
    }

    ses->user_uid = pw->pw_uid;
    ftp_reply(ses, FTP_GIVEPWORD, "Please specify the password.");
}

void do_pass(session_t *ses)
{
    //struct passwd *getpwuid(uid_t uid)
    struct passwd *pw;
    if((pw = getpwuid(ses->user_uid)) == NULL)
    {
        ftp_reply(ses, FTP_LOGINERR, "Login incorrect.");
        return;
    }

    //struct spwd *getspnam(const char *name);
    struct spwd *spw;
    if((spw = getspnam(pw->pw_name)) == NULL)
    {
        ftp_reply(ses, FTP_LOGINERR, "Login incorrect.");
        return;
    }

    //ar *crypt(const char *key, const char *salt);
    char *encrypted_password = crypt(ses->args, spw->sp_pwdp);
    if(strcmp(encrypted_password, spw->sp_pwdp) != 0)
    {
        ftp_reply(ses, FTP_LOGINERR, "Login incorrect.");
        return;
    }

    if(setegid(pw->pw_gid) == -1)
        ERR_EXIT("setegid");
    if(seteuid(pw->pw_uid) == -1)
        ERR_EXIT("seteuid");

    //home---切换到住目录home
    if(chdir(pw->pw_dir) == -1)
        ERR_EXIT("chdir");
    //umask
    umask(tunable_local_umask);

    ftp_reply(ses, FTP_LOGINOK, "Login successful.");
}

void do_cwd(session_t *ses)
{

}

void do_cdup(session_t *ses)
{

}

void do_quit(session_t *ses)
{

}

void do_port(session_t *ses)
{
    //设置主动工作模式
    //PORT 192,168,44,1,200,174
    unsigned int v[6] = {0};
    sscanf(ses->args, "%u,%u,%u,%u,%u,%u", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5]);
    ses->p_addr = (struct sockaddr_in *)malloc(sizeof (struct sockaddr_in));
    memset(ses->p_addr, 0, sizeof(struct sockaddr_in));
    ses->p_addr->sin_family = AF_INET;
    char *p = (char*)&ses->p_addr->sin_port;
    p[0] = v[4];
    p[1] = v[5];
    p = (char*)&ses->p_addr->sin_addr.s_addr;
    p[0] = v[0];
    p[1] = v[1];
    p[2] = v[2];
    p[3] = v[3];
    ftp_reply(ses, FTP_PORTOK, "PORT command successful. Consider using PASV.");
}

void do_pasv(session_t *ses)
{
    char ip[16] = {0};
    get_local_ip(ip);

    //给nobody发送命令
    priv_sock_send_cmd(ses->proto_fd, PRIV_SOCK_PASV_LISTEN);
    //接收nobody的应答
    char res = priv_sock_recv_result(ses->proto_fd);
    if(res == PRIV_SOCK_RESULT_BAD)
    {
        ftp_reply(ses, FTP_BADCMD, "get listenfd error");
        return;
    }
    //接收port
    uint16_t port = priv_sock_recv_int(ses->proto_fd);

    //227 Entering Passive Mode (192,168,44,136,194,6).
    unsigned int v[6];
    sscanf(ip, "%u.%u.%u.%u", &v[0], &v[1], &v[2], &v[3]);
    uint16_t net_endian_port = htons(port); //网络字节序
    unsigned char *p = (unsigned char*)&net_endian_port;
    v[4] = p[0];
    v[5] = p[1];

    char text[1024] = {0};
    snprintf(text, sizeof text, "Entering Passive Mode (%u,%u,%u,%u,%u,%u).", v[0], v[1], v[2], v[3], v[4], v[5]);

    ftp_reply(ses, FTP_PASVOK, text);
}

void do_type(session_t *ses)
{
    if(strcmp(ses->args, "A") == 0)
    {
        ses->ascii_mode = 1;
        ftp_reply(ses, FTP_TYPEOK, "Switching to ASCII mode.");
    }
    else if(strcmp(ses->args, "I") == 0)
    {
        ses->ascii_mode = 0;
        ftp_reply(ses, FTP_TYPEOK, "Switching to Binary mode.");
    }
    else
    {
        ftp_reply(ses, FTP_BADCMD, "Unrecognised TYPE command.");
    }
}

void do_stru(session_t *ses)
{

}

void do_mode(session_t *ses)
{

}

void do_retr(session_t *ses)
{

}

void do_stor(session_t *ses)
{

}

void do_appe(session_t *ses)
{

}

void do_list(session_t *ses)
{
    /*
    //发起数据连接
    int fd = tcp_client(0);
    printf("fd %d\n", fd);
    int ret = connect_timeout(fd, ses->p_addr, tunable_connect_timeout);
    //int ret = connect(fd, (struct sockaddr*)(&ses->p_addr), sizeof(struct sockaddr_in));
    if(ret < 0)
    perror("connect"); 
    printf("port = %d\n", ntohs(ses->p_addr->sin_port));
    printf("Ip = %s\n", inet_ntoa(ses->p_addr->sin_addr));
    printf("ret - %d\n", ret);
    if(ret == -1)
    return;
    ses->data_fd = fd;
    */
    if(get_trans_data_fd(ses) == 0)
        return;

    //给出150 Here comes the directory listing.
    ftp_reply(ses, FTP_DATACONN, "Here comes the directory listing.");

    //传输目录列表 
    trans_list(ses);
    close(ses->data_fd); // 传输结束不要忘了关闭
    //close(fd);
    ses->data_fd = -1;

    //给出226，Directory send OK.
    ftp_reply(ses, FTP_TRANSFEROK, "Directory send OK.");
}

void do_nlst(session_t *ses)
{

}

void do_rest(session_t *ses)
{

}

void do_abor(session_t *ses)
{

}

void do_pwd(session_t *ses)
{
    char tmp[1024] = {0};
    if(getcwd(tmp, sizeof tmp) == NULL)
    {
        //return值为-1/0，函数进入系统内核，
        //返回值判断用perror
        //返回值为NULL,不用perror，fprintf(stderr, "a");
        fprintf(stderr, "get cwd error\n");
        ftp_reply(ses, FTP_BADMODE, "error");
        return;
    }
    char text[1024] = {0};
    snprintf(text, sizeof text, "\"%s\"", tmp);
    ftp_reply(ses, FTP_PWDOK, text);
}

void do_mkd(session_t *ses)
{

}

void do_rmd(session_t *ses)
{

}

void do_dele(session_t *ses)
{

}

void do_rnfr(session_t *ses)
{

}

void do_rnto(session_t *ses)
{

}

void do_site(session_t *ses)
{

}

void do_syst(session_t *ses)
{
    ftp_reply(ses, FTP_SYSTOK, "UNIX Type: L8");
}

void do_feat(session_t *ses)
{
    //211-Features:
    ftp_lreply(ses, FTP_FEAT, "Features:");

    //EPRT
    writen(ses->peer_fd, " EPRT\r\n", strlen(" EPRT\r\n"));
    writen(ses->peer_fd, " EPSV\r\n", strlen(" EPSV\r\n"));
    writen(ses->peer_fd, " MDTM\r\n", strlen(" MDTM\r\n"));
    writen(ses->peer_fd, " PASV\r\n", strlen(" PASV\r\n"));
    writen(ses->peer_fd, " REST STREAM\r\n", strlen(" REST STREAM\r\n"));
    writen(ses->peer_fd, " SIZE\r\n", strlen(" SIZE\r\n"));
    writen(ses->peer_fd, " TVFS\r\n", strlen(" TVFS\r\n"));
    writen(ses->peer_fd, " UTF8\r\n", strlen(" UTF8\r\n"));

    //211 End
    ftp_reply(ses, FTP_FEAT, "End");
}

void do_size(session_t *ses)
{

}

void do_stat(session_t *ses)
{

}

void do_noop(session_t *ses)
{

}

void do_help(session_t *ses)
{

}

