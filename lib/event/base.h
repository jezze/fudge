#define EVENTS                          21
#define EVENT_ADDR_BROADCAST            0

struct event_header
{

    unsigned int destination;
    unsigned int source;
    unsigned int type;
    unsigned int length;

};

struct event
{

    struct event_header header;
    char data[FUDGE_BSIZE];

};

void event_read(unsigned int descriptor, struct event *event);
void event_send(unsigned int descriptor, struct event *event, unsigned int destination, unsigned int type, unsigned int length);
