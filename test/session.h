#ifndef _SESSION_H_
#define _SESSION_H_
#include "common.h"
#define MAX_COMMAND 1024

/*
 *核心模块
 */
typedef struct 
{
    char command[MAX_COMMAND];//client发来的FTP指令
    char com[MAX_COMMAND];//FTP指令
    char args[MAX_COMMAND];//FTP指令的参数

    int peer_fd;//客户连接的fd

    int nobody_fd;//nobody进程所使用的fd
    int proto_fd;//proto进程所使用的fd

    uid_t user_uid; // 用户uid
    int ascii_mode; //是否为ascii传输模式

    struct sockaddr_in *p_addr; // port模式下对方的IP和port
    int data_fd; //数据传输fd
    int listen_fd; // 监听fd，用于PASV模式
}session_t;

//初始化session
void session_init(session_t *);

//将三个字符数组置位
void session_reset_command(session_t *);

//处理会话，这里主要是创建nobody与ftp子进程
void session_begin(session_t *);

#endif  /*_SESSION_H_*/
