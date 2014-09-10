#ifndef _TRANS_DATA_H_
#define _TRANS_DATA_H_

#include "session.h"

void download_file(Session_t *sess);
void upload_file(Session_t *sess, int is_appe);
void trans_list(Session_t *sess, int list);

#endif  /*_TRANS_DATA_H_*/
