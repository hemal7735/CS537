#include <stdio.h>
#include "udp.h"
#include "lfs.h"
#include "message.h"

#define _BUFFER_SIZE (1000)

void Server_listen(int port);

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

    char buffer[NUM_ENTRIES * sizeof(MFS_DirEnt_t)];

    rc = Read(0, buffer, 0);
    if (rc < 0) {
        printf("Read failed\n");
    } else {
        printf("Read: %s\n", buffer);
    }

    MFS_DirEnt_t dir_entries[NUM_ENTRIES];
    memcpy(dir_entries, buffer, NUM_ENTRIES * sizeof(MFS_DirEnt_t));

    // rc = Read(foo_inode, buffer, 0);

    // rc = Unlink(0, foo);

    // if (rc < 0) {
    //     printf("deletion of foo failed\n");
    // }

    // rc = Unlink(foo_inode, bar);
    // if (rc < 0) {
    //     printf("deletion of bar failed\n");
    // } else {
    //     printf("bar deleted \n");
    // }

    // rc = Unlink(0, foo);

    // if (rc < 0) {
    //     printf("deletion of foo failed\n");
    // } else {
    //     printf("foo deleted \n");
    // }

    Shutdown();
}

// server code
int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: server [portnum] [file-system-image]\n");
        exit(1);
    }

    Startup(argv[2]);
    // test();
    Server_listen(atoi(argv[1]));
    
    return 0;
}

void Server_listen(int port) {
    int sd = UDP_Open(port);
    assert(sd > -1);

    while (1)
    {
        struct sockaddr_in addr;
        Message req, res;
        
        res.m_type = REPLY;

        // printf("server:: waiting...\n");
        int rc = UDP_Read(sd, &addr, (char *)&req, sizeof (req));
        // printf("server:: read message [m_type:(%u)]\n", req.m_type);
        
        if (rc > 0)
        {
            int inum;

            switch (req.m_type) {
                case LOOKUP:
                    rc = Lookup(req.inum, req.name);
                    res.rc = rc;

                    if (rc != -1) {
                        res.inum = rc;
                    }

                    break;
                case STAT:
                    rc = Stat(req.inum, &(res.stat));
                    res.rc = rc;

                    break;
                case WRITE:
                    rc = Write(req.inum, req.buffer, req.block);
                    res.rc = rc;
                    break;
                case READ:
                    rc = Read(req.inum, res.buffer, req.block);
                    res.rc = rc;

                    break;
                case CREAT:
                    rc = Creat(req.inum, req.type, req.name);
                    res.rc = rc;

                    break;
                case UNLINK:
                    rc = Unlink(req.inum, req.name);
                    res.rc = rc;

                    break;
                case SHUTDOWN:
                    // for shutdown there is no action
                    res.rc = 0;
                    break;

                default:
                    break;
            }
            
            rc = UDP_Write(sd, &addr, (char *)&res, sizeof(res));

            if (req.m_type == SHUTDOWN) {
                Shutdown();
            }
        }
    }
}
