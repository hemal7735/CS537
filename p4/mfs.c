#include <string.h>
#include <stdio.h>
#include "udp.h"
#include "mfs.h"
#include "message.h"

char *_hostname;
int _port;
int initialized = 0;

Message sendMessage(Message req) {
    struct sockaddr_in addrSnd, addrRcv;

    int sd = UDP_Open(20000);
    int rc = UDP_FillSockAddr(&addrSnd, _hostname, _port);

    rc = UDP_Write(sd, &addrSnd, (char *)&req, sizeof(req));
    if (rc < 0) {
        printf("client:: failed to send\n");
        exit(1);
    }

    printf("client:: wait for reply...\n");
    Message res;

    rc = UDP_Read(sd, &addrRcv, (char *)&res, sizeof(res));
    printf("client:: got reply [size:%d m_type:(%u)\n", rc, res.m_type);

    return res;
}

int MFS_Init(char *hostname, int port) {
    // store values
    _port = port;
    _hostname = strdup(hostname);

    initialized = 1;
    return 0;
}

// takes the parent inode number (which should be the inode number of a directory) 
// and looks up the entry name in it. 
// The inode number of name is returned. 
// Success: return inode number of name; 
// failure: return -1. 
//      Failure modes: invalid pinum, name does not exist in pinum.
int MFS_Lookup(int pinum, char *name) {
    Message req, res;
    
    req.type = LOOKUP;
    req.inum = pinum;
    strcpy(req.name, name);

    res = sendMessage(req);

    if (res.rc < 0) {
        return -1;
    } else {
        return res.inum;
    }

    return res.rc;
}
int MFS_Stat(int inum, MFS_Stat_t *m) {
    return 0;
}
int MFS_Write(int inum, char *buffer, int block) {
    return 0;
}
int MFS_Read(int inum, char *buffer, int block) {
    return 0;
}
int MFS_Creat(int pinum, int type, char *name) {
    return 0;
}
int MFS_Unlink(int pinum, char *name) {
    return 0;
}
int MFS_Shutdown() {
    return 0;
}