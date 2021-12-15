all: libmfs.c mfs.h udp.o
	gcc -g -c -fPIC libmfs.c -Wall -Werror 
	gcc -c -fPIC udp.c -Wall -Werror 
	gcc -shared -o libmfs.so libmfs.o udp.o -fPIC
	gcc server.c -Wall -Werror -o server udp.o
	gcc client.c -Wall -Werror -o client udp.o libmfs.o
server: server.c udp.o
	gcc server.c -Wall -Werror -o server udp.o
clean:
	rm -rf *.o *.so server client
client: client.c udp.o
	gcc client.c -Wall -Werror -o client udp.o