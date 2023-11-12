#include "uthread.h"
#include "user.h"

struct uthread uthread[MAX_UTHREADS];
struct uthread *currentThread;
struct uthread garb;
static int started = 0;

struct uthread *high_uthread[MAX_UTHREADS];


int uthread_create(void (*start_func)(), enum sched_priority priority)
{
    struct uthread *u;
    for (u = uthread; u < &uthread[MAX_UTHREADS]; u++) {
        if (u->state == FREE) {
            u->priority = priority;
            memset(&u->context, 0, sizeof(u->context));
            u->context.ra = (uint64) start_func;
            u->context.sp = (uint64) &u->ustack[STACK_SIZE];
            u->state = RUNNABLE;
            currentThread = u;
            if (priority == LOW) {
                u->priority_num = 0;
            } else if (priority == MEDIUM) {
                u->priority_num = 1;
            } else {
                u->priority_num = 2;
            }
            return 0;
        }
    }
    return -1;
}

struct uthread* pick_thread()
{
    struct uthread* t;
    struct uthread* curr = currentThread;
    int i = 0;
    int found = -1;
    int index_iter = currentThread->index;
    while (i < MAX_UTHREADS) {
        index_iter = (index_iter + 1) % MAX_UTHREADS;
        if (((t = &uthread[index_iter])->state) == RUNNABLE) {
            if (t->priority_num > curr->priority_num) {
                curr = t;
                found = 0;

            } else if (t->priority_num == curr->priority_num && found == -1) {
                curr = t;
                found = 1;
            }
        }
        i++;
    }
    if (found != -1) {
        return curr;
    }
    return 0;
}

void uthread_yield()
{
    struct uthread *u;

    u = currentThread;
    currentThread = pick_thread();
    u->state = RUNNABLE;
    currentThread->state = RUNNING;
    uswtch(&u->context, &currentThread->context);
}

void uthread_exit()
{
    currentThread->state = FREE;
    currentThread->index = 0;
    currentThread->priority_num = -1;
    struct uthread *u;
    u = currentThread;
    if ((currentThread = pick_thread()) == 0) {
        exit(0);
    }
    currentThread->state = RUNNING;
    uswtch(&u->context, &currentThread->context);
}

enum sched_priority uthread_set_priority(enum sched_priority priority)
{
    enum sched_priority old_priority = currentThread->priority;
    currentThread->priority = priority;
    return old_priority; 
}

enum sched_priority uthread_get_priority()
{
    return currentThread->priority;
}

int uthread_start_all()
{
    if (started != 0 || (currentThread = pick_thread()) == 0) {
        return -1;
    }
    uswtch(&garb.context, &currentThread->context);
    return 0;
}

struct uthread* uthread_self()
{
    return currentThread;
}