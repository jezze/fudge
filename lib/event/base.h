#define EVENT_INIT                      0x01
#define EVENT_EXIT                      0x02
#define EVENT_KILL                      0x03
#define EVENT_DATA                      0x04
#define EVENT_FILE                      0x05
#define EVENT_CHILD                     0x06

struct event_data
{

    unsigned int count;

};

struct event_file
{

    unsigned char num;

};

void *event_getheader(void *buffer);
void *event_getforward(struct event_header *header);
void *event_getdata(struct event_header *header);
void *event_addheader(void *buffer, unsigned int type, unsigned int source, unsigned int target);
void *event_addforward(void *buffer, unsigned int target);
struct event_header *event_read(unsigned int descriptor, void *data);
unsigned int event_send(unsigned int descriptor, struct event_header *header, unsigned int type, unsigned int source, unsigned int target, unsigned int length);
unsigned int event_sendinit(unsigned int descriptor, unsigned int source, unsigned int target);
unsigned int event_sendexit(unsigned int descriptor, unsigned int source, unsigned int target);
unsigned int event_sendkill(unsigned int descriptor, unsigned int source, unsigned int target);
unsigned int event_senddata(unsigned int descriptor, unsigned int source, unsigned int target, unsigned int count, void *buffer);
unsigned int event_sendfile(unsigned int descriptor, unsigned int source, unsigned int target, unsigned char num);
unsigned int event_sendchild(unsigned int descriptor, unsigned int source, unsigned int target);
