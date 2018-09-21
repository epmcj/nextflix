all: nextflix client

nextflix: nextflix.c server.o
	gcc -pthread -o server server.o nextflix.c

client:
	gcc -o client client.c

server.o: server.c server.h
	gcc -c server.c