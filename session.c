#include "session.h"
#include "common.h"
#include "ftp_proto.h"
#include "ftp_nobody.h"
#include "priv_sock.h"
#include "configure.h"

//初始化session
void session_init(Session_t *sess)
{
    memset(sess->command, 0, sizeof (sess->command));
    memset(sess->com, 0, sizeof (sess->com));
    memset(sess->args, 0, sizeof (sess->args));

    sess->ip = 0;
    memset(sess->username, 0, sizeof(sess->username));

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
    sess->curr_ip_clients = 0;
}

//将三个字符数组置位
void session_reset_command(Session_t *sess)
{
    memset(sess->command, 0, sizeof (sess->command));
    memset(sess->com, 0, sizeof (sess->com));
    memset(sess->args, 0, sizeof (sess->args));
}

//处理会话，这里主要是创建nobody与ftp子进程
void session_begin(Session_t *sess)
{
    /*  nobody 进程和服务进程初始化 */
    priv_sock_init(sess);

    pid_t pid;
    /* 创建服务进程 */
    if((pid = fork()) == -1)
        ERR_EXIT("fork");
    else if(pid == 0)
    {
        /* 服务进程 */
        priv_sock_set_proto_context(sess);
        /* 接收命令并处理 */
        handle_proto(sess);
    }
    else
    {
        //  nobody 进程
        priv_sock_set_nobody_context(sess);
        handle_nobody(sess);
    }
}

