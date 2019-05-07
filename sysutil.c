#include "sysutil.h"
#include "common.h"

static ssize_t recv_peek(int sockfd, void *buf, size_t len);

static int lock_file(int fd, int type);

static struct timeval tv = {0, 0}; //全局变量

//设置socket的选项，使其接收带外数据
void activate_oobinline(int sockfd)
{
    int oob_inline = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_OOBINLINE, &oob_inline, sizeof(oob_inline)) == -1)
        ERR_EXIT("setsockopt oobinline");
}

//开启SIGURG信号，当有带外数据到来时，发出该信号
void activate_signal_sigurg(int sockfd)
{
    if(fcntl(sockfd, F_SETOWN, getpid()) == -1)
        ERR_EXIT("fcntl sigurg");
}

int get_curr_time_sec()
{
    if(gettimeofday(&tv, NULL) == -1)
        ERR_EXIT("gettimeofday");
    return tv.tv_sec;
}

int get_curr_time_usec()
{
    return tv.tv_usec;
}

int nano_sleep(double t)
{
    int sec = (time_t)t;//取整数部分
    int nsec = (t - sec) * 1000000000;
    //int nanosleep(const struct timespec *req, struct timespec *rem);
    struct timespec ts;
    ts.tv_sec = sec;
    ts.tv_nsec = nsec;

    int ret;
    do
    {//当睡眠被打断时，剩余时间放到ts里面
        ret = nanosleep(&ts, &ts);
    }
    while(ret == -1 && errno == EINTR)
        ;

    return ret;
}

int lock_file_read(int fd)
{
    return lock_file(fd, F_RDLCK);
}

int lock_file_write(int fd)
{
    return lock_file(fd, F_WRLCK);
}

int unlock_file(int fd)
{
    struct flock lock;
    memset(&lock, 0, sizeof lock);
    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();

    return fcntl(fd, F_SETLK, &lock);
}

/*
 * 对文件设置锁，防止多进程并发产生冲突操作
 */
static int lock_file(int fd, int type)
{
    struct flock lock;
    memset(&lock, 0, sizeof lock);
    lock.l_type = type;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();

    int ret;
    do
    {
        // 设置锁，阻塞
        ret = fcntl(fd, F_SETLKW, &lock);
    }
    while(ret == -1 && errno == EINTR)
        ;

    return ret;
}

/*
 *函数功能：创建客户套接字
 *参数port: 端口号
 *函数返回值：返回客户端套接字
 */
int tcp_client(unsigned int port)
{
    int sockfd;
    //设置端口复用
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        ERR_EXIT("socket");

    if(port > 0)
    {
        int on = 1;
        //设置端口复用
        if((setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on))) == -1)
            ERR_EXIT("setsockopt");

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof addr);
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        char ip[16] = {0};
        get_local_ip(ip);
        addr.sin_addr.s_addr = inet_addr(ip);

        if(bind(sockfd, (struct sockaddr *)&addr, sizeof addr) == -1)
            ERR_EXIT("bind");
    }

    return sockfd;
}

/*
 *函数功能：启动服务器
 *参数host：服务器IP地址或者服务器主机名
 *参数port：服务器端口
 *函数返回值：成功则返回监听套接字
 */
int tcp_server(const char *host, unsigned short port)
{
    //建立套接字
    int listenfd;
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        ERR_EXIT("tcp_server");
    struct sockaddr_in seraddr;
    memset(&seraddr, 0, sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    if(host != NULL)
    {
        /*
         *inet_aton函数将const char * 类型的host
         *转化为二进制形式
         *并存放到in_addr结构体中，sin_addr是in_addr类型的结构体
         */
        if(inet_aton(host, &seraddr.sin_addr) == 0)
        {
            /*
             *gethostbyname返回一个hostent结构体类型
             *并将host拷贝到hostent结构体下的h_name中，
             *以及host的in_addr结构体类型内容拷贝到返回值中hostent结构体类型的h_addr_list[0]中
             *其中#define h_addr h_addr_list[0]
             */
            struct hostent *hp;
            hp = gethostbyname(host);
            if(hp == NULL)
                ERR_EXIT("gethostbyname");
            seraddr.sin_addr = *(struct in_addr*)hp->h_addr;
        }
    }
    else
        seraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    seraddr.sin_port = htons(port);

    //设置端口复用
    int on = 1;
    if((setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on))) < 0)
        ERR_EXIT("setsockopt");

    //绑定
    if(bind(listenfd, (struct sockaddr*)&seraddr, sizeof(seraddr)) < 0)
        ERR_EXIT("bind");

    //监听
    if(listen(listenfd, SOMAXCONN) < 0)
        ERR_EXIT("listen");

    return listenfd;
}

