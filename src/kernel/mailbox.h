#define MAILBOX_REPLYSLOTS              1

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
    struct list reply;
    struct list reserved;
    unsigned int held;
    volatile unsigned char *done;
    unsigned long pages;

};

void mailbox_reclaimtask(unsigned int itask);
void mailbox_sleep(struct mailbox *mailbox);
unsigned int mailbox_pick(struct mailbox *mailbox, struct message *messages, unsigned int count, unsigned int flags);
unsigned int mailbox_reservereply(struct mailbox *mailbox);
void mailbox_unreservereply(struct mailbox *mailbox);
unsigned int mailbox_keepreply(struct mailbox *mailbox);
void mailbox_unreserve(struct mailbox *mailbox);
unsigned int mailbox_placereply(struct mailbox *mailbox, unsigned int source, unsigned int event, unsigned int count, void *data);
unsigned int mailbox_placereserved(struct mailbox *mailbox, unsigned int source, unsigned int event, unsigned int count, void *data);
unsigned int mailbox_wait(struct mailbox *mailbox, unsigned int itask);
void mailbox_reset(struct mailbox *mailbox);
void mailbox_register(struct mailbox *mailbox, unsigned int itask, unsigned int ichannel, unsigned long pages);
void mailbox_init(struct mailbox *mailbox);
