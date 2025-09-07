struct mailbox
{

    struct resource resource;
    struct ring ring;
    struct spinlock spinlock;
    unsigned int itask;
    unsigned int inode;

};

void mailbox_reset(struct mailbox *mailbox, unsigned int itask);
void mailbox_register(struct mailbox *mailbox);
void mailbox_unregister(struct mailbox *mailbox);
void mailbox_init(struct mailbox *mailbox, void *buffer, unsigned int count);
void mailbox_setup(void);
