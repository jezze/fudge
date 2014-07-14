#include <kernel.h>
#include "resource.h"
#include "vfs.h"
#include "task.h"
#include "scheduler.h"
#include "rendezvous.h"

unsigned int rendezvous_asleep(struct rendezvous *rendezvous)
{

    return rendezvous->task != 0;

}

void rendezvous_sleep(struct rendezvous *rendezvous, unsigned int condition)
{

    if (rendezvous->task)
        return;

    if (!condition)
        return;

    rendezvous->task = scheduler_find_used_task();

    scheduler_block(rendezvous->task);

}

void rendezvous_unsleep(struct rendezvous *rendezvous)
{

    if (!rendezvous->task)
        return;

    scheduler_unblock(rendezvous->task);

    rendezvous->task->registers.ip -= 7;
    rendezvous->task = 0;

}

void rendezvous_setup()
{

}

