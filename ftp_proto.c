#include "ftp_proto.h"
#include "common.h"
#include "sysutil.h"
#include "strutil.h"
#include "ftp_codes.h"
#include "command_map.h"


void handle_proto(session_t *ses)
{
    //往客户端写
    //writen(ses->peerfd, "220 (FtpServer 1.0)\r\n", strlen("220 (FtpServer 1.0)\r\n"));
    ftp_reply(ses, FTP_GREET, "(FtpServer 1.0)");
    while(1)
    {
        session_reset_command(ses); //清空状态

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

        //相关命令操作都交给命令映射函数执行
        do_command_map(ses);
    }
}   


