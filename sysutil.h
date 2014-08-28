#ifndef _SYSUTIL_H_
#define _SYSUTIL_H_
#include "common.h"

int tcp_server(const char *, unsigned short);
int get_local_ip(char *);

void activate_nonblock(int);
void deactivate_nonblock(int);

int read_timeout(int, unsigned int);
int write_timeout(int, unsigned int);
int accept_timeout(int, struct sockaddr_in *, unsigned int);
int connect_timeout(int, struct sockaddr_in *, unsigned int);

ssize_t readn(int, void*, size_t);
ssize_t writen(int, void*, size_t);
ssize_t recv_peek(int, void*, size_t);
ssize_t readline(int, void*, size_t);

void send_fd(int, int);
int recv_fd(const int);


#endif  /*_SYSUTIL_H_*/
