#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "pstat.h"
#include "param.h"

int
main(int argc, char *argv[])
{
    if (argc != 3) exit();

    int tickets_child = atoi(argv[1]);
    int tickets_parent = atoi(argv[2]);

    int rc = fork();

    if (rc < 0) {
        // something wrong
        exit();
    } else if (rc == 0) {
        // child
        settickets(tickets_child);
    } else {
        // parent
        settickets(tickets_parent);
    }
    
    int count = 0;

    for(int i = 0; i < 100000000; i++) {
        count += i;
    }

    if (rc != 0) {
        // wait for child to exit
        wait();

        struct pstat *procstats = (struct pstat *)malloc(sizeof(struct pstat));
        
        getpinfo(procstats);

        for(int i = 0; i < NPROC; i++) {
            // printf(1, "[in-use:%d] [tickets:%d] [pid:%d] [ticks:%d]\n", procstats->inuse[i], procstats->tickets[i], procstats->pid[i], procstats->ticks[i]);
        } 
    }

    exit();

    return 0;
}
