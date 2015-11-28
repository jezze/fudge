#define EVENTS                          17
#define EVENT_KEYPRESS                  0x01
#define EVENT_KEYRELEASE                0x02
#define EVENT_MOUSEPRESS                0x03
#define EVENT_MOUSERELEASE              0x04
#define EVENT_MOUSEMOVE                 0x05
#define EVENT_WMMAP                     0x06
#define EVENT_WMUNMAP                   0x07
#define EVENT_WMRESIZE                  0x08
#define EVENT_WMSHOW                    0x09
#define EVENT_WMHIDE                    0x0A
#define EVENT_TICK                      0x10

struct event_header
{

    unsigned int destination;
    unsigned int source;
    unsigned int type;
    unsigned int count;

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

struct event_wmresize
{

    unsigned int x;
    unsigned int y;
    unsigned int w;
    unsigned int h;

};

struct event_tick
{

    unsigned int counter;

};
