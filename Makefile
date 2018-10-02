all: client nextflix

nextflix: nextflix.c server.o socket_sup.o tools.o
	gcc -pthread -o server server.o socket_sup.o tools.o nextflix.c

client: client.c socket_sup.o tools.o
	gcc -o client socket_sup.o tools.o client.c

server.o: server.c server.h tools.o
	gcc -c tools.o server.c 

socket_sup.o: socket_sup.c socket_sup.h
	gcc -c socket_sup.c

tools.o: tools.c tools.h
	gcc -c tools.c