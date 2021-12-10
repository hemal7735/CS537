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

// takes a host name and port number and uses those to find the server exporting the file system.
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
    if (!initialized) return -1;

    Message req, res;

    req.m_type = LOOKUP;
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

// returns some information about the file specified by inum. 
// Upon success, return 0, otherwise -1. 
// The exact info returned is defined by MFS_Stat_t. 
// Failure modes: inum does not exist.
int MFS_Stat(int inum, MFS_Stat_t *m) {
    if (!initialized) return -1;

    Message req, res;
    
    req.m_type = STAT;
    req.inum = inum;

    res = sendMessage(req);

    if (res.rc < 0) {
        return -1;
    }

    m->type = res.stat.type;
    m->size = res.stat.size;

    return 0;
}

// writes a block of size 4096 bytes at the block offset specified by block. 
// Returns 0 on success, -1 on failure. 
// Failure modes: invalid inum, invalid block, not a regular file (because you can't write to directories).
int MFS_Write(int inum, char *buffer, int block) {
    if (!initialized) return -1;

    Message req, res;

    req.m_type = WRITE;
    req.inum = inum;
    strcpy(req.buffer, buffer);
    req.block = block;

    res = sendMessage(req);

    if (res.rc < 0) {
        return -1;
    }

    return 0;
}

// reads a block specified by block into the buffer from file specified by inum. 
// The routine should work for either a file or directory; 
// directories should return data in the format specified by MFS_DirEnt_t. 
// Success: 0, failure: -1. 
// ailure modes: invalid inum, invalid block.
int MFS_Read(int inum, char *buffer, int block) {
    if (!initialized) return -1;

    Message req, res;

    req.m_type = READ;
    req.inum = inum;
    req.block = block;

    res = sendMessage(req);

    if (res.rc < 0) {
        return -1;
    }

    strcpy(buffer, res.buffer);

    return 0;
}


// makes a file (type == MFS_REGULAR_FILE) or directory (type == MFS_DIRECTORY) in the parent directory specified by pinum of name name. 
// Returns 0 on success, -1 on failure. 
// Failure modes: pinum does not exist, or name is too long. 
// If name already exists, return success (think about why).
int MFS_Creat(int pinum, int type, char *name) {
    if (!initialized) return -1;

    Message req, res;

    req.m_type = CREAT;
    req.inum = pinum;
    req.type = type;
    strcpy(req.name, name);

    res = sendMessage(req);

    if (res.rc < 0) {
        return -1;
    }

    return 0;
}

// removes the file or directory name from the directory specified by pinum. 
// 0 on success, -1 on failure. 
// Failure modes: pinum does not exist, directory is NOT empty. 
// Note that the name not existing is NOT a failure by our definition (think about why this might be).
int MFS_Unlink(int pinum, char *name) {
    if (!initialized) return -1;

    Message req, res;

    req.m_type = UNLINK;
    req.inum = pinum;
    strcpy(req.name, name);

    res = sendMessage(req);

    if (res.rc < 0) {
        return -1;
    }

    return 0;
}

 // just tells the server to force all of its data structures to disk and shutdown by calling exit(0). 
 // This interface will mostly be used for testing purposes
int MFS_Shutdown() {
    if (!initialized) return -1;
    
    Message req;

    req.m_type = SHUTDOWN;
    sendMessage(req);
    return 0;
}