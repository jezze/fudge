#define EVENT_KEYBOARD                  1
#define EVENT_MOUSE                     2
#define EVENT_CONSOLE                   3
#define EVENT_TIMER                     4
#define EVENT_NETWORK                   5

#define EVENT_KEYBOARD_PRESS            0x0001
#define EVENT_KEYBOARD_RELEASE          0x0002

#define EVENT_MOUSE_MOVE                0x0001
#define EVENT_MOUSE_PRESS               0x0002
#define EVENT_MOUSE_RELEASE             0x0004

struct event_header
{

    unsigned int destination;
    unsigned int source;
    unsigned int type;
    unsigned int count;

};

struct event_keyboard
{

    struct event_header header;
    unsigned int mask;
    unsigned int scancode;

};

struct event_mouse
{

    struct event_header header;
    unsigned int mask;
    unsigned int button;
    unsigned int x;
    unsigned int y;

};
