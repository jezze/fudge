struct mailbox
{

    struct resource resource;
    struct ring ring;
    struct spinlock spinlock;
    unsigned int itask;
    unsigned int inode;

};

unsigned int mailbox_pick(struct mailbox *mailbox, struct message *message, unsigned int count, void *data);
unsigned int mailbox_place(struct mailbox *mailbox, unsigned int event, unsigned int source, unsigned int count, void *data);
void mailbox_reset(struct mailbox *mailbox, unsigned int itask, unsigned int inode);
void mailbox_register(struct mailbox *mailbox);
void mailbox_unregister(struct mailbox *mailbox);
void mailbox_init(struct mailbox *mailbox, void *buffer, unsigned int count);
