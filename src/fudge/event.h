#define EVENT_KEYPRESS                  0x0001
#define EVENT_KEYRELEASE                0x0002
#define EVENT_MOUSEPRESS                0x0003
#define EVENT_MOUSERELEASE              0x0004
#define EVENT_MOUSEMOVE                 0x0005

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
