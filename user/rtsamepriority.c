#include <inc/lib.h>

void
umain(int argc, char **argv)
{
    uint64_t period=1000000, deadline=500000, wcet=100000, priority=10;
    envid_t kids[3];
    int kid_pos = 0;
    for (int k = 0; k < 3; k++) {
        envid_t pid = fork();
        if (pid == 0) {
            int r = sys_rt_register(period, deadline, wcet, priority, NULL);
            if (r < 0) panic("rt_register");

            for (int i = 0; i < 50; i++) {
                cprintf("[env %08x] i=%d\n", sys_getenvid(), i);

                // Небольшая работа, чтобы было много тиков таймера
                for (volatile int j = 0; j < 2000000; j++) {}

                sys_rt_periodic_wait();
            }
            exit();
        }
        kids[kid_pos++] = pid; 
    }

    for (int i = 0; i < kid_pos; i++) {
        wait(kids[i]);
    }
}