/*
 *功能：获取当前主机的IP
 *ip：
 *返回值：
 */
int get_local_ip(char *ip)
{
    int sockfd;
    if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        ERR_EXIT("socket");
    }

    struct ifreq req;

    bzero(&req, sizeof(struct ifreq));
    strcpy(req.ifr_name, "eth0");

    if(ioctl(sockfd, SIOCGIFADDR, &req) == -1)
        ERR_EXIT("ioctl");

    struct sockaddr_in *host = (struct sockaddr_in*)&req.ifr_addr;
    strcpy(ip, inet_ntoa(host->sin_addr));
    close(sockfd);
    return 1;
}

/*
 *功能：设置I/O为非阻塞模式
 *fd：文件描述符
 */
void activate_nonblock(int fd)
{
    int ret;
    int flags = fcntl(fd, F_GETFL);
    if(flags == -1)
        ERR_EXIT("fcntl");

    flags |= O_NONBLOCK;//
    ret = fcntl(fd, F_SETFL, flags);
    if(ret == -1)
        ERR_EXIT("fcntl");
}

/*
 *功能：设置I/O为阻塞模式
 *fd：文件描述符
 */
void deactivate_nonblock(int fd)
{
    int ret;
    int flags = fcntl(fd, F_GETFL);
    if(flags == -1)
        ERR_EXIT("fcntl");

    flags &= ~O_NONBLOCK;
    ret = fcntl(fd, F_SETFL, flags);
    if(ret == -1)
        ERR_EXIT("fcntl");
}

/*
 *功能：读操作检测函数，不包含读操作
 *fd：文件描述符
 *wait_seconds：等待超时时间(s),如果为0表示不检测超时
 *返回值：成功返回0，失败返回-1，超时返回-1，且errno = ETIMEDOUT
 */
int read_timeout(int fd, unsigned int wait_seconds)
{
    int ret;
    if(wait_seconds > 0)
    {
        fd_set read_fd;
        FD_ZERO(&read_fd);
        FD_SET(fd, &read_fd);

        struct timeval timeout;
        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;

        do
        {
            ret = select(fd + 1, &read_fd, NULL, NULL, &timeout);
        }while(ret < 0 && errno == EINTR);

        if(ret == 0)
        {
            ret = -1;
            errno = ETIMEDOUT;//ETIMEDOUT--connection timed out(POSIX.1)
        }
        else if(ret == 1)
            ret = 0;
    }
    return ret;
}

/*
 *功能：写超时检测函数，不包含写操作
 *fd：文件描述符
 *wait_seconds：等待超时时间(秒)，如果为0表示不检测超时
 *返回值：成功返回0，失败返回-1，超时返回-1，且errno = ETIMEDOUT
 */
int write_timeout(int fd, unsigned int wait_seconds)
{
    int ret;
    if(wait_seconds > 0)
    {
        fd_set write_fd;
        // 初始化套接字队列
        FD_ZERO(&write_fd);
        // 将套接字 fd 插入到套接字队列 write_fd 中
        FD_SET(fd, &write_fd);

        struct timeval timeout;
        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;

        do
        {
            ret = select(fd + 1, NULL, &write_fd, NULL,  &timeout);
        }while(ret < 0 && errno == EINTR);

        if(ret == 0)
        {
            ret = -1;
            errno = ETIMEDOUT;//ETIMEDOUT--connection timed out(POSIX.1)
        }
        else if(ret == 1)
            ret = 0;
    }
    return ret;
}

/*
 *功能：有超时功能的accept
 *fd：套接字
 *addr:输出参数，返回对方地址
 *wait_seconds:等待超时时间，0表示正常不带有超时功能
 *返回值：成功(未超时)返回已连接套接字，超时返回-1，并且errno = ETIMEDOUT
 */
