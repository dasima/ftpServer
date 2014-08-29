#include "ftp_proto.h"

void handle_proto(session_t *ses)
{
    writen(ses->peerfd, "220(miniftp 0.1)\r\n", strlen("220(miniftp 0.1)\r\n"));
    while(1)
    {
        for(;;)
            pause();
    }
}   

