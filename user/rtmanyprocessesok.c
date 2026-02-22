// user/rtmanyprocessesok.c

#include <inc/lib.h>

void
umain(int argc, char **argv)
{
    const uint64_t period = 1000000;
    const uint64_t wcet = period / 1000;
    const uint64_t deadline = period;

    // Попробуем зарегистрировать 1000 процессов, поставив им малый wcet
    for (int i = 1; i <= 1000; i++) {
        envid_t pid = fork();
        if (pid < 0) {
            cprintf("%d FAIL: fork (%d)\n", i, pid);
            return;
        }

        if (pid == 0) {
            // child: регистрируемся в RT и печатаем i + OK: registered
            int r = sys_rt_register(period, deadline, wcet, (i % 100) + 1, 0);
            if (r < 0) {
                cprintf("%d FAIL: unregistered (%d)\n", i, r);
            } else {
                cprintf("%d OK: registered\n", i);
            }
            // чтобы дети не продолжали цикл и не плодили процессы
            exit();
        }

        // parent продолжает создавать следующих детей
    }

    exit();
}