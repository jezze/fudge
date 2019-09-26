struct mailbox
{

    struct ring ring;
    struct spinlock spinlock;

};

unsigned int mailbox_pick(struct mailbox *mailbox, struct event_header *header, void *data);
unsigned int mailbox_place(struct mailbox *mailbox, struct event_header *header, void *data);
void mailbox_reset(struct mailbox *mailbox);
void mailbox_init(struct mailbox *mailbox, unsigned int count, char *buffer);
