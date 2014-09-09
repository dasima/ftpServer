#include "trans_data.h"
#include "common.h"
#include "command_map.h"
#include "ftp_codes.h"
#include "configure.h"
#include "sysutil.h"
#include "priv_sock.h"

//这5个函数为静态函数, 他们只能在本文件中使用
static const char *statbuf_get_perms(struct stat *);
static const char *statbuf_get_date(struct stat *);
static const char *statbuf_get_filename(struct stat*, const char *);
static const char *statbuf_get_user_info(struct stat *);
static const char *statbuf_get_size(struct stat *);

//判断主动模式是否开启
static int is_port_active(session_t *);
//被动模式是否开启
static int is_pasv_active(session_t *);

static void get_port_data_fd(session_t *);
static void get_pasv_data_fd(session_t *);
//返回值表示成功与否
int get_trans_data_fd(session_t *ses)
{
    int is_port = is_port_active(ses);
    int is_pasv = is_pasv_active(ses);


    //两者都未开启，应返回425
    if(!is_port && !is_pasv)
    {
        ftp_reply(ses, FTP_BADSENDCONN, "Use PORT or PASV first.");
        return 0;
    }

    if(is_port && is_pasv)
    {
        fprintf(stderr, "both of PORT and PASV are active\n");
        exit(EXIT_FAILURE);
    }

    //主动模式
    if(is_port)
        get_port_data_fd(ses);

    if(is_pasv)
        get_pasv_data_fd(ses);
    return 1;
}

/*
 *功能：传输文件列表
 */
void trans_list(session_t *ses)
{
    DIR *dir = opendir(".");
    if(dir == NULL)
        ERR_EXIT("opendir");

    struct dirent *dr;
    while((dr = readdir(dir)))
    {
        const char *filename = dr->d_name;
        if(filename[0] == '.')
            continue;

        char buf[1024] = {0};
        struct stat sbuf;
        if(lstat(filename, &sbuf) == -1)
            ERR_EXIT("lstat");

        strcpy(buf, statbuf_get_perms(&sbuf));
        strcat(buf, " ");
        strcat(buf, statbuf_get_user_info(&sbuf));
        strcat(buf, " ");
        strcat(buf, statbuf_get_size(&sbuf));
        strcat(buf, " ");
        strcat(buf, statbuf_get_date(&sbuf));
        strcat(buf, " ");
        strcat(buf, statbuf_get_filename(&sbuf, filename));

        strcat(buf, "\r\n");
        writen(ses->data_fd, buf, strlen(buf));
    }

    closedir(dir);
}


static const char *statbuf_get_perms(struct stat *sbuf)
{
    //这里使用static返回perms
    static char perms[] = "----------";
    mode_t mode = sbuf->st_mode;

    //文件类型
    switch(mode & S_IFMT)
    {
        case S_IFSOCK:
            perms[0] = 's';
            break;
        case S_IFLNK:
            perms[0] = 'l';
            break;
        case S_IFREG:
            perms[0] = '-';
            break;
        case S_IFBLK:
            perms[0] = 'b';
            break;
        case S_IFDIR:
            perms[0] = 'd';
            break;
        case S_IFCHR:
            perms[0] = 'c';
            break;
        case S_IFIFO:
            perms[0] = 'p';
            break;
    }

    //权限
    if(mode & S_IRUSR)
        perms[1] = 'r';
    if(mode & S_IWUSR)
        perms[2] = 'w';
    if(mode & S_IXUSR)
        perms[3] = 'x';
    if(mode & S_IRGRP)
        perms[4] = 'r';
    if(mode & S_IWGRP)
        perms[5] = 'w';
    if(mode & S_IXGRP)
        perms[6] = 'x';
    if(mode & S_IROTH)
        perms[7] = 'r';
    if(mode & S_IWOTH)
        perms[8] = 'w';
    if(mode & S_IXOTH)
        perms[9] = 'x';

    if(mode & S_ISUID)
        perms[3] = (perms[3] == 'x') ? 's' : 'S';
    if(mode & S_ISGID)
        perms[6] = (perms[6] == 'x') ? 's' : 'S';
    if(mode & S_ISVTX)
        perms[9] = (perms[9] == 'x') ? 't' : 'T';

    return perms;
}

