struct mailbox
{

    struct resource resource;
    struct spinlock spinlock;
    unsigned int itask;
    unsigned int inode;
    struct message messages[MESSAGE_SLOTS];
    unsigned long data;
    unsigned int head;
    unsigned int tail;
    unsigned int steps;

};

void mailbox_reset(struct mailbox *mailbox, unsigned int itask);
void mailbox_register(struct mailbox *mailbox);
void mailbox_unregister(struct mailbox *mailbox);
void mailbox_init(struct mailbox *mailbox, unsigned long data);
void mailbox_setup(void);
