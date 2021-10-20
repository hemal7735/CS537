#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int
main(int argc, char *argv[])
{
    if (argc != 2) exit();
    int tickets = atoi(argv[1]);

    settickets(tickets);
 
    while(1) {
        
    }

    exit();

    return 0;
}
