#ifndef _TRANS_CTRL_H_
#define _TRANS_CTRL_H_

/*
 *限速模块
 */
#include "session.h"

void limit_curr_rate(Session_t *sess, int nbytes, int is_upload);

//控制连接
void setup_signal_alarm_ctrl_fd();
void start_signal_alarm_ctrl_fd();

//数据连接
void setup_signal_alarm_data_fd();
void start_signal_alarm_data_fd();

void cancel_signal_alarm();

void setup_signal_sigurg();

#endif  /*_TRANS_CTRL_H_*/
