#include "command_map.h"
#include "common.h"
#include "sysutil.h"
#include "ftp_codes.h"


typedef struct ftpcmd
{
    const char *cmd;    //FTP指令
    void (*cmd_handler)(session_t *ses);//该指令所对应的执行函数
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
    for (i=0; i<size; ++i)
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
    writen(ses->peerfd, tmp, strlen(tmp));
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

}

void do_pasv(session_t *ses)
{

}

void do_type(session_t *ses)
{

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

}

void do_feat(session_t *ses)
{

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

