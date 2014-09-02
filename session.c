#include "session.h"
#include "common.h"
#include "ftp_proto.h"
#include "ftp_nobody.h"
#include "priv_sock.h"

void session_init(session_t *ses)
{
    memset(ses->command, 0, sizeof(ses->command));
    memset(ses->com, 0, sizeof(ses->com));
    memset(ses->args, 0, sizeof(ses->args));
    ses->peerfd = -1;
    ses->nobody_fd = -1;
    ses->proto_fd = -1;

    ses->user_uid = 0;
    ses->ascii_mode = 0;

    ses->p_addr = NULL;
    ses->data_fd = -1;
    ses->listen_fd = -1;
}

void session_reset_command(session_t *ses)
{
    memset(ses->command, 0, sizeof (ses->command));
    memset(ses->com, 0, sizeof (ses->com));
    memset(ses->args, 0, sizeof (ses->args));
}

void session_begin(session_t *ses)
{
    priv_sock_init(ses);

    pid_t pid;
    if((pid = fork()) == -1)
        ERR_EXIT("fork");
    else if(pid == 0)
    {
        priv_sock_set_proto_context(ses);
        handle_proto(ses);
    }
    else
    {
        priv_sock_set_nobody_context(ses);
        handle_nobody(ses);
    }
}
