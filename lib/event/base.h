#define EVENTS                          21
#define EVENT_ADDR_BROADCAST            0
#define EVENT_KEYPRESS                  0x10
#define EVENT_KEYRELEASE                0x11
#define EVENT_MOUSEMOVE                 0x12
#define EVENT_MOUSEPRESS                0x13
#define EVENT_MOUSERELEASE              0x14
#define EVENT_CONSOLEDATA               0x15
#define EVENT_TIMERTICK                 0x16
#define EVENT_VIDEOMODE                 0x17

struct event_header
{

    unsigned int destination;
    unsigned int source;
    unsigned int type;
    unsigned int length;

};

struct event_keypress
{

    unsigned char scancode;

};

struct event_keyrelease
{

    unsigned char scancode;

};

struct event_mousepress
{

    unsigned int button;

};

struct event_mouserelease
{

    unsigned int button;

};

struct event_mousemove
{

    char relx;
    char rely;

};

struct event_consoledata
{

    unsigned char data;

};

struct event_timertick
{

    unsigned int counter;

};

struct event_videomode
{

    unsigned int w;
    unsigned int h;
    unsigned int bpp;

};

struct event
{

    struct event_header header;
    char data[FUDGE_BSIZE];

};

void event_read(unsigned int descriptor, struct event *event);
void event_send(unsigned int descriptor, struct event *event, unsigned int destination, unsigned int type, unsigned int length);
