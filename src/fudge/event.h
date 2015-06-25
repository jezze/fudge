#define EVENT_KEYPRESS                  0x0101
#define EVENT_KEYRELEASE                0x0102
#define EVENT_MOUSEPRESS                0x0201
#define EVENT_MOUSERELEASE              0x0202
#define EVENT_MOUSEMOVE                 0x0203
#define EVENT_WMMAP                     0x0301
#define EVENT_WMRESIZE                  0x0302
#define EVENT_WMDRAW                    0x0303
#define EVENT_WMQUIT                    0x0304

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

struct event_wmresize
{

    struct event_header header;
    unsigned int x;
    unsigned int y;
    unsigned int w;
    unsigned int h;

};

struct event_wmdraw
{

    struct event_header header;
    unsigned int x;
    unsigned int y;
    unsigned int w;
    unsigned int h;

};

struct event_wmquit
{

    struct event_header header;

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
    struct event_wmresize wmresize;
    struct event_wmdraw wmdraw;
    struct event_wmquit wmquit;

};
