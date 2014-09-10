#include "trans_ctrl.h"
#include "common.h"
#include "sysutil.h"
#include "configure.h"
#include "command_map.h"
#include "ftp_codes.h"
#include "strutil.h"
Session_t *p_sess = NULL;

static void handle_signal_alarm_ctrl_fd(int sig);
static void handle_signal_alarm_data_fd(int sig);
static void handle_signal_sigurg(int sig);

//限速功能
void limit_curr_rate(Session_t *sess, int nbytes, int is_upload)
{
    //获取当前时间
    int curr_time_sec = get_curr_time_sec();
    int curr_time_usec = get_curr_time_usec();

    //求时间差
    double elapsed = 0.0;
    elapsed += (curr_time_sec - sess->start_time_sec);
    elapsed += (curr_time_usec - sess->start_time_usec) / (double)1000000;
    if(elapsed < 0.000001) //double和0不能用==
        elapsed = 0.001;

    //求速度
    double curr_rate = nbytes / elapsed;

    //求比率
    double rate_radio = 0.0;
    if(is_upload)
    {
        //如果用户配置了限速，并且当前速度已经超过了限定速度
        if(sess->limits_max_upload > 0 && curr_rate > sess->limits_max_upload)
        {
            rate_radio = curr_rate / (sess->limits_max_upload);
        }
        else
        {
            //如果不限速，必须更新时间
            sess->start_time_sec = get_curr_time_sec();
            sess->start_time_usec = get_curr_time_usec();
            return;
        }
    }else
    {
        if(sess->limits_max_download > 0 && curr_rate > sess->limits_max_download)
        {
            rate_radio = curr_rate / (sess->limits_max_download);
        }
        else
        {
            //如果不限速，必须更新时间
            sess->start_time_sec = get_curr_time_sec();
            sess->start_time_usec = get_curr_time_usec();
            return;
        }
    }

    //求出限速时间
    double sleep_time = (rate_radio - 1) * elapsed;

    //限速睡眠
    if(nano_sleep(sleep_time) == -1)
        ERR_EXIT("nano_sleep");

    //注意更新当前时间
    sess->start_time_sec = get_curr_time_sec();
    sess->start_time_usec = get_curr_time_usec();
}

//开启控制连接的定时器
void setup_signal_alarm_ctrl_fd()
{
    if(signal(SIGALRM, handle_signal_alarm_ctrl_fd) == SIG_ERR)
        ERR_EXIT("signal");
}

//开始计时
void start_signal_alarm_ctrl_fd()
{
    alarm(tunable_idle_session_timeout);
}

//信号处理程序用于控制连接
static void handle_signal_alarm_ctrl_fd(int sig)
{
    if(tunable_idle_session_timeout > 0) //用户配置了此选项
    {
        //直接关闭控制连接，然后退出
        shutdown(p_sess->peer_fd, SHUT_RD);
        //421
        ftp_reply(p_sess, FTP_IDLE_TIMEOUT, "Timeout.");
        shutdown(p_sess->peer_fd, SHUT_WR);
        exit(EXIT_SUCCESS);
    }
}

//安装数据连接的定时器
void setup_signal_alarm_data_fd()
{
    if(signal(SIGALRM, handle_signal_alarm_data_fd) == SIG_ERR)
        ERR_EXIT("signal");
}

//数据连接开始计时
void start_signal_alarm_data_fd()
{
    alarm(tunable_data_connection_timeout);
}

//信号处理，用户数据连接
static void handle_signal_alarm_data_fd(int sig)
{
    if(tunable_data_connection_timeout > 0)
    {
        if(p_sess->is_translating_data == 1)
        {
            //有数据传输则重新启动定时器
            start_signal_alarm_data_fd();
        }
        else
        {
            //没有数据则给421，并且退出
            close(p_sess->data_fd);
            shutdown(p_sess->peer_fd, SHUT_RD);
            ftp_reply(p_sess, FTP_DATA_TIMEOUT, "Timeout.");
            shutdown(p_sess->peer_fd, SHUT_WR);
            exit(EXIT_SUCCESS);
        }
    }
}

//取消定时器
void cancel_signal_alarm()
{
    alarm(0);
}

//处理SIGURG信号，实质是处理带外数据
static void handle_signal_sigurg(int sig)
{
    char cmdline[1024] = {0};
    int ret = readline(p_sess->peer_fd, cmdline, sizeof cmdline);
    if(ret <= 0) //存在带外数据，不可能为0
        ERR_EXIT("readline");

    str_trim_crlf(cmdline);
    str_upper(cmdline);

    if(strcmp("ABOR", cmdline) == 0 || strcmp("\377\364\377\362ABOR", cmdline) == 0)
    {
        //处理abor指令
        p_sess->is_receive_abor = 1;
        close(p_sess->data_fd);
        p_sess->data_fd = -1;
    }
    else
    {
        //未识别的命令
        ftp_reply(p_sess, FTP_BADCMD, "Unknown command.");
    }
}

//安装sigurg信号
void setup_signal_sigurg()
{
    if(signal(SIGURG, handle_signal_sigurg) == SIG_ERR)
        ERR_EXIT("signal");
}

void do_site_chmod(Session_t *sess, char *args)
{
   if (strlen(args) == 0)
   {
       ftp_reply(sess, FTP_BADCMD, "SITE CHMOD needs 2 arguments.");
       return;
   }

   char perm[100] = {0};
   char file[100] = {0};
   str_split(args , perm, file, ' ');
   if (strlen(file) == 0)
   {
       ftp_reply(sess, FTP_BADCMD, "SITE CHMOD needs 2 arguments.");
       return;
   }

   unsigned int mode = str_octal_to_uint(perm);
   if (chmod(file, mode) < 0)
   {
       ftp_reply(sess, FTP_CHMODOK, "SITE CHMOD command failed.");
   }
   else
   {
       ftp_reply(sess, FTP_CHMODOK, "SITE CHMOD command ok.");
   }
}

void do_site_umask(Session_t *sess, char *args)
{
     // SITE UMASK [umask]
   if (strlen(args) == 0)
   {
       char text[1024] = {0};
       sprintf(text, "Your current UMASK is 0%o", tunable_local_umask);
       ftp_reply(sess, FTP_UMASKOK, text);
   }
   else
   {
       unsigned int um = str_octal_to_uint(args);
       umask(um);
       char text[1024] = {0};
       sprintf(text, "UMASK set to 0%o", um);
       ftp_reply(sess, FTP_UMASKOK, text);
   }
}

void do_site_help(Session_t *sess)
{
     //214 CHMOD UMASK HELP
   ftp_reply(sess, FTP_HELP, "CHMOD UMASK HELP");
}
