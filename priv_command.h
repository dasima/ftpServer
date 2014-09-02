#ifndef _PRIV_COMMAND_H_
#define _PRIV_COMMAND_H_

#include "session.h"

/*
 *被动模式模块
 */

void privop_pasv_get_data_sock(session_t *);
void privop_pasv_active(session_t *);
void privop_pasv_listen(session_t *);
void privop_pasv_accept(session_t *);

#endif  /*_PRIV_COMMAND_H_*/
