#include "ftp_proto.h"
#include "common.h"
#include "sysutil.h"
#include "strutil.h"
#include "ftp_codes.h"

void ftp_reply(session_t *, int, const char *);
void do_user(session_t *);

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
        printf("COMMD=[%s], ARGS=[%s]\n", ses->com, ses->args);
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
