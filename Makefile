CC = gcc
CFLAGS  = -g -Wall
INCLUDE_FLAGS = -I include/

default: equipment server


## COMPILING THE CLIENT
equipment:  client.o protocol.o utils.o messaging.o threads.o
	$(CC) $(CFLAGS) -pthread -o equipment obj/client.o obj/protocol.o obj/utils.o obj/messaging.o obj/threads.o

client.o:  src/client/client.c include/protocol.h include/utils.h include/messaging.h include/threads.h
	@mkdir -p obj
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c src/client/client.c -o obj/client.o

## COMPILING THE SERVER
server:  server.o protocol.o utils.o messaging.o control.o threads.o
	$(CC) $(CFLAGS) -pthread -o server obj/server.o obj/protocol.o obj/utils.o obj/messaging.o obj/control.o obj/threads.o

server.o:  src/server/server.c include/protocol.h include/utils.h include/messaging.h include/control.h include/threads.h
	@mkdir -p obj
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c src/server/server.c -o obj/server.o


## other libs
protocol.o:  src/protocol/protocol.c include/protocol.h include/utils.h
	@mkdir -p obj
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c src/protocol/protocol.c -o obj/protocol.o

messaging.o:  src/common/messaging.c include/messaging.h include/utils.h
	@mkdir -p obj
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c src/common/messaging.c -o obj/messaging.o

control.o:  src/server/control.c include/control.h
	@mkdir -p obj
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c src/server/control.c -o obj/control.o

threads.o:  src/common/threads.c include/threads.h include/utils.h
	@mkdir -p obj
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c src/common/threads.c -o obj/threads.o

utils.o:  src/common/utils.c include/utils.h
	@mkdir -p obj
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c src/common/utils.c -o obj/utils.o

## CLEANING THE FILES
clean:
	@rm -rf ./obj/* server equipment