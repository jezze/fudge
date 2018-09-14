#define EVENTS                          21
#define EVENT_ADDR_SELF                 0
#define EVENT_ADDR_BROADCAST            0
#define EVENT_INIT                      0x01
#define EVENT_EXIT                      0x02
#define EVENT_KILL                      0x03
#define EVENT_DATA                      0x04
#define EVENT_FILE                      0x05

struct event_header
{

    unsigned int type;
    unsigned int source;
    unsigned int destination;
    unsigned int length;

};

struct event
{

    struct event_header header;
    char data[FUDGE_BSIZE];

};

struct event_file
{

    unsigned char num;

};

void event_read(unsigned int descriptor, struct event *event);
void event_send(unsigned int descriptor, struct event *event, unsigned int type, unsigned int source, unsigned int destination, unsigned int length);
void event_sendinit(unsigned int descriptor, unsigned int source, unsigned int destination);
void event_sendexit(unsigned int descriptor, unsigned int source, unsigned int destination);
void event_sendkill(unsigned int descriptor, unsigned int source, unsigned int destination);
void event_senddata(unsigned int descriptor, unsigned int source, unsigned int destination, unsigned int count, void *buffer);
void event_sendfile(unsigned int descriptor, unsigned int source, unsigned int destination, unsigned char num);
void event_initheader(struct event_header *header, unsigned int type, unsigned int source, unsigned int destination, unsigned int length);
