#define EVENTS                          21
#define EVENT_ADDR_BROADCAST            0
#define EVENT_KEYPRESS                  0x01
#define EVENT_KEYRELEASE                0x02
#define EVENT_MOUSEMOVE                 0x03
#define EVENT_MOUSEPRESS                0x04
#define EVENT_MOUSERELEASE              0x05
#define EVENT_TIMERTICK                 0x06
#define EVENT_VIDEOMODE                 0x07
#define EVENT_WMMAP                     0x08
#define EVENT_WMUNMAP                   0x09
#define EVENT_WMINIT                    0x0A
#define EVENT_WMEXIT                    0x0B
#define EVENT_WMRESIZE                  0x0C
#define EVENT_WMSHOW                    0x0D
#define EVENT_WMHIDE                    0x0E
#define EVENT_WMFLUSH                   0x0F
#define EVENT_WMKEYPRESS                0x10
#define EVENT_WMKEYRELEASE              0x11
#define EVENT_WMMOUSEMOVE               0x12
#define EVENT_WMMOUSEPRESS              0x13
#define EVENT_WMMOUSERELEASE            0x14

struct event_header
{

    unsigned int destination;
    unsigned int source;
    unsigned int type;

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

struct event_handlers
{

    void (*keypress)(struct event_header *header, struct event_keypress *keypress);
    void (*keyrelease)(struct event_header *header, struct event_keyrelease *keyrelease);
    void (*mousemove)(struct event_header *header, struct event_mousemove *mousemove);
    void (*mousepress)(struct event_header *header, struct event_mousepress *mousepress);
    void (*mouserelease)(struct event_header *header, struct event_mouserelease *mouserelease);
    void (*timertick)(struct event_header *header, struct event_timertick *timertick);
    void (*videomode)(struct event_header *header, struct event_videomode *videomode);
    void (*wmmap)(struct event_header *header);
    void (*wmunmap)(struct event_header *header);
    void (*wminit)(struct event_header *header);
    void (*wmexit)(struct event_header *header);
    void (*wmresize)(struct event_header *header, struct event_wmresize *wmresize);
    void (*wmshow)(struct event_header *header);
    void (*wmhide)(struct event_header *header);
    void (*wmflush)(struct event_header *header);
    void (*wmkeypress)(struct event_header *header, struct event_wmkeypress *wmkeypress);
    void (*wmkeyrelease)(struct event_header *header, struct event_wmkeyrelease *wmkeyrelease);
    void (*wmmousemove)(struct event_header *header, struct event_wmmousemove *wmmousemove);
    void (*wmmousepress)(struct event_header *header, struct event_wmmousepress *wmmousepress);
    void (*wmmouserelease)(struct event_header *header, struct event_wmmouserelease *wmmouserelease);

};

unsigned int event_read(struct event_handlers *handlers, unsigned int descriptor);
void event_sendwmmap(unsigned int descriptor, unsigned int destination);
void event_sendwmunmap(unsigned int descriptor, unsigned int destination);
void event_sendwminit(unsigned int descriptor, unsigned int destination);
void event_sendwmexit(unsigned int descriptor, unsigned int destination);
void event_sendwmresize(unsigned int descriptor, unsigned int destination, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int padding, unsigned int lineheight);
void event_sendwmshow(unsigned int descriptor, unsigned int destination);
void event_sendwmhide(unsigned int descriptor, unsigned int destination);
void event_sendwmflush(unsigned int descriptor, unsigned int destination);
void event_sendwmkeypress(unsigned int descriptor, unsigned int destination, unsigned char scancode);
void event_sendwmkeyrelease(unsigned int descriptor, unsigned int destination, unsigned char scancode);
void event_sendwmmousemove(unsigned int descriptor, unsigned int destination, char relx, char rely);
void event_sendwmmousepress(unsigned int descriptor, unsigned int destination, unsigned int button);
void event_sendwmmouserelease(unsigned int descriptor, unsigned int destination, unsigned int button);
