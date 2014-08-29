#include "ftp_proto.h"
#include "common.h"
#include "sysutil.h"

void handle_proto(session_t *ses)
{
    writen(ses->peerfd, "220(FtpServer 1.0)\r\n", strlen("220(FtpServer 1.0)\r\n"));
    while(1)
    {
        for(;;)
            pause();
    }
}   

