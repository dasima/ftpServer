#ifndef _TRANS_CTRL_H_
#define _TRANS_CTRL_H_

/*
 *限速模块
 */
#include "session.h"

void limit_curr_rate(Session_t *sess, int nbytes, int is_upload);


#endif  /*_TRANS_CTRL_H_*/