//获取文件最近更改日期
static const char *statbuf_get_date(struct stat *sbuf)
{
    static char datebuf[1024] = {0};
    struct tm *ptm;
    time_t ct = sbuf->st_ctime;
    if((ptm = localtime(&ct)) == NULL)
        ERR_EXIT("localtime");

    const char *format = "%b %e %H:%M"; //时间格式

    if(strftime(datebuf, sizeof datebuf, format, ptm) == 0)
    {
        fprintf(stderr, "strftime error\n");
        exit(EXIT_FAILURE);
    }

    return datebuf;
}

//获取文件名字
static const char *statbuf_get_filename(struct stat *sbuf, const char *name)
{
    static char filename[1024] = {0};
    //name 处理链接名字
    if(S_ISLNK(sbuf->st_mode))
    {
        char linkfile[1024] = {0};
        if(readlink(name, linkfile, sizeof linkfile) == -1)
            ERR_EXIT("readlink");
        snprintf(filename, sizeof filename, " %s -> %s", name, linkfile);
    }else
    {
        strcpy(filename, name);
    }

    return filename;
}

//获取用户信息
static const char *statbuf_get_user_info(struct stat *sbuf)
{
    static char info[1024] = {0};
    snprintf(info, sizeof info, " %3d %8d %8d", sbuf->st_nlink, sbuf->st_uid, sbuf->st_gid);

    return info;
}

//获取文件大小
static const char *statbuf_get_size(struct stat *sbuf)
{
    static char buf[100] = {0};
    snprintf(buf, sizeof buf, "%8lu", (unsigned long)sbuf->st_size);
    return buf;
}

static int is_port_active(session_t *ses)
{
    return (ses->p_addr != NULL);
}

static int is_pasv_active(session_t *ses)
{
    //首先给nobody发命令
    priv_sock_send_cmd(ses->proto_fd, PRIV_SOCK_PASV_ACTIVE);
    //接收结果
    return priv_sock_recv_int(ses->proto_fd);
}

static void get_port_data_fd(session_t *ses)
{
    //发送cmd
    priv_sock_send_cmd(ses->proto_fd, PRIV_SOCK_GET_DATA_SOCK);
    //发送ip port
    char *ip = inet_ntoa(ses->p_addr->sin_addr);
    uint16_t port = ntohs(ses->p_addr->sin_port);
    printf("Port = %d\n", port);
    priv_sock_send_str(ses->proto_fd, ip, strlen(ip));
    priv_sock_send_int(ses->proto_fd, port);
    //接收应答
    char result = priv_sock_recv_result(ses->proto_fd);
    if(result == PRIV_SOCK_RESULT_BAD)
    {
        ftp_reply(ses, FTP_BADCMD, "get port data_fd error.");
        fprintf(stderr, "get data fd error\n");
        exit(EXIT_FAILURE);
    }
    //接收fd
    ses->data_fd = priv_sock_recv_fd(ses->proto_fd);

    //释放port模式
    free(ses->p_addr);
    ses->p_addr = NULL;
}

static void get_pasv_data_fd(session_t *ses)
{
    //先给nobody发命令
    priv_sock_send_cmd(ses->proto_fd, PRIV_SOCK_PASV_ACCEPT);

    //接收结果
    char res = priv_sock_recv_result(ses->proto_fd);
    if(res == PRIV_SOCK_RESULT_BAD)
    {
        ftp_reply(ses, FTP_BADCMD, "get pasv data_fd error.");
        fprintf(stderr, "get data fd error\n");
        exit(EXIT_FAILURE);
    }

    //接收fd
    ses->data_fd = priv_sock_recv_fd(ses->proto_fd);
}

