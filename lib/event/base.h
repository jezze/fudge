#define EVENTS                          21
#define EVENT_ADDR_BROADCAST            0
#define EVENT_INIT                      0x01
#define EVENT_EXIT                      0x02
#define EVENT_KILL                      0x03
#define EVENT_REIN                      0x04
#define EVENT_REOUT                     0x05
#define EVENT_DATA                      0x06

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
void event_sendinit(unsigned int descriptor, unsigned int destination);
void event_sendexit(unsigned int descriptor, unsigned int destination);
void event_sendkill(unsigned int descriptor, unsigned int destination);
