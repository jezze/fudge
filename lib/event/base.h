#define EVENTS                          21
#define EVENT_ADDR_BROADCAST            0
#define EVENT_KEYPRESS                  0x01
#define EVENT_KEYRELEASE                0x02
#define EVENT_MOUSEMOVE                 0x03
#define EVENT_MOUSEPRESS                0x04
#define EVENT_MOUSERELEASE              0x05
#define EVENT_TIMERTICK                 0x06
#define EVENT_VIDEOMODE                 0x07
#define EVENT_WMMAP                     0x08
#define EVENT_WMUNMAP                   0x09
#define EVENT_WMINIT                    0x0A
#define EVENT_WMEXIT                    0x0B
#define EVENT_WMRESIZE                  0x0C
#define EVENT_WMSHOW                    0x0D
#define EVENT_WMHIDE                    0x0E
#define EVENT_WMFLUSH                   0x0F
#define EVENT_WMKEYPRESS                0x10
#define EVENT_WMKEYRELEASE              0x11
#define EVENT_WMMOUSEMOVE               0x12
#define EVENT_WMMOUSEPRESS              0x13
#define EVENT_WMMOUSERELEASE            0x14

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

unsigned int event_read(struct event *event, unsigned int descriptor);
