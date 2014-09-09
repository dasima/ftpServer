#ifndef _PRIV_SOCK_H_
#define _PRIV_SOCK_H_

#include "session.h"

// FTP服务进程向nobody进程请求的命令
#define PRIV_SOCK_GET_DATA_SOCK 1
#define PRIV_SOCK_PASV_ACTIVE 2
#define PRIV_SOCK_PASV_LISTEN 3
#define PRIV_SOCK_PASV_ACCEPT 4

// nobody进程对FTP服务进程的应答
#define PRIV_SOCK_RESULT_OK 1
#define PRIV_SOCK_RESULT_BAD 2

void priv_sock_init(Session_t *sess);
void priv_sock_close(Session_t *sess);
void priv_sock_set_nobody_context(Session_t *sess);
void priv_sock_set_proto_context(Session_t *sess);
void priv_sock_send_cmd(int fd, char cmd);
char priv_sock_recv_cmd(int fd);
void priv_sock_send_result(int fd, char res);
char priv_sock_recv_result(int fd);
void priv_sock_send_int(int fd, int the_int);
int priv_sock_recv_int(int fd);
void priv_sock_send_str(int fd, const char *buf, unsigned int len);
void priv_sock_recv_str(int fd, char *buf, unsigned int len);
void priv_sock_send_fd(int sock_fd, int fd);
int priv_sock_recv_fd(int sock_fd);

#endif  /*_PRIV_SOCK_H_*/
