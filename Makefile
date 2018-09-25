all: client nextflix

nextflix: nextflix.c server.o socket_sup.o
	gcc -pthread -o server server.o socket_sup.o nextflix.c

client: client.c socket_sup.o
	gcc -o client socket_sup.o client.c

server.o: server.c server.h
	gcc -c server.c

socket_sup.o: socket_sup.c socket_sup.h
	gcc -c socket_sup.c