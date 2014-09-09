#include "priv_sock.h"
#include "common.h"
#include "sysutil.h"

void priv_sock_init(session_t *ses)
{
    int fds[2];
    if(socketpair(PF_UNIX, SOCK_STREAM, 0, fds) == -1)
        ERR_EXIT("socketpair");

    ses->nobody_fd = fds[0];
    ses->proto_fd = fds[1];
}

void priv_sock_close(session_t *ses)
{
    if(ses->nobody_fd != -1)
    {
        close(ses->nobody_fd);
        ses->nobody_fd = -1;
    }

    if(ses->proto_fd != -1)
    {
        close(ses->proto_fd);
        ses->proto_fd = -1;
    }
}
void priv_sock_set_nobody_context(session_t *ses)
{
    if(ses->proto_fd != -1)
    {
        close(ses->proto_fd);
        ses->proto_fd = -1;
    }
}

void priv_sock_set_proto_context(session_t *ses)
{
    if(ses->nobody_fd != -1)
    {
        close(ses->nobody_fd);
        ses->nobody_fd = -1;
    }
}

void priv_sock_send_cmd(int fd, char cmd)
{
    int ret = writen(fd, &cmd, sizeof cmd);
    if(ret != sizeof(cmd))
    {
        fprintf(stderr, "priv_sock_send_cmd error\n");
        exit(EXIT_FAILURE);
    }
}

char priv_sock_recv_cmd(int fd)
{
    char res;
    int ret = readn(fd, &res, sizeof res);
    //子进程关闭
    if(ret == 0)
    {
        printf("Proto close!\n");
        exit(EXIT_SUCCESS);
    }
    if(ret != sizeof(res))
    {
        fprintf(stderr, "priv_sock_recv_cmd error\n");
        exit(EXIT_FAILURE);
    }

    return res;
}

void priv_sock_send_result(int fd, char res)
{
    int ret = writen(fd, &res, sizeof res);
    if(ret != sizeof(res))
    {
        fprintf(stderr, "priv_sock_send_result\n");
        exit(EXIT_FAILURE);
    }
}

char priv_sock_recv_result(int fd)
{
    char res;
    int ret = readn(fd, &res, sizeof res);
    if(ret != sizeof(res))
    {
        fprintf(stderr, "priv_sock_recv_result error\n");
        exit(EXIT_FAILURE);
    }

    return res;
}

void priv_sock_send_int(int fd, int the_int)
{
    int ret = writen(fd, &the_int, sizeof(int));
    if(ret != sizeof(the_int))
    {
        fprintf(stderr, "priv_sock_send_int error\n");
        exit(EXIT_FAILURE);
    }
}

int priv_sock_recv_int(int fd)
{
    int res;
    int ret = readn(fd, &res, sizeof(res));
    if(ret != sizeof(res))
    {
        fprintf(stderr, "priv_sock_recv_int error\n");
        exit(EXIT_FAILURE);
    }

    return res;
}

void priv_sock_send_str(int fd, const char *buf, unsigned int len)
{
    priv_sock_send_int(fd, (int)len);
    int ret = writen(fd, (void*)buf, len);
    if(ret != (int)len)
    {
        fprintf(stderr, "priv_sock_send_str error\n");        
        exit(EXIT_FAILURE);
    }
}

void priv_sock_recv_str(int fd, char *buf, unsigned int len)
{
    unsigned int recv_len = (unsigned int)priv_sock_recv_int(fd);
    if (recv_len > len)
    {
        fprintf(stderr, "priv_sock_recv_str error\n");
        exit(EXIT_FAILURE);
    }

    int ret = readn(fd, buf, recv_len);
    if (ret != (int)recv_len)
    {
        fprintf(stderr, "priv_sock_recv_str error\n");
        exit(EXIT_FAILURE);
    }
}

void priv_sock_send_fd(int sock_fd, int fd)
{
    send_fd(sock_fd, fd);
}

int priv_sock_recv_fd(int sock_fd)
{
    return recv_fd(sock_fd);
}
