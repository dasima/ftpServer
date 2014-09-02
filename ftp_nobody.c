#include "ftp_nobody.h"
#include "common.h"
#include "sysutil.h"
#include "priv_command.h"
#include "priv_sock.h"

void set_nobody();

//nobody时刻准备从子进程接受命令
void handle_nobody(session_t *ses)
{
    //设置为nobody进程
    set_nobody();

    char cmd;
    while(1)
    {
        cmd = priv_sock_recv_cmd(ses->nobody_fd);
        switch (cmd)
        {
            case PRIV_SOCK_GET_DATA_SOCK:
                privop_pasv_get_data_sock(ses);
                break;
            case PRIV_SOCK_PASV_ACTIVE:
                privop_pasv_active(ses);
                break;
            case PRIV_SOCK_PASV_LISTEN:
                privop_pasv_listen(ses);
                break;
            case PRIV_SOCK_PASV_ACCEPT:
                privop_pasv_accept(ses);
                break;
            default:
                fprintf(stderr, "Unkown command\n");
                exit(EXIT_FAILURE);
        }
    }
}

void set_nobody()
{
    //基本思路
    //1. 首先获取nobody的uid、gid
    //2. 然后逐项进行设置
    struct passwd *pw;
    if((pw = getpwnam("nobody")) == NULL)
        ERR_EXIT("getpwnam");

    //先获取gid
    if(setegid(pw->pw_gid) == -1)
        ERR_EXIT("setegid");

    //euid---有效的用户ID
    if(seteuid(pw->pw_uid) == -1)
        ERR_EXIT("seteuid");
}
