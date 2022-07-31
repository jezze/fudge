#define EVENT_ERROR                     0x00
#define EVENT_MAIN                      0x01
#define EVENT_TERM                      0x02
#define EVENT_CLOSE                     0x03
#define EVENT_REDIRECT                  0x04
#define EVENT_OPTION                    0x05
#define EVENT_PATH                      0x06
#define EVENT_DATA                      0x07
#define EVENT_QUERY                     0x08
#define EVENT_KEYPRESS                  0x10
#define EVENT_KEYRELEASE                0x11
#define EVENT_MOUSEMOVE                 0x12
#define EVENT_MOUSESCROLL               0x13
#define EVENT_MOUSEPRESS                0x14
#define EVENT_MOUSERELEASE              0x15
#define EVENT_CONSOLEDATA               0x16
#define EVENT_TIMERTICK                 0x17
#define EVENT_VIDEOMODE                 0x18
#define EVENT_BLOCKREQUEST              0x19
#define EVENT_WMMAP                     0x20
#define EVENT_WMUNMAP                   0x21
#define EVENT_WMSHOW                    0x22
#define EVENT_WMHIDE                    0x23
#define EVENT_WMKEYPRESS                0x24
#define EVENT_WMKEYRELEASE              0x25
#define EVENT_WMMOUSEMOVE               0x26
#define EVENT_WMMOUSESCROLL             0x27
#define EVENT_WMMOUSEPRESS              0x28
#define EVENT_WMMOUSERELEASE            0x29
#define EVENT_WMRENDERDATA              0x2A
#define EVENT_WMINIT                    0x2B
#define EVENT_WMCLICK                   0x2C
#define EVENT_P9P                       0x30
#define EVENT_REDIRECT_TARGET           0x01
#define EVENT_REDIRECT_SOURCE           0x02

struct event_redirect
{

    unsigned int event;
    unsigned int mode;
    unsigned int id;

};

struct event_keypress
{

    unsigned char scancode;

};

struct event_keyrelease
{

    unsigned char scancode;

};

struct event_mousemove
{

    char relx;
    char rely;

};

struct event_mousescroll
{

    char relz;

};

struct event_mousepress
{

    unsigned int button;

};

struct event_mouserelease
{

    unsigned int button;

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

    void *framebuffer;
    unsigned int w;
    unsigned int h;
    unsigned int bpp;

};

struct event_blockrequest
{

    unsigned int sector;
    unsigned int count;

};

struct event_wmkeypress
{

    unsigned char scancode;
    unsigned int keymod;
    unsigned int unicode;
    unsigned int length;

};

struct event_wmkeyrelease
{

    unsigned char scancode;
    unsigned int keymod;
    unsigned int unicode;
    unsigned int length;

};

struct event_wmmousemove
{

    char relx;
    char rely;

};

struct event_wmmousescroll
{

    char relz;

};

struct event_wmmousepress
{

    unsigned int button;

};

struct event_wmmouserelease
{

    unsigned int button;

};

struct event_wmclick
{

    char clicked[16];

};

