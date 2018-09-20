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

unsigned int event_addwmkeypress(void *buffer, unsigned char scancode);
unsigned int event_addwmkeyrelease(void *buffer, unsigned char scancode);
unsigned int event_addwmmousepress(void *buffer, unsigned int button);
unsigned int event_addwmmouserelease(void *buffer, unsigned int button);
unsigned int event_addwmmousemove(void *buffer, char relx, char rely);
unsigned int event_sendwmresize(unsigned int descriptor, unsigned int source, unsigned int target, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int padding, unsigned int lineheight);
unsigned int event_sendwmshow(unsigned int descriptor, unsigned int source, unsigned int target);
unsigned int event_sendwmhide(unsigned int descriptor, unsigned int source, unsigned int target);
unsigned int event_sendwmflush(unsigned int descriptor, unsigned int source, unsigned int target, unsigned int count, void *buffer);
