CC = gcc
CFLAGS  = -g -Wall
INCLUDE_FLAGS = -I include/

default: client server


## COMPILING THE CLIENT
client:  client.o
	$(CC) $(CFLAGS) -o client obj/client.o

client.o:  src/client/client.c
	@mkdir -p obj
	$(CC) $(CFLAGS) -pthread -c src/client/client.c -o obj/client.o

## COMPILING THE SERVER
server:  server.o
	$(CC) $(CFLAGS) -pthread -o server obj/server.o

server.o:  src/server/server.c
	@mkdir -p obj
	$(CC) $(CFLAGS) -c src/server/server.c -o obj/server.o

## CLEANING THE FILES
clean:
	@rm -rf ./obj/* client server