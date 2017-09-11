struct spinlock
{

    unsigned int lock;

};

void spinlock_hold(struct spinlock *spinlock);
void spinlock_release(struct spinlock *spinlock);
