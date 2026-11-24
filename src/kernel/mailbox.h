#define MAILBOX_REPLYSLOTS              1
#define MAILBOX_MAILSLOTS               1

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
    unsigned int reply;
    unsigned int reserved;
    unsigned long pages;

};

void mailbox_sleep(struct mailbox *mailbox);
unsigned int mailbox_pick(struct mailbox *mailbox, struct message_box *boxes, unsigned int count, unsigned int flags);
unsigned int mailbox_reservereply(struct mailbox *mailbox);
void mailbox_unreservereply(struct mailbox *mailbox);
unsigned int mailbox_keepreply(struct mailbox *mailbox);
void mailbox_unreserve(struct mailbox *mailbox);
unsigned int mailbox_placenotify(struct mailbox *mailbox, unsigned int source, unsigned int event, unsigned int count, void *data);
unsigned int mailbox_placereply(struct mailbox *mailbox, unsigned int source, unsigned int event, unsigned int count, void *data);
unsigned int mailbox_placereserved(struct mailbox *mailbox, unsigned int source, unsigned int event, unsigned int count, void *data);
void mailbox_reset(struct mailbox *mailbox);
void mailbox_register(struct mailbox *mailbox, unsigned int itask, unsigned int ichannel, unsigned long pages);
void mailbox_init(struct mailbox *mailbox);
