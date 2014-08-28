#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ERR_EXIT(m) \
    do { \
        perror(m);\
        exit(EXIT_FAILURE);\
    }while(0)

int main(int argc, const char *argv[])
{
    int fds[2];
    if(socketpair(PF_UNIX, SOCK_STREAM, 0, fds) == -1)
        ERR_EXIT(EXIT_FAILURE);

    int fd = open("test.txt", O_RDWR, 0755);
    if(fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    pid_t pid;
    if((pid = fork()) < 0)
        ERR_EXIT("fork");
    if(pid == 0)
    {
        printf("child\n");
        close(fds[0]);

        write(fd, "hello\n", sizeof("hello\n"));
        close(fds[1]);
    }
    else if(pid > 0)
    {
        close(fds[1]);
        printf("parent\n");
        sleep(1);
        char buf[128];
        read(fd, buf, sizeof(buf));
        printf("%s\n", buf);
        close(fds[0]);
    }
    return 0;
}
