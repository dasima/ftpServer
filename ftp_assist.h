#ifndef _FTP_ASSIST_H_
#define _FTP_ASSIST_H_

#include "session.h"
#include "hash.h"

extern Session_t *p_sess;
extern unsigned int num_of_clients;
extern Hash_t *ip_to_clients;
extern Hash_t *pid_to_ip;

void init_hash();
void check_permission();
void setup_signal_chld();
void print_conf();


void limit_num_clients(Session_t *sess);
void add_clients_to_hash(Session_t *sess, uint32_t ip);
void add_pid_ip_to_hash(pid_t pid, uint32_t ip);

#endif  /*_FTP_ASSIST_H_*/
