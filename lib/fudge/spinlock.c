#include "spinlock.h"

void spinlock_hold(struct spinlock *spinlock)
{

    while (spinlock->lock);

    spinlock->lock = 1;

}

void spinlock_release(struct spinlock *spinlock)
{

    spinlock->lock = 0;

}

