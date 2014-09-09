#ifndef _PRIV_COMMAND_H_
#define _PRIV_COMMAND_H_

#include "session.h"

void privop_pasv_get_data_sock(Session_t *sess);
void privop_pasv_active(Session_t *sess);
void privop_pasv_listen(Session_t *sess);
void privop_pasv_accept(Session_t *sess);

#endif  /*_PRIV_COMMAND_H_*/
