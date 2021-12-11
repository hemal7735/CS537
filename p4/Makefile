all: libmfs.so client server

server:
	gcc server.c udp.c lfs.c -o server

libmfs.so:
	gcc -shared -fPIC -o libmfs.so mfs.c udp.c

client: libmfs.so
	gcc -L. -lmfs -o client client.c

clean:
	rm -rf libmfs.so client server

refresh: clean all
