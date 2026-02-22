// user/rtdeadlinemiss.c
#include <inc/lib.h>



void deadline_missed_handler(void) {
    cprintf("Deadline missed handler was called!\n");
}

void
umain(int argc, char **argv)
{
    uint64_t period, deadline, wcet, priority;
    period = 2000000;
    deadline = 100000;
    wcet = 80000;
    priority = 10;
    int r = sys_rt_register(period, deadline, wcet, priority, deadline_missed_handler);
    if (r == 0){
        cprintf("OK: registered\n");
        // uint64_t now = sys_gettime();
        for(volatile int i = 0; i < 200000000; i++) {
            i += 1;
            i -= 1;
        }
        //sys_rt_periodic_wait();
    }
    else
        cprintf("FAIL: ret=%d\n", r);
}