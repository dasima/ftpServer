#ifndef _PRIV_SOCK_H_
#define _PRIV_SOCK_H_

#include "session.h"
/*
 *内部进程间通信模块
 */

// FTP服务进程向nobody进程请求的命令
#define PRIV_SOCK_GET_DATA_SOCK     1
#define PRIV_SOCK_PASV_ACTIVE       2
#define PRIV_SOCK_PASV_LISTEN       3
#define PRIV_SOCK_PASV_ACCEPT       4

// nobody进程对FTP服务进程的应答
#define PRIV_SOCK_RESULT_OK         1
#define PRIV_SOCK_RESULT_BAD        2

void priv_sock_init(session_t *);
void priv_sock_close(session_t *);
void priv_sock_set_nobody_context(session_t *);
void priv_sock_set_proto_context(session_t *);
void priv_sock_send_cmd(int, char);
char priv_sock_recv_cmd(int);
void priv_sock_send_result(int, char);
char priv_sock_recv_result(int);
void priv_sock_send_int(int, int);
int priv_sock_recv_int(int);
void priv_sock_send_str(int, const char *, unsigned int);
void priv_sock_recv_str(int, char *, unsigned int);
void priv_sock_send_fd(int, int);
int priv_sock_recv_fd(int);

#endif  /*_PRIV_SOCK_H_*/
