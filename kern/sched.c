#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/env.h>
#include <kern/monitor.h>


struct Taskstate cpu_ts;
_Noreturn void sched_halt(void);

/* Choose a user environment to run and run it */
_Noreturn void
sched_yield(void) {
    /* Implement simple round-robin scheduling.
     *
     * Search through 'envs' for an ENV_RUNNABLE environment in
     * circular fashion starting just after the env was
     * last running.  Switch to the first such environment found.
     *
     * If no envs are runnable, but the environment previously
     * running is still ENV_RUNNING, it's okay to
     * choose that environment.
     *
     * If there are no runnable environments,
     * simply drop through to the code
     * below to halt the cpu */

    // LAB 3: Your code here:
    if (curenv && curenv->env_status == ENV_DYING) {  //освобождаем если зомби
        env_free(curenv);
        curenv = NULL;
    }

    /* ====================================================
     * itask: ищем RT-процесс с наивысшим приоритетом, 
     * среди них используем порядок FIFO.
     * Если RT-процессов нет, выбираем по round-robin
     * обычные процессы.
     * ==================================================== */
    struct Env *edf_best = NULL;

    for (int priority = ARINC_MAX_PRIORITY; priority >= ARINC_MIN_PRIORITY; priority--) {
        if (rt_priority_queues[priority].first == NULL) { // Пустая очередь
            continue;
        }
        
        struct queue *q = &rt_priority_queues[priority];
         // Пытаемся найти валидный env, вычищая мусор с головы очереди
        while (q->first != NULL) {
            struct Env *e = q->first;

            // pop head
            q->first = e->queue_next;
            if (!q->first)
                q->last = NULL;
            e->queue_next = NULL;

            // Проверяем валидность кандидата
            // Нам нужен env, который реально может быть запущен сейчас
            if (e->env_is_rt && (e->env_status == ENV_RUNNABLE || e->env_status == ENV_RUNNING)) {
                edf_best = e;
                break;
            }
        }

        if (edf_best)
            break;
    }

    if (edf_best) {
        cprintf("RT pick %08x status=%d is_rt=%d\n", edf_best->env_id, edf_best->env_status, edf_best->env_is_rt);
        env_run(edf_best);
    }

    // Round Robin
    int start_idx = curenv ? (curenv - envs + 1) % NENV : 0;
    for (int i = start_idx; i < NENV; ++i) {
        if (envs[i].env_status == ENV_RUNNABLE && !envs[i].env_is_rt) {
            env_run(&envs[i]);
        }
    }
    for (int i = 0; i < start_idx; ++i) {
        if (envs[i].env_status == ENV_RUNNABLE && !envs[i].env_is_rt) {
            env_run(&envs[i]);
        }
    }
    if (curenv && curenv->env_status == ENV_RUNNING && !curenv->env_is_rt) {
        env_run(curenv);
    }

    cprintf("Halt\n");

    /* No runnable environments,
     * so just halt the cpu */
    sched_halt();
}

/* Halt this CPU when there is nothing to do. Wait until the
 * timer interrupt wakes it up. This function never returns */
_Noreturn void
sched_halt(void) {

    /* For debugging and testing purposes, if there are no runnable
     * environments in the system, then drop into the kernel monitor */
    int i;
    for (i = 0; i < NENV; i++)
        if (envs[i].env_status == ENV_RUNNABLE ||
            envs[i].env_status == ENV_RUNNING) break;
    if (i == NENV) {
        cprintf("No runnable environments in the system!\n");
        for (;;) monitor(NULL);
    }

    /* Mark that no environment is running on CPU */
    curenv = NULL;

    /* Reset stack pointer, enable interrupts and then halt */
    asm volatile(
            "movq $0, %%rbp\n"
            "movq %0, %%rsp\n"
            "pushq $0\n"
            "pushq $0\n"
            "sti\n"
            "hlt\n" ::"a"(cpu_ts.ts_rsp0));

    /* Unreachable */
    for (;;)
        ;
}
