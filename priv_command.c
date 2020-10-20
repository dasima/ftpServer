#include "priv_command.h"
#include "common.h"
#include "sysutil.h"
#include "priv_sock.h"
#include "configure.h"
#include <time.h>

/* 获得一个随机数 */
unsigned int zrand(void)
{
    return (unsigned int)(((int)time(0)) * 1103515245 + 12345 % 0x7fff);
}

//获取数据套接字
void privop_pasv_get_data_sock(Session_t *sess)
{
    char ip[16] = {0};
    priv_sock_recv_str(sess->nobody_fd, ip, sizeof ip);
    uint16_t port = priv_sock_recv_int(sess->nobody_fd);
    //创建fd
    /* 端口号：20 !!! */
    int data_fd = tcp_client(20);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    int ret = connect_timeout(data_fd, &addr, tunable_connect_timeout);
    if(ret == -1)
        ERR_EXIT("connect_timeout");
    priv_sock_send_result(sess->nobody_fd, PRIV_SOCK_RESULT_OK);
    priv_sock_send_fd(sess->nobody_fd, data_fd);
    close(data_fd);
}

//判断pasv模式是否开启
void privop_pasv_active(Session_t *sess)
{
    //发给proto结果
    priv_sock_send_int(sess->nobody_fd, (sess->listen_fd != -1));
}

//获取监听fd
void privop_pasv_listen(Session_t *sess)
{
    //创建listen fd
    char ip[16] = {0};
    get_local_ip(ip);

    /* ==== 从指定的端口范围内获取一个可用的端口号 ==*/
    int firstport = tunable_pasv_port_min;
    int lastport = tunable_pasv_port_max;

    int firstporttried = firstport + zrand() % (lastport - firstport + 1);
    int p = firstporttried;
    int data_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (data_fd == -1)
    {
        ERR_EXIT("socket");
        // error(425, MSG_CANT_PASSIVE);
        // return;
    }
    int on = 1;
    if (setsockopt(data_fd, SOL_SOCKET, SO_REUSEADDR,
                   (char *)&on, sizeof on) < 0)
    {
        ERR_EXIT("setsockopt");
        // error(421, "setsockopt");
        // return;
    }
    for (;;)
    {
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof addr);
        addr.sin_family = AF_INET;
        addr.sin_port = htons(p);
        if (bind(data_fd, (struct sockaddr *)&addr,
                 sizeof(addr)) == 0)
        {
            break;
        }
        p--;
        if (p < firstport)
        {
            p = lastport;
        }
        if (p == firstporttried)
        {
            (void)close(data_fd);
            data_fd = -1;
            // addreply_noformat(425, MSG_PORTS_BUSY);
            // return;
            ERR_EXIT("MSG_PORTS_BUSY");
        }
    }
    /* ========================================*/
    close(data_fd);
    int listenfd = tcp_server(ip, p);
    // int listenfd = tcp_server(ip, 36000);
    sess->listen_fd = listenfd;

    struct sockaddr_in addr;
    socklen_t len = sizeof addr;
    /* 获取套接字的本地协议地址 */
    if(getsockname(listenfd, (struct sockaddr*)&addr, &len) == -1)
        ERR_EXIT("getsockname");

    //发送应答
    priv_sock_send_result(sess->nobody_fd, PRIV_SOCK_RESULT_OK);
    //发送port
    uint16_t net_endian_port = ntohs(addr.sin_port);
    priv_sock_send_int(sess->nobody_fd, net_endian_port);
}

//accept一个新的连接
void privop_pasv_accept(Session_t *sess)
{
    //接受新连接
    int peerfd = accept_timeout(sess->listen_fd, NULL, tunable_accept_timeout);
    //清除状态
    close(sess->listen_fd);
    sess->listen_fd = -1;
    if(peerfd == -1)
    {
        priv_sock_send_result(sess->nobody_fd, PRIV_SOCK_RESULT_BAD);
        ERR_EXIT("accept_timeout");
    }

    //给对方回应
    priv_sock_send_result(sess->nobody_fd, PRIV_SOCK_RESULT_OK);
    //将data fd传给对方
    priv_sock_send_fd(sess->nobody_fd, peerfd);
    close(peerfd);
}

