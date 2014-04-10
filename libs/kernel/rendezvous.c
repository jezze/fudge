#include <kernel.h>
#include "resource.h"
#include "vfs.h"
#include "task.h"
#include "scheduler.h"
#include "rendezvous.h"

unsigned int rendezvous_islocked(struct rendezvous *rendezvous)
{

    return rendezvous->task != 0;

}

void rendezvous_lock(struct rendezvous *rendezvous)
{

    if (rendezvous_islocked(rendezvous))
        return;

    rendezvous->task = scheduler_find_used_task();

}

void rendezvous_unlock(struct rendezvous *rendezvous)
{

    if (!rendezvous_islocked(rendezvous))
        return;

    rendezvous->task = 0;

}

void rendezvous_sleep(struct rendezvous *rendezvous, unsigned int condition)
{

    if (!condition)
        return;

    if (!rendezvous_islocked(rendezvous))
        return;

    scheduler_block(rendezvous->task);

}

void rendezvous_unsleep(struct rendezvous *rendezvous, unsigned int condition)
{

    if (!condition)
        return;

    if (!rendezvous_islocked(rendezvous))
        return;

    scheduler_unblock(rendezvous->task);

    rendezvous->task->registers.ip -= 7;

}

void rendezvous_setup()
{

}

