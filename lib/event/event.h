#define EVENTS                          16
#define EVENT_ADDR_BROADCAST            0
#define EVENT_KEYPRESS                  0x01
#define EVENT_KEYRELEASE                0x02
#define EVENT_MOUSEMOVE                 0x03
#define EVENT_MOUSEPRESS                0x04
#define EVENT_MOUSERELEASE              0x05
#define EVENT_TIMERTICK                 0x06
#define EVENT_VIDEOMODE                 0x07
#define EVENT_WMMAP                     0x08
#define EVENT_WMINIT                    0x09
#define EVENT_WMEXIT                    0x0A
#define EVENT_WMRESIZE                  0x0B
#define EVENT_WMSHOW                    0x0C
#define EVENT_WMHIDE                    0x0D
#define EVENT_WMFLUSH                   0x0E

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
    void (*wminit)(struct event_header *header);
    void (*wmexit)(struct event_header *header);
    void (*wmresize)(struct event_header *header, struct event_wmresize *wmresize);
    void (*wmshow)(struct event_header *header);
    void (*wmhide)(struct event_header *header);
    void (*wmflush)(struct event_header *header);

};

unsigned int event_read(struct event_handlers *handlers, unsigned int descriptor);
void event_sendkeypress(unsigned int descriptor, unsigned int destination, unsigned char scancode);
void event_sendkeyrelease(unsigned int descriptor, unsigned int destination, unsigned char scancode);
void event_sendmousemove(unsigned int descriptor, unsigned int destination, char relx, char rely);
void event_sendmousepress(unsigned int descriptor, unsigned int destination, unsigned int button);
void event_sendmouserelease(unsigned int descriptor, unsigned int destination, unsigned int button);
void event_sendwmmap(unsigned int descriptor, unsigned int destination);
void event_sendwminit(unsigned int descriptor, unsigned int destination);
void event_sendwmexit(unsigned int descriptor, unsigned int destination);
void event_sendwmresize(unsigned int descriptor, unsigned int destination, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int padding, unsigned int lineheight);
void event_sendwmshow(unsigned int descriptor, unsigned int destination);
void event_sendwmhide(unsigned int descriptor, unsigned int destination);
void event_sendwmflush(unsigned int descriptor, unsigned int destination);
