CCC = GCC
CCFLAGS = - ansi

client: client.o
	gcc client.o -o client367

client.o: client.c
	gcc -c client.c

server: server.o
	gcc server.o -o server367

server.o: server.c
	gcc -c server.c

pipe: pipe.o
	gcc pipe.o -o pipe

pipe.o: pipe.c
	gcc -c pipe.c

clean:
	rm -rf *o client367 server367 pipe
