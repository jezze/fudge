#include "atomic.h"
#include "spinlock.h"

void spinlock_acquire(struct spinlock *spinlock)
{

    while (atomic_testandset(1, &spinlock->lock));

}

void spinlock_release(struct spinlock *spinlock)
{

    atomic_testandset(0, &spinlock->lock);

}

void spinlock_init(struct spinlock *spinlock)
{

    spinlock->lock = 0;

}