int accept_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds)
{
    int ret;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    /* 超时检测 */
    if(wait_seconds > 0)
    {
        fd_set accept_fd;
        FD_ZERO(&accept_fd);
        FD_SET(fd, &accept_fd);

        struct timeval timeout;
        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;

        do
        {
            ret = select(fd + 1, &accept_fd, NULL, NULL, &timeout);
        }while(ret < 0 && errno == EINTR);
        if(ret == -1)
            return -1;
        else if(ret == 0)
        {
            errno = ETIMEDOUT;
            return -1;
        }
    }

    if(addr != NULL)
        ret = accept(fd, (struct sockaddr*)addr, &addrlen);
    else
        ret = accept(fd, NULL, NULL);

    return ret;
}

/*
 *功能：有超时功能的connect
 *fd: 套接字
 *addr: 要连接的对方地址
 *wait_seconds: 等待的超时时间(秒), 0表示为正常没有超时设置
 *返回值：成功(未超时)返回0， 失败返回-1， 超时返回-1并且errno = ETIMEDOUT
 */
//???????????????
int connect_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds)
{
    int ret;
    socklen_t addrlen = sizeof(struct sockaddr_in);

    if(wait_seconds > 0)
        activate_nonblock(fd);

    ret = connect(fd, (struct sockaddr*)addr, addrlen);
    printf("ret %d\n", ret);
    if(ret < 0 && errno == EINPROGRESS)
    {
        fd_set connect_fd;
        FD_ZERO(&connect_fd);
        FD_SET(fd, &connect_fd);

        struct timeval timeout;
        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;

        do
        {
            printf("a\n");
            /*一旦连接建立，套接字就可写*/
            ret = select(fd + 1, NULL, &connect_fd, NULL, &timeout);
            printf("ret - %d\n", ret);
        }while(ret < 0 && errno == EINTR);
        if(ret == 0)
        {
            ret = -1;
            errno = ETIMEDOUT;
        }
        else if(ret < 0)
            return -1;
        else if(ret == 1)
        {
            /*
             *ret返回为1
             *可能有两种情况：一种是连接建立成功，一种是套接字产生错误
             *此时错误信息不会保存至errno变量中，因此，需要调用getsockopt来获取
             */
            int err;
            socklen_t socklen = sizeof(err);
            int sockoptret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &socklen);
            if(sockoptret == -1)
            {
                return -1;
            }
            //else if(err == 0)
            if(err == 0)//这里
            {
                ret = 0;
            }
            else
            {
                errno = err;
                ret = -1;
            }
        }
    }
    if(wait_seconds > 0)
    {
        deactivate_nonblock(fd);
    }
    return ret;
}

/*
 *功能：读取固定的字节数
 *fd:文件描述符
 *buf:接受数据的缓冲区
 *n:要读取的字节数
 *返回值：成功返回n,失败返回-1，读到EOF返回小于n的值
 */
ssize_t readn(int fd, void *buf, size_t n)
{
    size_t nleft = n;
    ssize_t nread;
    char *bufp = (char*)buf;

    while(nleft > 0)
    {
        if((nread = read(fd, bufp, nleft)) < 0)//这里
        {
            if(errno == EINTR)
                continue;
            return -1;
        }
        else if(nread == 0)
            return n - nleft;

        bufp += nread;
        nleft -= nread;
    }
    return n;
}

/*
 *功能：发送固定的字节数
 *fd:文件描述符
 *buf:发送数据的缓冲区
 *n:要发送的字节数
 *返回值：成功返回n,失败返回-1
 */
ssize_t writen(int fd, const void *buf, size_t n)
{
    size_t nleft = n;
    ssize_t nwrite;
    char *bufp = (char*)buf;

    while(nleft > 0)
    {
        if((nwrite = write(fd, bufp, nleft)) < 0)//这里
        {
            if (errno == EINTR)
                continue;
            return -1;
        }
        else if (nwrite == 0)
            continue;

        bufp += nwrite;
        nleft -= nwrite;
    }
    return n;
}

/*
 *功能：查看套接字缓冲区数据，但不移除数据
 *sockfd:套接字
 *buf:接受数据缓冲区
 *len: 接受字节数长度
 *返回值：成功一个返回大于零（即从recv获得的字节数）的值，失败返回-1
 */
