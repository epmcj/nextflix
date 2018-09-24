all: client nextflix

nextflix: nextflix.c server.o
	gcc -pthread -o server server.o nextflix.c

client: client.c
	gcc -o client client.c

server.o: server.c server.h
	gcc -c server.c