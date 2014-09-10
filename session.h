#ifndef _SESSION_H_
#define _SESSION_H_

#include "common.h"
#define MAX_COMMAND 1024

/*
 * 核心模块
 */

typedef struct 
{
    char command[MAX_COMMAND];//client发来的FTP指令
    char com[MAX_COMMAND];//FTP指令
    char args[MAX_COMMAND];//FTP指令的参数

    int peer_fd;//客户连接的fd

    int nobody_fd;//nobody进程所使用的fd
    int proto_fd;//proto进程所使用的fd

    uid_t user_uid; //用户的uid
    int ascii_mode; //是否为ascii传输模式

    struct sockaddr_in *p_addr; //port模式下对方的ip和port
    int data_fd; //数据传输fd
    int listen_fd; //监听fd，用于PASV模式

    long long restart_pos; //文件传输断点
    char *rnfr_name; //文件重命名 RNTR RNTO


    int limits_max_upload; //限定的最大上传速度
    int limits_max_download; //限定的最大下载速度
    int start_time_sec; //开始的秒数
    int start_time_usec; //开始的微秒数
}Session_t;

//初始化session
void session_init(Session_t *sess);

//将三个字符数组置位
void session_reset_command(Session_t *sess);

//处理会话，这里主要是创建nobody与ftp子进程
void session_begin(Session_t *sess);

#endif  /*_SESSION_H_*/
