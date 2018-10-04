#define EVENT_ADDR_BROADCAST            0
#define EVENT_INIT                      0x01
#define EVENT_EXIT                      0x02
#define EVENT_KILL                      0x03
#define EVENT_DATA                      0x04
#define EVENT_FILE                      0x05
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
#define EVENT_WMRESIZE                  0x82
#define EVENT_WMSHOW                    0x83
#define EVENT_WMHIDE                    0x84
#define EVENT_WMFLUSH                   0x85
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
    unsigned int length;
    unsigned int nroutes;
    unsigned int reserved0;
    unsigned int reserved1;
    unsigned int reserved2;
    unsigned int routes[16];

};

struct event_data
{

    unsigned int session;
    unsigned int count;

};

struct event_file
{

    unsigned int session;
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

struct event_wmresize
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

void *event_getdata(struct event_header *header);
unsigned int event_avail(struct event_header *header);
struct event_header *event_addheader(void *buffer, unsigned int type, unsigned int target);
void *addpayload(struct event_header *header, unsigned int length);
unsigned int event_addroute(void *buffer, unsigned int target);
struct event_header *event_request(void *buffer, struct event_header *header, unsigned int type, unsigned int target);
struct event_header *event_forward(void *buffer, struct event_header *header, unsigned int type, unsigned int target);
struct event_header *event_reply(void *buffer, struct event_header *header, unsigned int type);
unsigned int event_adddata(struct event_header *header, unsigned int session);
unsigned int event_appenddata(struct event_header *header, unsigned int count, void *buffer);
unsigned int event_addfile(struct event_header *header, unsigned int session, unsigned int descriptor);
unsigned int event_addkeypress(struct event_header *header, unsigned char scancode);
unsigned int event_addkeyrelease(struct event_header *header, unsigned char scancode);
unsigned int event_addmousepress(struct event_header *header, unsigned int button);
unsigned int event_addmouserelease(struct event_header *header, unsigned int button);
unsigned int event_addmousemove(struct event_header *header, char relx, char rely);
unsigned int event_addconsoledata(struct event_header *header, char data);
unsigned int event_addtimertick(struct event_header *header, unsigned int counter);
unsigned int event_addvideomode(struct event_header *header, unsigned int w, unsigned int h, unsigned int bpp);
unsigned int event_addwmkeypress(struct event_header *header, unsigned char scancode);
unsigned int event_addwmkeyrelease(struct event_header *header, unsigned char scancode);
unsigned int event_addwmmousepress(struct event_header *header, unsigned int button);
unsigned int event_addwmmouserelease(struct event_header *header, unsigned int button);
unsigned int event_addwmmousemove(struct event_header *header, char relx, char rely);
unsigned int event_addwmresize(struct event_header *header, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int padding, unsigned int lineheight);
unsigned int event_addwmflush(struct event_header *header, unsigned int count, void *buffer);
