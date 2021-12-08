#include <stdio.h>
#include "udp.h"
#include "message.h"

#define _BUFFER_SIZE (1000)

// client code
int main(int argc, char *argv[]) {
    struct sockaddr_in addrSnd, addrRcv;

    int sd = UDP_Open(20000);
    int rc = UDP_FillSockAddr(&addrSnd, "localhost", 10000);

    Message msg;
    msg.m_type = LOOKUP;

    rc = UDP_Write(sd, &addrSnd, (char *)&msg, sizeof(msg));
    if (rc < 0) {
        printf("client:: failed to send\n");
        exit(1);
    }

    printf("client:: wait for reply...\n");
    rc = UDP_Read(sd, &addrRcv, (char *)&msg, sizeof(msg));
    printf("client:: got reply [size:%d m_type:(%u)\n", rc, msg.m_type);
    return 0;
}

