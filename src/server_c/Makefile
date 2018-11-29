objects  = server.o video.o message.o tools.o
exec_dir = ../../

all: nextflix clean

nextflix: nextflix.c $(objects)
	gcc -pthread -o $(exec_dir)nextflix $(objects) nextflix.c

server.o: server.c server.h video.o message.o tools.o
	gcc -c tools.o video.o message.o server.c 
	
video.o: video.c video.h
	gcc -c video.c 

message.o: message.c message.h
	gcc -c message.c 

# socket_sup.o: socket_sup.c socket_sup.h
# 	gcc -c socket_sup.c

tools.o: tools.c tools.h
	gcc -c tools.c

clean:
	rm *.o