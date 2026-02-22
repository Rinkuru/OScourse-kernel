// user/admissionctrlfail.c

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
    uint64_t deadline, wcet, period, priority;
    deadline = 1.0 * 1e6;
    wcet     = 0.05 * 1e6;
    period   = 1.0 * 1e6;
    priority = 0;

    envid_t kids[20];
    int nkids = 0;

    for (int i = 1; i <= 20; i++) {
        priority += 1;
        envid_t pid = fork();
        if (pid < 0) {
            cprintf("%d FAIL: fork (%d)\n", i, pid);
            return;
        }

        if (pid == 0) {
            // child
            int r = sys_rt_register(period, deadline, wcet, priority, 0);
            if (r < 0) {
                cprintf("%d FAIL: unregistered (%d)\n", i, r);
                if (r == -E_NO_FREE_ENV) {
                    cprintf("Admission control fail\n");
                }
            } else {
                cprintf("%d OK: registered\n", i);
                work();
            }
            exit();
        }
        kids[nkids++] = pid;
    }
    for (int k = 0; k < nkids; k++) {
        wait(kids[k]);
    }

    cprintf("Parent: all children finished.\n");
}