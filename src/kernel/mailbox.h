struct mailbox
{

    struct resource resource;
    struct ring ring;
    struct spinlock spinlock;
    struct list_item item;

};

unsigned int mailbox_pick(struct mailbox *mailbox, struct ipc_header *header, void *data);
unsigned int mailbox_place(struct mailbox *mailbox, struct ipc_header *header, void *data);
void mailbox_reset(struct mailbox *mailbox);
void mailbox_init(struct mailbox *mailbox, unsigned int count, char *buffer);
