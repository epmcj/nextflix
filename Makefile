all: client nextflix

nextflix: nextflix.c server.o socket_sup.o video.o message.o tools.o
	gcc -pthread -o server server.o socket_sup.o video.o tools.o message.o nextflix.c

# client: client.c socket_sup.o tools.o
# 	gcc -o client socket_sup.o tools.o client.c

server.o: server.c server.h video.o message.o tools.o
	gcc -c tools.o video.o message.o server.c 
	
video.o: video.c video.h
	gcc -c video.c 

message.o: message.c message.h
	gcc -c message.c 

socket_sup.o: socket_sup.c socket_sup.h
	gcc -c socket_sup.c

tools.o: tools.c tools.h
	gcc -c tools.c