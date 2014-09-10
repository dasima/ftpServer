.PHONY:clean
CC=gcc
CFLAGS=-Wall -g
BIN=ftpServer
OBJS=main.o sysutil.o session.o strutil.o ftp_nobody.o ftp_proto.o configure.o parse_conf.o command_map.o trans_data.o priv_sock.o priv_command.o trans_ctrl.o hash.o ftp_assist.o
LIB=-lcrypt
$(BIN):$(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIB)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f *.o $(BIN)
