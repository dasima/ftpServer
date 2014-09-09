#ifndef _COMMAND_MAP_H_
#define _COMMAND_MAP_H_

#include "session.h"

//命令映射相关函数

void do_command_map(Session_t *);
void ftp_reply(Session_t *, int, const char *);
void ftp_lreply(Session_t *, int, const char *);

void do_user(Session_t *);
void do_pass(Session_t *);
void do_cwd(Session_t *);
void do_cdup(Session_t *);
void do_quit(Session_t *);
void do_port(Session_t *);
void do_pasv(Session_t *);
void do_type(Session_t *);
void do_stru(Session_t *);
void do_mode(Session_t *);
void do_retr(Session_t *);
void do_stor(Session_t *);
void do_appe(Session_t *);
void do_list(Session_t *);
void do_nlst(Session_t *);
void do_rest(Session_t *);
void do_abor(Session_t *);
void do_pwd(Session_t *);
void do_mkd(Session_t *);
void do_rmd(Session_t *);
void do_dele(Session_t *);
void do_rnfr(Session_t *);
void do_rnto(Session_t *);
void do_site(Session_t *);
void do_syst(Session_t *);
void do_feat(Session_t *);
void do_size(Session_t *);
void do_stat(Session_t *);
void do_noop(Session_t *);
void do_help(Session_t *);

#endif  /*_COMMAND_MAP_H_*/
