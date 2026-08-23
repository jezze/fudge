#define MAILBOX_SIZE                    0x1000
#define MAILBOX_SLOTS                   8

struct mailbox
{

    struct resource resource;
    struct spinlock spinlock;
    unsigned int itask;
    unsigned int inode;
    struct message messages[8];
    void *data;
    unsigned int head;
    unsigned int tail;

};

void mailbox_reset(struct mailbox *mailbox, unsigned int itask);
void mailbox_register(struct mailbox *mailbox);
void mailbox_unregister(struct mailbox *mailbox);
void mailbox_init(struct mailbox *mailbox, void *data);
void mailbox_setup(void);
