#include "trans_data.h"
#include "common.h"
#include "sysutil.h"
#include "ftp_codes.h"
#include "command_map.h"
#include "configure.h"
#include "priv_sock.h"

//这5个函数为静态函数, 他们只能在本文件中使用
static const char *statbuf_get_perms(struct stat *sbuf);
static const char *statbuf_get_date(struct stat *sbuf);
static const char *statbuf_get_filename(struct stat *sbuf, const char *name);
static const char *statbuf_get_user_info(struct stat *sbuf);
static const char *statbuf_get_size(struct stat *sbuf);

//判断主动模式是否开启
static int is_port_active(Session_t *sess);
//判断被动模式是否开启
static int is_pasv_active(Session_t *sess);

static void get_port_data_fd(Session_t *sess);
static void get_pasv_data_fd(Session_t *sess);

static void trans_list_common(Session_t *sess, int list);

void upload_file(Session_t *sess, int is_appe)
{
    //获取data fd
    if(get_trans_data_fd(sess) == 0)
    {
        ftp_reply(sess, FTP_UPLOADFAIL, "Failed to get data fd.");
        return;
    }

    //open 文件
    int fd = open(sess->args, O_WRONLY | O_CREAT, 0666);
    if(fd == -1)
    {
        ftp_reply(sess, FTP_UPLOADFAIL, "Failed to open file.");
        return;
    }

    //对文件加锁
    if(lock_file_write(fd) == -1)
    {
        ftp_reply(sess, FTP_UPLOADFAIL, "Failed to lock file.");
        return;
    }

    //判断是否是普通文件
    struct stat sbuf;
    if(fstat(fd, &sbuf) == -1)
        ERR_EXIT("fstat");
    if(!S_ISREG(sbuf.st_mode))
    {
        ftp_reply(sess, FTP_UPLOADFAIL, "Can only upload regular file.");
        return;
    }

    //区分模式
    long long offset = sess->restart_pos;
    unsigned long filesize = 0;
    if(!is_appe && offset == 0) //STOR
    {
        //创建新的文件
        ftruncate(fd, 0); //如果源文件存在则直接覆盖
    }
    else if(!is_appe && offset != 0) // REST + STOR
    {
        //lseek进行偏移
        ftruncate(fd, offset); //截断后面的内容
        if(lseek(fd, offset, SEEK_SET) == -1)
            ERR_EXIT("lseek");
        filesize = offset;
    }
    else //APPE
    {
        //对文件进行扩展 偏移到末尾进行追加
        if(lseek(fd, 0, SEEK_END) == -1)
            ERR_EXIT("lseek");

        //获取文件大小
        if(fstat(fd, &sbuf) == -1)
            ERR_EXIT("fstat");
        filesize = sbuf.st_size;
    }

    //150 ascii
    //150 Opening ASCII mode data connection for /home/wing/redis-stable.tar.gz (1251318 bytes).
    char text[1024] = {0};
    if(sess->ascii_mode == 1)
        snprintf(text, sizeof text, "Opening ASCII mode data connection for %s (%lu bytes).", sess->args, filesize);
    else
        snprintf(text, sizeof text, "Opening Binary mode data connection for %s (%lu bytes).", sess->args, filesize);
    ftp_reply(sess, FTP_DATACONN, text);

    //上传
    char buf[4096] = {0};
    int flag = 0;
    while(1)
    {
        int nread = read(sess->data_fd, buf, sizeof buf);
        if(nread == -1)
        {
            if(errno == EINTR)
                continue;
            flag = 1;
            break;
        }
        else if(nread == 0)
        {
            flag = 0;
            break;
        }

        if(writen(fd, buf, nread) != nread)
        {
            flag = 2;
            break;
        }
    }

    //清理 关闭fd 文件解锁
    if(unlock_file(fd) == -1)
        ERR_EXIT("unlock_file");
    close(fd);
    close(sess->data_fd);
    sess->data_fd = -1;


    //226
    if(flag == 0)
        ftp_reply(sess, FTP_TRANSFEROK, "Transfer complete.");
    else if(flag == 1)
        ftp_reply(sess, FTP_BADSENDNET, "Reading from Network Failed.");
    else
        ftp_reply(sess, FTP_BADSENDFILE, "Writing to File Failed.");

}

