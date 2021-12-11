all: client server

server:
	gcc server.c udp.c lfs.c -o server

# Compile libs - for some reason this needs to be done on Linux
# On Mac steps are much simpler
udp-compile:
	gcc -fPIC -g -c -Wall udp.c
mfs-compile:
	gcc -fPIC -g -c -Wall mfs.c

libmfs.so: udp-compile mfs-compile
	gcc -shared -o libmfs.so udp.o mfs.o -lc

client: libmfs.so
	gcc -o client client.c -L. -lmfs

clean:
	rm -rf libmfs.so client server

refresh: clean all
