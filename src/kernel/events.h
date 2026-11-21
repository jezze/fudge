#define EVENTS_RING                     64
#define EVENTS_DATASIZE                 24
#define EVENTS_SUBSCRIBERS              8

struct events_entry
{

    unsigned int event;
    unsigned int length;
    char data[EVENTS_DATASIZE];

};

struct events_subscriber
{

    unsigned int target;
    unsigned int bit;
    unsigned int next;

};

struct events
{

    struct spinlock spinlock;
    struct events_entry ring[EVENTS_RING];
    unsigned int head;
    struct events_subscriber subscribers[EVENTS_SUBSCRIBERS];

};

unsigned int events_record(struct events *events, unsigned int event, unsigned int count, void *data);
unsigned int events_subscribe(struct events *events, unsigned int source, unsigned int bit);
unsigned int events_unsubscribe(struct events *events, unsigned int source);
unsigned int events_send(struct events *events, unsigned int inode, unsigned int source);
void events_init(struct events *events);
