#include "ftp_proto.h"
#include "common.h"
#include "sysutil.h"
#include "strutil.h"
#include "ftp_codes.h"

void ftp_reply(session_t *, int, const char *);
void do_user(session_t *);
void do_pass(session_t *);

void handle_proto(session_t *ses)
{
    //往客户端写
    //writen(ses->peerfd, "220 (FtpServer 1.0)\r\n", strlen("220 (FtpServer 1.0)\r\n"));
    ftp_reply(ses, FTP_GREET, "(FtpServer 1.0)");
    while(1)
    {
        int ret = readline(ses->peerfd, ses->command, sizeof(ses->command));
        if(ret == -1)
        {
            if(errno == EINTR)
                continue;
            ERR_EXIT("readline");
        }
        else if(ret == 0)
        {
            exit(EXIT_SUCCESS);
        }
        str_trim_crlf(ses->command);
        str_split(ses->command, ses->com, ses->args, ' ');
        //这里把命令转化为大写
        str_upper(ses->com);
        printf("COMMD=[%s], ARGS=[%s]\n", ses->com, ses->args);
        if(strcmp("USER", ses->com) == 0)
            do_user(ses);
        if(strcmp("PASS", ses->com) == 0)
            do_pass(ses);
    }
}   

void ftp_reply(session_t *ses, int status, const char *text)
{
    char tmp[1024] = {0};
    snprintf(tmp, sizeof(tmp), "%d %s\r\n", status, text);
    writen(ses->peerfd, tmp, strlen(tmp));
}

void do_user(session_t *ses)
{
    struct passwd *pw;
    if((pw = getpwnam(ses->args)) == NULL)
    {
        ftp_reply(ses, FTP_LOGINERR, "login incorrect.");
        return;
    }
    ses->user_uid = pw->pw_uid;
    ftp_reply(ses, FTP_GIVEPWORD, "Please specify the pass.");
}

void do_pass(session_t *ses)
{
    //struct passwd *getpwuid(uid_t uid)
    struct passwd *pw;
    //getpwuid()---获取用户uid
    if((pw = getpwuid(ses->user_uid)) == NULL)
    {
        ftp_reply(ses, FTP_LOGINERR, "Login incorrect.");
        return;
    }

    //struct spwd *getspnam(const char *name);
    struct spwd *spw;
    //getpwnam()---获取与密码对应的用户名
    //function returns a pointer to a structure
   //containing the broken-out fields of the record
    //in  the  shadow password database
    //that matches the username name
    if((spw = getspnam(pw->pw_name)) == NULL)
    {
        ftp_reply(ses, FTP_LOGINERR, "Login incorrect.");
        return;
    }
    // the password encryption function
    // char *crypt(const char *key, const char *salt);
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

