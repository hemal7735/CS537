#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "pstat.h"
#include "param.h"

void compute() {
    int count = 0;

    for(int i = 0; i < 10000000; i++) {
        count += i;
    }
}

int
main(int argc, char *argv[])
{
    if (argc != 4) exit();

    int tickets_parent = atoi(argv[1]);
    int tickets_child1 = atoi(argv[2]);
    int tickets_child2 = atoi(argv[3]);

    // int pid_A = getpid();
    int pid_B = fork();
    int pid_C = 0;

    if (pid_B < 0) {
        exit();
    } else if (pid_B == 0) {
        // child
        settickets(tickets_child1);

        pid_C = fork();

        if (pid_C < 0) {
            exit();
        } else if (pid_C == 0) {
            settickets(tickets_child2);
            compute();
            // printf(1, "child2 [A:%d] [B:%d] [C:%d]\n", pid_A, pid_B, pid_C);

        } else {
            compute();
            wait();
            // printf(1, "child1 [A:%d] [B:%d] [C:%d]\n", pid_A, pid_B, pid_C);
        }
    } else {
        // parent
        settickets(tickets_parent);

        compute();
        wait();

        // printf(1, "parent [A:%d] [B:%d] [C:%d]\n", pid_A, pid_B, pid_C);

        struct pstat *procstats = (struct pstat *)malloc(sizeof(struct pstat));
        
        getpinfo(procstats);

        for(int i = 2; i < 5; i++) {
            printf(1, "\n[in-use:%d] [tickets:%d] [pid:%d] [ticks:%d]\n", procstats->inuse[i], procstats->tickets[i], procstats->pid[i], procstats->ticks[i]);
        } 
    }

    exit();

    return 0;
}
