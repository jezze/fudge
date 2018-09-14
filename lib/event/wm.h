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

void event_sendwmmap(unsigned int descriptor, unsigned int source, unsigned int destination);
void event_sendwmunmap(unsigned int descriptor, unsigned int source, unsigned int destination);
void event_sendwmresize(unsigned int descriptor, unsigned int source, unsigned int destination, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int padding, unsigned int lineheight);
void event_sendwmshow(unsigned int descriptor, unsigned int source, unsigned int destination);
void event_sendwmhide(unsigned int descriptor, unsigned int source, unsigned int destination);
void event_sendwmflush(unsigned int descriptor, unsigned int source, unsigned int destination, unsigned int count, void *buffer);
void event_sendwmkeypress(unsigned int descriptor, unsigned int source, unsigned int destination, unsigned char scancode);
void event_sendwmkeyrelease(unsigned int descriptor, unsigned int source, unsigned int destination, unsigned char scancode);
void event_sendwmmousemove(unsigned int descriptor, unsigned int source, unsigned int destination, char relx, char rely);
void event_sendwmmousepress(unsigned int descriptor, unsigned int source, unsigned int destination, unsigned int button);
void event_sendwmmouserelease(unsigned int descriptor, unsigned int source, unsigned int destination, unsigned int button);
