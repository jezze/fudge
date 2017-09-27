struct spinlock
{

    unsigned int lock;

};

void spinlock_acquire(struct spinlock *spinlock);
void spinlock_release(struct spinlock *spinlock);
