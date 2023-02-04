struct mailbox
{

    struct resource resource;
    struct ring ring;
    struct spinlock spinlock;

};

unsigned int mailbox_pick(struct mailbox *mailbox, struct message *message, void *data);
unsigned int mailbox_place(struct mailbox *mailbox, struct message *message, void *data);
void mailbox_reset(struct mailbox *mailbox);
void mailbox_register(struct mailbox *mailbox);
void mailbox_unregister(struct mailbox *mailbox);
void mailbox_init(struct mailbox *mailbox, void *buffer, unsigned int count);
