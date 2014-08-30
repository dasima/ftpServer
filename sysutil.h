#ifndef _SYSUTIL_H_
#define _SYSUTIL_H_
#include "common.h"

//创建监听fd
int tcp_server(const char *, unsigned short);
int get_local_ip(char *);

//设置fd为阻塞或非阻塞
void activate_nonblock(int);
void deactivate_nonblock(int);

//超时IO函数
int read_timeout(int, unsigned int);
int write_timeout(int, unsigned int);
int accept_timeout(int, struct sockaddr_in *, unsigned int);
int connect_timeout(int, struct sockaddr_in *, unsigned int);

//处理TCP粘包问题函数
ssize_t readn(int, void*, size_t);
ssize_t writen(int, void*, size_t);
ssize_t recv_peek(int, void*, size_t);
ssize_t readline(int, void*, size_t);

//发送与接受函数
void send_fd(int, int);
int recv_fd(const int);


#endif  /*_SYSUTIL_H_*/
