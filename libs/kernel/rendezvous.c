#include <kernel.h>
#include "resource.h"
#include "vfs.h"
#include "task.h"
#include "scheduler.h"
#include "rendezvous.h"

void rendezvous_sleep(struct rendezvous *rendezvous, unsigned int condition)
{

    if (!condition)
        return;

    if (rendezvous->task)
        return;

    rendezvous->task = scheduler_find_used_task();

    scheduler_block(rendezvous->task);

}

void rendezvous_unsleep(struct rendezvous *rendezvous, unsigned int condition)
{

    if (!condition)
        return;

    if (!rendezvous->task)
        return;

    scheduler_unblock(rendezvous->task);

    /* this is an ugly fix. it rewinds the instruction
     * pointer so when a task wakes up it will run the
     * syscall again. this way i dont have to care
     * about copy to userspace. dont know if there is
     * a better or more generic way to do this. the
     * number 7 feels so arbitrary
     */
    rendezvous->task->registers.ip -= 7;

    rendezvous->task = 0;

}

void rendezvous_setup()
{

}

