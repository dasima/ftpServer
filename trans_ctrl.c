#include "trans_ctrl.h"
#include "common.h"
#include "sysutil.h"
#include "configure.h"
#include "command_map.h"
#include "ftp_codes.h"
Session_t *p_sess = NULL;

static void handle_signal_alarm_ctrl_fd(int sig);
static void handle_signal_alarm_data_fd(int sig);


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
