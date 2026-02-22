// user/schedtest.c
#include <inc/lib.h>

static void
work(void) {
    // маленькая "работа" (без таймеров)
    for (volatile int i = 0; i < 2000000; i++)
        ;
}

void
umain(int argc, char **argv) {
    envid_t child = fork();
    if (child < 0)
        panic("fork: %d", child);
    for (int i = 0; i < 100; i++) {work();}
    

    if (child == 0) {
        // параметры: period, deadline, wcet (в микросекундах)
        // (подбери так, чтобы C <= D <= P и admission control пропустил)
        uint64_t P = 2000000;  // 2s
        uint64_t D = 800000;   // 0.8s
        uint64_t C = 100000;   // 0.1s
        uint8_t priority = 1;

        int r = sys_rt_register(P, D, C, priority, 0);
        if (r < 0)
            panic("rt_register: %d", r);

        for (int k = 1; k <= 5; k++) {
            cprintf("[rt] cycle %d/5 env=%08x\n", k, thisenv->env_id);
            work();
            sys_rt_periodic_wait();   // уснуть до следующего периода
        }

        sys_rt_unregister();
        cprintf("[rt] done env=%08x\n", thisenv->env_id);
        return; // завершить ребёнка
    }

    // родитель
    wait(child);
    cprintf("[parent] child %08x finished\n", child);
}