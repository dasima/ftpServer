#ifndef _CONFIGURE_H_
#define _CONFIGURE_H_

extern int tunable_pasv_enable;
extern int tunable_port_enable;
extern unsigned int tunable_listen_port;
extern unsigned int tunable_pasv_port_min;
extern unsigned int tunable_pasv_port_max;
extern unsigned int tunable_max_clients;
extern unsigned int tunable_max_per_ip;
extern unsigned int tunable_accept_timeout;
extern unsigned int tunable_connect_timeout;
extern unsigned int tunable_idle_session_timeout;
extern unsigned int tunable_data_connection_timeout;
extern unsigned int tunable_local_umask;
extern unsigned int tunable_upload_max_rate;
extern unsigned int tunable_download_max_rate;
extern const char *tunable_listen_address;

#endif  /*_CONFIGURE_H_*/
