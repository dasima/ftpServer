#include "session.h"
#include "common.h"
#include "ftp_proto.h"
#include "ftp_nobody.h"
#include "priv_sock.h"
#include "configure.h"

void session_init(Session_t *sess)
{
    memset(sess->command, 0, sizeof (sess->command));
    memset(sess->com, 0, sizeof (sess->com));
    memset(sess->args, 0, sizeof (sess->args));
    sess->peer_fd = -1;
    sess->nobody_fd = -1;
    sess->proto_fd = -1;

    sess->user_uid = 0;
    sess->ascii_mode = 0;

    sess->p_addr = NULL;
    sess->data_fd = -1;
    sess->listen_fd = -1;

    sess->restart_pos = 0;
    sess->rnfr_name = NULL;

    sess->limits_max_upload = tunable_upload_max_rate * 1024;
    sess->limits_max_download = tunable_download_max_rate * 1024;
    sess->start_time_sec = 0;
    sess->start_time_usec = 0;

    sess->is_translating_data = 0;
    sess->is_receive_abor = 0;

    sess->curr_clients = 0;
}

void session_reset_command(Session_t *sess)
{
    memset(sess->command, 0, sizeof (sess->command));
    memset(sess->com, 0, sizeof (sess->com));
    memset(sess->args, 0, sizeof (sess->args));
}

void session_begin(Session_t *sess)
{
    priv_sock_init(sess);

    pid_t pid;
    if((pid = fork()) == -1)
        ERR_EXIT("fork");
    else if(pid == 0)
    {
        priv_sock_set_proto_context(sess);
        handle_proto(sess);
    }
    else
    {
        priv_sock_set_nobody_context(sess);
        handle_nobody(sess);
    }

}
