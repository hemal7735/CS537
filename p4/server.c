#include <stdio.h>
#include "udp.h"
#include "message.h"
#include "lfs.h"

#define _BUFFER_SIZE (1000)

// server code
int main(int argc, char *argv[])
{
    Startup("abc.img");
    int rc = Lookup(0, "..");
    printf("rc:%d\n", rc);
    // server_listen(10000);
    Shutdown();

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
