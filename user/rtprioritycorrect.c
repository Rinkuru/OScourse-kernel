// user/rtprioritycorrect.c

#include <inc/lib.h>

void work(void) {
    for (int i = 1; i < 5; i++) {
        cprintf("i = %d", i);
        for (volatile int j = 1; j < 100000; j++) {} // бурная деятельность
        sys_rt_periodic_wait();
    }
}

void
umain(int argc, char **argv) {
    uint64_t period, deadline, wcet;
    period   = 1   * 1e6;
    deadline = 0.5 * 1e6;
    wcet     = 0.1 * 1e6;
    envid_t pid1 = fork();
    if (pid1 < 0) {
        cprintf("FAIL fork\n");
    }
    if (pid1 == 0) {
        // child
        int r = sys_rt_register(period, deadline, wcet, 10, NULL); // приоритет = 10
        if (r < 0) {
            cprintf("child 1: FAIL register\n");
            exit();
        }
        work();
        exit();
    }
    envid_t pid2 = fork();
    if (pid2 < 0) {
        cprintf("FAIL fork\n");
    }
    if (pid2 == 0) {
        // child
        int r = sys_rt_register(period, deadline, wcet, 200, NULL); // приоритет = 200
        if (r < 0) {
            cprintf("child 2: FAIL register\n");
            exit();
        }
        work();
        exit();
    }
    wait(pid1);
    wait(pid2);
    cprintf("Parent: all children finished.\n");
}