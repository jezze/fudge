#define MAILBOX_SIZE                    0x00008000

struct mailbox
{

    struct resource resource;
    struct spinlock spinlock;
    unsigned int itask;
    unsigned int inode;
    unsigned int slots;
    struct message *messages;
    void *data;
    unsigned int head;
    unsigned int tail;

};

void mailbox_reset(struct mailbox *mailbox, unsigned int itask);
void mailbox_register(struct mailbox *mailbox);
void mailbox_unregister(struct mailbox *mailbox);
void mailbox_init(struct mailbox *mailbox, void *messages, void *data, unsigned int count);
void mailbox_setup(void);
