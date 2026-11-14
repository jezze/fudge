struct mailbox
{

    struct resource resource;
    struct service service;
    struct spinlock spinlock;
    unsigned int itask;
    unsigned int ichannel;
    struct message messages[MESSAGE_SLOTS];
    struct list_item items[MESSAGE_SLOTS];
    struct list free;
    struct list queued;
    struct list picked;
    unsigned long data;

};

struct mailbox *mailbox_get(unsigned int inode);
unsigned int mailbox_pick(unsigned int source, struct message *messages, unsigned int count, unsigned int slots, unsigned int flags);
unsigned int mailbox_isfull(unsigned int inode);
unsigned int mailbox_wait(unsigned int target, unsigned int itask);
void mailbox_wake(unsigned int inode);
void mailbox_reset(struct mailbox *mailbox);
void mailbox_register(struct mailbox *mailbox, unsigned int itask, unsigned int ichannel);
void mailbox_init(struct mailbox *mailbox, unsigned long data);
