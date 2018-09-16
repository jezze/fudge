#define EVENT_ADDR_SELF                 0
#define EVENT_ADDR_BROADCAST            0

struct event_header
{

    unsigned int type;
    unsigned int source;
    unsigned int destination;
    unsigned int length;

};

void event_initheader(struct event_header *header, unsigned int type, unsigned int source, unsigned int destination, unsigned int length);
