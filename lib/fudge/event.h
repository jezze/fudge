#define EVENTS                          64
#define EVENT_ANY                       0x00
#define EVENT_KILL                      0x01
#define EVENT_EMPTY                     0x02
#define EVENT_DATA                      0x03
#define EVENT_FILE                      0x04
#define EVENT_OPEN                      0x05
#define EVENT_CLOSE                     0x06
#define EVENT_KEYPRESS                  0x10
#define EVENT_KEYRELEASE                0x11
#define EVENT_MOUSEMOVE                 0x12
#define EVENT_MOUSEPRESS                0x13
#define EVENT_MOUSERELEASE              0x14
#define EVENT_CONSOLEDATA               0x15
#define EVENT_TIMERTICK                 0x16
#define EVENT_VIDEOMODE                 0x17
#define EVENT_BLOCKREQUEST              0x18
#define EVENT_WMCLOSE                   0x20
#define EVENT_WMMAP                     0x21
#define EVENT_WMUNMAP                   0x22
#define EVENT_WMCONFIGURE               0x23
#define EVENT_WMSHOW                    0x24
#define EVENT_WMHIDE                    0x25
#define EVENT_WMKEYPRESS                0x26
#define EVENT_WMKEYRELEASE              0x27
#define EVENT_WMMOUSEMOVE               0x28
#define EVENT_WMMOUSEPRESS              0x29
#define EVENT_WMMOUSERELEASE            0x2A

struct event_file
{

    unsigned char descriptor;

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

struct event_blockrequest
{

    unsigned int sector;
    unsigned int count;

};

struct event_wmconfigure
{

    unsigned int x;
    unsigned int y;
    unsigned int w;
    unsigned int h;
    unsigned int padding;
    unsigned int lineheight;

};

struct event_wmkeypress
{

    unsigned char scancode;

};

struct event_wmkeyrelease
{

    unsigned char scancode;

};

struct event_wmmousepress
{

    unsigned int button;

};

struct event_wmmouserelease
{

    unsigned int button;

};

struct event_wmmousemove
{

    char relx;
    char rely;

};
