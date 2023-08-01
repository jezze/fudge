#define EVENT_MAIN                      0x01
#define EVENT_TERM                      0x02
#define EVENT_CLOSE                     0x03
#define EVENT_REDIRECT                  0x04
#define EVENT_OPTION                    0x05
#define EVENT_PATH                      0x06
#define EVENT_DATA                      0x07
#define EVENT_ERROR                     0x08
#define EVENT_CONFIG                    0x09
#define EVENT_QUERY                     0x0A
#define EVENT_STATUS                    0x0B
#define EVENT_LOGINFO                   0x0C
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
#define EVENT_WMGRAB                    0x22
#define EVENT_WMUNGRAB                  0x23
#define EVENT_WMSHOW                    0x24
#define EVENT_WMHIDE                    0x25
#define EVENT_WMKEYPRESS                0x26
#define EVENT_WMKEYRELEASE              0x27
#define EVENT_WMMOUSEMOVE               0x28
#define EVENT_WMMOUSESCROLL             0x29
#define EVENT_WMMOUSEPRESS              0x2A
#define EVENT_WMMOUSERELEASE            0x2B
#define EVENT_WMRENDERDATA              0x2C
#define EVENT_WMINIT                    0x2D
#define EVENT_WMEVENT                   0x2E
#define EVENT_WMACK                     0x2F
#define EVENT_P9P                       0x30
#define EVENT_WALKREQUEST               0x41
#define EVENT_WALKRESPONSE              0x42
#define EVENT_READREQUEST               0x43
#define EVENT_READRESPONSE              0x44
#define EVENT_LISTREQUEST               0x45
#define EVENT_LISTRESPONSE              0x46
#define EVENT_WRITEREQUEST              0x47
#define EVENT_WRITERESPONSE             0x48
#define EVENT_REDIRECT_TARGET           0x01
#define EVENT_REDIRECT_SOURCE           0x02

struct event_redirect
{

    unsigned int event;
    unsigned int mode;
    unsigned int target;

};

struct event_loginfo
{

    unsigned int level;
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

    unsigned int id;
    unsigned char scancode;
    unsigned int keymod;
    unsigned int unicode;
    unsigned int length;

};

struct event_wmkeyrelease
{

    unsigned int id;
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

struct event_wmevent
{

    unsigned int type;
    unsigned int length;

};

struct event_listrequest
{

    unsigned int session;
    unsigned int id;

};

struct event_listresponse
{

    unsigned int session;
    unsigned int nrecords;

};

struct event_readrequest
{

    unsigned int session;
    unsigned int id;
    unsigned int offset;
    unsigned int count;

};

struct event_readresponse
{

    unsigned int session;
    unsigned int count;

};

struct event_walkrequest
{

    unsigned int session;
    unsigned int parent;
    unsigned int length;

};

struct event_walkresponse
{

    unsigned int session;
    unsigned int id;

};

struct event_writerequest
{

    unsigned int session;
    unsigned int id;
    unsigned int offset;
    unsigned int count;

};

struct event_writeresponse
{

    unsigned int session;
    unsigned int count;

};
