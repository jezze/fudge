#include "atomic.h"
#include "spinlock.h"

void spinlock_hold(struct spinlock *spinlock)
{

    while (atomic_testandset(1, &spinlock->lock));

}

void spinlock_release(struct spinlock *spinlock)
{

    atomic_testandset(0, &spinlock->lock);

}

