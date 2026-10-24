struct mailbox
{

    struct resource resource;
    struct spinlock spinlock;
    unsigned int itask;
    unsigned int inode;
    unsigned int ichannel;
    struct message messages[MESSAGE_SLOTS];
    struct list_item items[MESSAGE_SLOTS];
    struct list free;
    struct list queued;
    struct list picked;
    unsigned long data;
    unsigned int waiters;

};

unsigned int mailbox_wait(unsigned int target, unsigned int itask);
void mailbox_wake(unsigned int inode);
unsigned int mailbox_reset(struct mailbox *mailbox);
void mailbox_register(struct mailbox *mailbox, unsigned int itask, unsigned int ichannel);
void mailbox_unregister(struct mailbox *mailbox);
void mailbox_init(struct mailbox *mailbox, unsigned long data);
void mailbox_setup(void);
