#include "priv_command.h"
#include "common.h"
#include "sysutil.h"
#include "priv_sock.h"
#include "configure.h"

//获取数据套接字
void privop_pasv_get_data_sock(session_t *ses)
{
    char ip[16] = {0};
    priv_sock_recv_str(ses->nobody_fd, ip, sizeof ip);
    uint16_t port = priv_sock_recv_int(ses->nobody_fd);
    //创建fd
    int data_fd = tcp_client(20);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    int ret = connect_timeout(data_fd, &addr, tunable_connect_timeout);
    if(ret == -1)
        ERR_EXIT("connect_timeout");
    priv_sock_send_result(ses->nobody_fd, PRIV_SOCK_RESULT_OK);
    priv_sock_send_fd(ses->nobody_fd, data_fd);
    close(data_fd);
}

//判断pasv模式是否开启
void privop_pasv_active(session_t *ses)
{
    //发给proto结果
    priv_sock_send_int(ses->nobody_fd, (ses->listen_fd != -1));
}

//获取监听fd
void privop_pasv_listen(session_t *ses)
{
    //创建listen fd
    char ip[16] = {0};
    get_local_ip(ip);
    int listenfd = tcp_server(ip, 0);
    ses->listen_fd = listenfd;

    struct sockaddr_in addr;
    socklen_t len = sizeof addr;
    if(getsockname(listenfd, (struct sockaddr*)&addr, &len) == -1)
         ERR_EXIT("getsockname");

    //发送应答
    priv_sock_send_result(ses->nobody_fd, PRIV_SOCK_RESULT_OK);
    //发送port
    uint16_t net_endian_port = ntohs(addr.sin_port);
    priv_sock_send_int(ses->nobody_fd, net_endian_port);
}

//accept一个新的连接
void privop_pasv_accept(session_t *ses)
{
    //接受新连接
    int peerfd = accept_timeout(ses->listen_fd, NULL, tunable_accept_timeout);
    //清除状态
    close(ses->listen_fd);
    ses->listen_fd = -1;
    if(peerfd == -1)
    {
        priv_sock_send_result(ses->nobody_fd, PRIV_SOCK_RESULT_BAD);
        ERR_EXIT("accept_timeout");
    }

    //给对方回应
    priv_sock_send_result(ses->nobody_fd, PRIV_SOCK_RESULT_OK);
    //将data fd传给对方
    priv_sock_send_fd(ses->nobody_fd, peerfd);
    close(peerfd);
}

