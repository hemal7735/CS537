#include <stdio.h>
#include "udp.h"
#include "message.h"
#include "lfs.h"

#define _BUFFER_SIZE (1000)

void test() {
    // create /foo
    char foo[] = "foo";
    char bar[] = "bar";

    int rc = Creat(0, 0, foo);
    if (rc < 0) {
        printf("/foo failed\n");
    }

    int foo_inode = Lookup(0, foo);
    printf("/foo inum is:%d\n", foo_inode);

    // create "/foo/bar"
    rc = Creat(foo_inode, 1, bar);
    if (rc < 0) {
        printf("/bar failed\n");
    }

    int bar_inode = Lookup(foo_inode, bar);
    printf("/bar inum is:%d\n", bar_inode);

    // char buffer[10000];

    // rc = Read(0, buffer, 0);
    // if (rc < 0) {
    //     printf("Read failed\n");
    // } else {
    //     printf("Read: %s\n", buffer);
    // }

    // rc = Read(foo_inode, buffer, 0);

    rc = Unlink(0, foo);

    if (rc < 0) {
        printf("deletion of foo failed\n");
    }

    rc = Unlink(foo_inode, bar);
    if (rc < 0) {
        printf("deletion of bar failed\n");
    } else {
        printf("bar deleted \n");
    }

    rc = Unlink(0, foo);

    if (rc < 0) {
        printf("deletion of foo failed\n");
    } else {
        printf("foo deleted \n");
    }

    Shutdown();
}

// server code
int main(int argc, char *argv[])
{
    char file[] = "abc.dmg";
    Startup(file);
    test();
    // server_listen(10000);

    

    return 0;
}

void Server_listen(int port) {
    int sd = UDP_Open(port);
    assert(sd > -1);

    while (1)
    {
        struct sockaddr_in addr;
        Message request;
        
        printf("server:: waiting...\n");
        int rc = UDP_Read(sd, &addr, (char *)&request, sizeof (request));
        printf("server:: read message [m_type:(%u)]\n", request.m_type);
        
        if (rc > 0)
        {
            int inum;

            switch (request.m_type) {
                case LOOKUP:
                    rc = Lookup(request.inum, request.name);
                    break;
                case STAT:
                    rc = Stat(request.inum, &(request.stat));
                    break;
                case WRITE:
                    rc = Write(request.inum, request.buffer, request.block);
                    break;
                case READ:
                    rc = Read(request.inum, request.buffer, request.block);
                    break;
                case CREAT:
                    rc = Creat(request.inum, request.type, request.name);
                    break;
                case UNLINK:
                    rc = Unlink(request.inum, request.name);
                    break;
                case SHUTDOWN:
                    Shutdown();
                    break;

                default:
                    break;
            }

            Message response;
            response.m_type = REPLY;
            rc = UDP_Write(sd, &addr, (char *)&response, sizeof(response));
            printf("server:: replied\n");
        }
    }
}
