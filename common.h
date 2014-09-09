#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <pwd.h>
#include <shadow.h>
#include "crypt.h"
#include <dirent.h>
#include <time.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/capability.h>
#include <sys/syscall.h>
#include <bits/syscall.h>

#define ERR_EXIT(m) \
    do { \
        perror(m);\
        exit(EXIT_FAILURE);\
    }while(0)

#endif  /*_COMMON_H_*/
