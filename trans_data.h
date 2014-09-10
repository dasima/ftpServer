#ifndef _TRANS_DATA_H_
#define _TRANS_DATA_H_

#include "session.h"

void upload_file(Session_t *sess, int is_appe);
int get_trans_data_fd(Session_t *sess);
void trans_list(Session_t *sess, int list);

#endif  /*_TRANS_DATA_H_*/
