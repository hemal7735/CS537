#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int
main(int argc, char *argv[])
{
    // if (argc != 2) exit();

    // int tickets = atoi(argv[1]);
    // int loopCounter = atoi(argv[2]);

    int rc = fork();

    if (rc < 0) {
        // somtehitn wrong
    } else if (rc == 0) {
        // child
        settickets(2);
    } else {
        // parent
        settickets(4);
    }
    
    int count = 0;

    for(int i = 0; i < 100000000; i++) {
        count += i;
        // printf(1, "val - [i-%d]\n", i); 
    }


    // while(1) {

    // }

    printf(1, "spin - [count-%d]\n", count);
    
    if (rc != 0) {
        wait();
    }

    exit();

    return 0;
}
