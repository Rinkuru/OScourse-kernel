// user/rtregok.c
#include <inc/lib.h>


void deadline_missed_handler(void) {
    cprintf("Deadline missed handler was called!\n");
}

void
umain(int argc, char **argv)
{
    uint64_t period, deadline, wcet, priority;
    period = 2000000;
    deadline = 1000000;
    wcet = 960000;
    priority = 5;
    int r = sys_rt_register(period, deadline, wcet, priority, deadline_missed_handler);
    if (r == 0)
        cprintf("OK: registered\n");
    else
        cprintf("FAIL: ret=%d\n", r);
}