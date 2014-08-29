#ifndef _SESSION_H_
#define _SESSION_H_

/*
 *核心模块
 */
typedef struct 
{
    char command[1024];//client发来的FTP指令
    char com[1024];//FTP指令
    char args[1024];//FTP指令的参数

    int peerfd;//客户连接的fd

    int nobody_fd;//nobody进程所使用的fd
    int proto_fd;//proto进程所使用的fd
}session_t;

//初始化session
void session_init(session_t *);

//处理会话，这里主要是创建nobody与ftp子进程
void session_begin(session_t *);

#endif  /*_SESSION_H_*/