static ssize_t recv_peek(int sockfd, void *buf, size_t len)
{
    int nread;
    while (1)
    {
        //这个过程只成功调用一次
        nread = recv(sockfd, buf, len, MSG_PEEK);
        if (nread < 0 && errno == EINTR)//这里
        {
            //被中断则继续读取
            continue;
        }
        if(nread < 0)
        {
            return -1;
        }
        break;
    }
    return nread;
}

/*
 *功能：按行读取数据
 *sockfd:套接字
 *buf:接受数据的缓冲区
 *maxsize: 一行的最大长度
 *返回值：成功返回大于等于零的值， 失败返回-1
 */
ssize_t readline(int sockfd, void *buf, size_t maxsize)
{
    int nread;  //一次IO读取的数量
    int nleft;  //还剩余的字节数
    char *ptr;  //存放数据的指针的位置
    int ret;    //readn的返回值
    int total = 0;  //目前总共读取的字节数

    nleft = maxsize-1;
    ptr = buf;

    while (nleft > 0) {
        //这一次调用仅仅是预览数据
        //并没有真的把数据从缓冲区中取走
        ret = recv_peek(sockfd, ptr, nleft);
        //注意这里读取的字节不够，绝对不是错>    误！！！
        if(ret <= 0)
        {
            return ret;
        }

        nread = ret;
        int i;
        for(i = 0; i < nread; ++i)
        {
            if (ptr[i] == '\n')
            {
                //这里才是真正的读取过程
                ret = readn(sockfd, ptr, i + 1);
                if (ret != i + 1)
                {
                    return -1;
                }
                total += ret;
                ptr += ret;
                *ptr = 0;
                return total;   //返回此行的>    长度 '\n'包含在其中
            }
        }
        //如果没有发现\n,这些数据应全部接收
        ret = readn(sockfd, ptr, nread);
        if(ret != nread)
        {
            return -1;
        }
        nleft -= nread;
        total += nread;
        ptr += nread;
    }
    *ptr = 0;
    return maxsize - 1;
}

/*
 *功能：
 *sockfd:
 *fd:
 */
void send_fd(int sockfd, int fd)
{
    int ret;
    struct msghdr msg;
    struct cmsghdr *p_cmsg;
    struct iovec vec;
    char cmsgbuf[CMSG_SPACE(sizeof(fd))];//这里用到宏
    int *p_fds;
    char sendchar = 0;
    msg.msg_control = cmsgbuf;//这里用到cmsgbuf
    msg.msg_controllen = sizeof(cmsgbuf);
    p_cmsg = CMSG_FIRSTHDR(&msg);
    p_cmsg->cmsg_level = SOL_SOCKET;
    p_cmsg->cmsg_type = SCM_RIGHTS;
    p_cmsg->cmsg_len = CMSG_LEN(sizeof(fd));
    p_fds = (int*)CMSG_DATA(p_cmsg);
    *p_fds = fd;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &vec;//这里用到vec
    msg.msg_iovlen = 1;
    msg.msg_flags = 0;

    vec.iov_base = &sendchar;
    vec.iov_len = sizeof(sendchar);
    ret = sendmsg(sockfd, &msg, 0);//这里用到msg
    if(ret != 1)
        ERR_EXIT("sendmsg");
}

/*
 *功能：
 *sockfd:
 *返回值：
 */
int recv_fd(const int sockfd)
{
    int ret;
    struct msghdr msg;
    char recvchar;
    struct iovec vec;
    int recvfd;
    char cmsgbuf[CMSG_SPACE(sizeof(recvfd))];
    struct cmsghdr *p_cmsg;
    int *p_fd;
    vec.iov_base = &recvchar;
    vec.iov_len = sizeof(recvchar);
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &vec;
    msg.msg_iovlen = 1;
    msg.msg_control = cmsgbuf;
    msg.msg_controllen = sizeof(cmsgbuf);
    msg.msg_flags = 0;

    p_fd = (int*)CMSG_DATA(CMSG_FIRSTHDR(&msg));
    *p_fd = -1;
    ret = recvmsg(sockfd, &msg, 0);
    if(ret != 1)
        ERR_EXIT("recvmsg");

    p_cmsg = CMSG_FIRSTHDR(&msg);
    if(p_cmsg == NULL)
        ERR_EXIT("no passed fd");

    p_fd = (int *)CMSG_DATA(p_cmsg);
    recvfd = *p_fd;
    if(recvfd == -1)
        ERR_EXIT("no [assed fd");

    return recvfd;
}

