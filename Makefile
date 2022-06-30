CC = gcc
CFLAGS  = -g -Wall
INCLUDE_FLAGS = -I include/

default: equipment server


## COMPILING THE CLIENT
equipment:  client.o protocol.o utils.o messaging.o
	$(CC) $(CFLAGS) -o equipment obj/client.o obj/protocol.o obj/utils.o obj/messaging.o

client.o:  src/client/client.c include/protocol.h include/utils.h include/messaging.h
	@mkdir -p obj
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -pthread -c src/client/client.c -o obj/client.o

## COMPILING THE SERVER
server:  server.o protocol.o utils.o
	$(CC) $(CFLAGS) -pthread -o server obj/server.o obj/protocol.o obj/utils.o

server.o:  src/server/server.c include/protocol.h include/utils.h
	@mkdir -p obj
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c src/server/server.c -o obj/server.o


## other libs
protocol.o:  src/protocol/protocol.c include/protocol.h include/utils.h
	@mkdir -p obj
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c src/protocol/protocol.c -o obj/protocol.o


messaging.o:  src/common/messaging.c include/messaging.h include/utils.h
	@mkdir -p obj
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c src/common/messaging.c -o obj/messaging.o

utils.o:  src/common/utils.c include/utils.h
	@mkdir -p obj
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c src/common/utils.c -o obj/utils.o


## CLEANING THE FILES
clean:
	@rm -rf ./obj/* server equipment