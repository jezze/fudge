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

void *event_getforward(void *buffer);
void *event_getdata(void *buffer);
unsigned int event_addheader(void *buffer, unsigned int type, unsigned int source, unsigned int target);
unsigned int event_addforward(void *buffer, unsigned int target);
unsigned int event_addrequest(void *buffer, struct event_header *header, unsigned int type, unsigned int id);
unsigned int event_addpipe(void *buffer, struct event_header *header, unsigned int type, unsigned int id);
unsigned int event_addresponse(void *buffer, struct event_header *header, unsigned int type);
unsigned int event_adddata(void *buffer, unsigned int count, void *data);
unsigned int event_addfile(void *buffer, unsigned int num);
struct event_header *event_read(void *data);
unsigned int event_sendbuffer(void *buffer);
unsigned int event_send(unsigned int descriptor, struct event_header *header, unsigned int type, unsigned int source, unsigned int target, unsigned int length);
