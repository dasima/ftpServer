#include "trans_ctrl.h"
#include "common.h"
#include "sysutil.h"
#include "configure.h"

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
