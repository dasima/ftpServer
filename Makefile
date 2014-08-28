.PHONY:clean
CC=gcc
CFLAGS=-Wall -g
BIN=ftpServer
OBJS=sysutil.o main.o session.o strutil.o ftp_nobody.o ftp_proto.o
#OTHER=-std=c++0x
#LINKS=-lecho -lpthread
$(BIN):$(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ 
#	$(CC) $(CFLAGS) $^ -o $@ $(LINKS)
%.o:%.cc
	$(CC) $(CFLAGS) -c $< -o $@ 
#	$(CC) $(CFLAGS) -c $< -o $@ $(OTHER)
clean:
	rm -f *.o $(BIN)
