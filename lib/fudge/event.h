#define EVENT_BROADCAST                 0
#define EVENT_INIT                      0x01
#define EVENT_KILL                      0x02
#define EVENT_DATA                      0x03
#define EVENT_FILE                      0x04
#define EVENT_STOP                      0x05
#define EVENT_KEYPRESS                  0x10
#define EVENT_KEYRELEASE                0x11
#define EVENT_MOUSEMOVE                 0x12
#define EVENT_MOUSEPRESS                0x13
#define EVENT_MOUSERELEASE              0x14
#define EVENT_CONSOLEDATA               0x15
#define EVENT_TIMERTICK                 0x16
#define EVENT_VIDEOMODE                 0x17
#define EVENT_WMMAP                     0x80
#define EVENT_WMUNMAP                   0x81
#define EVENT_WMCONFIGURE               0x82
#define EVENT_WMSHOW                    0x83
#define EVENT_WMHIDE                    0x84
#define EVENT_WMKEYPRESS                0x90
#define EVENT_WMKEYRELEASE              0x91
#define EVENT_WMMOUSEMOVE               0x92
#define EVENT_WMMOUSEPRESS              0x93
#define EVENT_WMMOUSERELEASE            0x94

struct event_header
{

    unsigned int type;
    unsigned int source;
    unsigned int target;
    unsigned int session;
    unsigned int length;
    unsigned int nroutes;
    unsigned int routes[16];

};

union event_message
{

    struct event_header header;
    char buffer[FUDGE_BSIZE];

};

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

struct event_wmconfigure
{

    unsigned int rendertarget;
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

void *event_getdata(union event_message *message);
unsigned int event_getdatasize(union event_message *message);
unsigned int event_avail(union event_message *message);
unsigned int event_route(union event_message *message, unsigned int target);
unsigned int event_addfile(union event_message *message, unsigned int descriptor);
unsigned int event_addconsoledata(union event_message *message, char data);
unsigned int event_addkeypress(union event_message *message, unsigned char scancode);
unsigned int event_addkeyrelease(union event_message *message, unsigned char scancode);
unsigned int event_addmousepress(union event_message *message, unsigned int button);
unsigned int event_addmouserelease(union event_message *message, unsigned int button);
unsigned int event_addmousemove(union event_message *message, char relx, char rely);
unsigned int event_addtimertick(union event_message *message, unsigned int counter);
unsigned int event_addvideomode(union event_message *message, unsigned int w, unsigned int h, unsigned int bpp);
unsigned int event_addwmconfigure(union event_message *message, unsigned int rendertarget, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int padding, unsigned int lineheight);
unsigned int event_addwmkeypress(union event_message *message, unsigned char scancode);
unsigned int event_addwmkeyrelease(union event_message *message, unsigned char scancode);
unsigned int event_addwmmousepress(union event_message *message, unsigned int button);
unsigned int event_addwmmouserelease(union event_message *message, unsigned int button);
unsigned int event_addwmmousemove(union event_message *message, char relx, char rely);
unsigned int event_append(union event_message *message, unsigned int count, void *buffer);
void event_create(union event_message *message, unsigned int type, unsigned int target, unsigned int session);
void event_forward(union event_message *omessage, union event_message *imessage, unsigned int type, unsigned int target);
void event_request(union event_message *omessage, union event_message *imessage, unsigned int type, unsigned int target, unsigned int session);
void event_reply(union event_message *omessage, union event_message *imessage, unsigned int type);
