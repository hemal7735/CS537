#include <stdio.h>
#include "mfs.h"

#define _BUFFER_SIZE (1000)

// client code
int main(int argc, char *argv[]) {
    char localhost[] = "localhost";
    char foo[] = "foo";
    int port;

    MFS_Init(localhost, 10000);
    int inum = MFS_Lookup(0, foo);

    printf("[inum:%d]\n", inum);

    return 0;
}