void trans_list(Session_t *sess, int list)
{
    //发起数据连接
    if(get_trans_data_fd(sess) == 0)
        return ;

    //给出150 Here comes the directory listing.
    ftp_reply(sess, FTP_DATACONN, "Here comes the directory listing.");

    //传输目录列表
    if(list == 1)
        trans_list_common(sess, 1);
    else
        trans_list_common(sess, 0);
    close(sess->data_fd); //传输结束记得关闭
    sess->data_fd = -1;

    //给出226 Directory send OK.
    ftp_reply(sess, FTP_TRANSFEROK, "Directory send OK.");
}

//返回值表示成功与否
int get_trans_data_fd(Session_t *sess)
{
    int is_port = is_port_active(sess);
    int is_pasv = is_pasv_active(sess);


    //两者都未开启，应返回425
    if(!is_port && !is_pasv)
    {
        ftp_reply(sess, FTP_BADSENDCONN, "Use PORT or PASV first.");
        return 0;
    }

    if(is_port && is_pasv)
    {
        fprintf(stderr, "both of PORT and PASV are active\n");
        exit(EXIT_FAILURE);
    }

    //主动模式
    if(is_port)
    {
        get_port_data_fd(sess);
    }

    if(is_pasv)
    {
        get_pasv_data_fd(sess);    
    }

    return 1;
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
        snprintf(filename, sizeof filename, "%s -> %s", name, linkfile);
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

static int is_port_active(Session_t *sess)
{
    return (sess->p_addr != NULL);
}

static int is_pasv_active(Session_t *sess)
{
    //首先给nobody发命令
    priv_sock_send_cmd(sess->proto_fd, PRIV_SOCK_PASV_ACTIVE);
    //接收结果
    return priv_sock_recv_int(sess->proto_fd);
}

static void get_port_data_fd(Session_t *sess)
{
    //发送cmd
    priv_sock_send_cmd(sess->proto_fd, PRIV_SOCK_GET_DATA_SOCK);
    //发送ip port
    char *ip = inet_ntoa(sess->p_addr->sin_addr);
    uint16_t port = ntohs(sess->p_addr->sin_port);
    priv_sock_send_str(sess->proto_fd, ip, strlen(ip));
    priv_sock_send_int(sess->proto_fd, port);
    //接收应答
    char result = priv_sock_recv_result(sess->proto_fd);
    if(result == PRIV_SOCK_RESULT_BAD)
    {
        ftp_reply(sess, FTP_BADCMD, "get pasv data_fd error");
        fprintf(stderr, "get data fd error\n");
        exit(EXIT_FAILURE);
    }
    //接收fd
    sess->data_fd = priv_sock_recv_fd(sess->proto_fd);

    //释放port模式
    free(sess->p_addr);
    sess->p_addr = NULL;
}

static void get_pasv_data_fd(Session_t *sess)
{
    //先给nobody发命令
    priv_sock_send_cmd(sess->proto_fd, PRIV_SOCK_PASV_ACCEPT);

    //接收结果
    char res = priv_sock_recv_result(sess->proto_fd);
    if(res == PRIV_SOCK_RESULT_BAD)
    {
        ftp_reply(sess, FTP_BADCMD, "get pasv data_fd error");
        fprintf(stderr, "get data fd error\n");
        exit(EXIT_FAILURE);
    }

    //接收fd
    sess->data_fd = priv_sock_recv_fd(sess->proto_fd);
}


static void trans_list_common(Session_t *sess, int list)
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

        if(list == 1) // LIST
        {
            strcpy(buf, statbuf_get_perms(&sbuf));
            strcat(buf, " ");
            strcat(buf, statbuf_get_user_info(&sbuf));
            strcat(buf, " ");
            strcat(buf, statbuf_get_size(&sbuf));
            strcat(buf, " ");
            strcat(buf, statbuf_get_date(&sbuf));
            strcat(buf, " ");
            strcat(buf, statbuf_get_filename(&sbuf, filename));
        }
        else //NLST
        {
            strcpy(buf, statbuf_get_filename(&sbuf, filename));
        }

        strcat(buf, "\r\n");
        writen(sess->data_fd, buf, strlen(buf));
    }

    closedir(dir);
}
