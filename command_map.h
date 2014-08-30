#ifndef _COMMAND_MAP_H_
#define _COMMAND_MAP_H_

#include "session.h"

//命令映射相关函数

void do_command_map(session_t *);
void ftp_reply(session_t *, int status, const char *text);

void do_user(session_t *);
void do_pass(session_t *);
void do_cwd(session_t *);
void do_cdup(session_t *);
void do_quit(session_t *);
void do_port(session_t *);
void do_pasv(session_t *);
void do_type(session_t *);
void do_stru(session_t *);
void do_mode(session_t *);
void do_retr(session_t *);
void do_stor(session_t *);
void do_appe(session_t *);
void do_list(session_t *);
void do_nlst(session_t *);
void do_rest(session_t *);
void do_abor(session_t *);
void do_pwd(session_t *);
void do_mkd(session_t *);
void do_rmd(session_t *);
void do_dele(session_t *);
void do_rnfr(session_t *);
void do_rnto(session_t *);
void do_site(session_t *);
void do_syst(session_t *);
void do_feat(session_t *);
void do_size(session_t *);
void do_stat(session_t *);
void do_noop(session_t *);
void do_help(session_t *);


#endif  /*_COMMAND_MAP_H_*/
