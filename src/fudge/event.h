#define EVENTS                          16
#define EVENT_KEYPRESS                  0x01
#define EVENT_KEYRELEASE                0x02
#define EVENT_MOUSEPRESS                0x03
#define EVENT_MOUSERELEASE              0x04
#define EVENT_MOUSEMOVE                 0x05
#define EVENT_WMMAP                     0x06
#define EVENT_WMUNMAP                   0x07
#define EVENT_WMMAPNOTIFY               0x08
#define EVENT_WMEXPOSE                  0x09

struct event_header
{

    unsigned int destination;
    unsigned int source;
    unsigned int type;
    unsigned int count;

};

struct event_keypress
{

    struct event_header header;
    unsigned char scancode;

};

struct event_keyrelease
{

    struct event_header header;
    unsigned char scancode;

};

struct event_mousepress
{

    struct event_header header;
    unsigned int button;

};

struct event_mouserelease
{

    struct event_header header;
    unsigned int button;

};

struct event_mousemove
{

    struct event_header header;
    char relx;
    char rely;

};

struct event_wmmap
{

    struct event_header header;

};

struct event_wmunmap
{

    struct event_header header;

};

struct event_wmmapnotify
{

    struct event_header header;
    unsigned int x;
    unsigned int y;
    unsigned int w;
    unsigned int h;

};

struct event_wmexpose
{

    struct event_header header;
    unsigned int x;
    unsigned int y;
    unsigned int w;
    unsigned int h;

};

union event
{

    unsigned char data[512];
    struct event_header header;
    struct event_keypress keypress;
    struct event_keyrelease keyrelease;
    struct event_mousepress mousepress;
    struct event_mouserelease mouserelease;
    struct event_mousemove mousemove;
    struct event_wmmap wmmap;
    struct event_wmunmap wmunmap;
    struct event_wmmapnotify wmmapnotify;
    struct event_wmexpose wmexpose;

};
