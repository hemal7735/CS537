#include <stdio.h>
#include "udp.h"
#include "message.h"

#define _BUFFER_SIZE (1000)

// server code
int main(int argc, char *argv[])
{
    int sd = UDP_Open(10000);
    assert(sd > -1);
    while (1)
    {
        struct sockaddr_in addr;
        // char message[BUFFER_SIZE];
        Message message;
        printf("server:: waiting...\n");
        int rc = UDP_Read(sd, &addr, (char *)&message, sizeof (message));
        printf("server:: read message [m_type:(%u)]\n", message.m_type);
        
        if (rc > 0)
        {
            switch (message.m_type) {
                case LOOKUP:
                    break;
                case STAT:
                    break;
                case WRITE:
                    break;
                case READ:
                    break;
                case CREAT:
                    break;
                case UNLINK:
                    break;
                case SHUTDOWN:
                    break;

                default:
                    break;
            }

            Message reply;
            reply.m_type = REPLY;
            rc = UDP_Write(sd, &addr, (char *)&reply, sizeof(reply));
            printf("server:: replied\n");
        }
    }
    return 0;
}
