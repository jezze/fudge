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
    unsigned int reserved0;
    unsigned int reserved1;
    unsigned int routes[16];

};

union event_message
{

    struct event_header header;
    char message[FUDGE_BSIZE];

};

struct event_data
{

    unsigned int count;

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

void *event_getdata(struct event_header *header);
unsigned int event_avail(struct event_header *header);
unsigned int event_route(struct event_header *header, unsigned int target);
unsigned int event_appenddata(struct event_header *header, unsigned int count, void *buffer);
struct event_header *event_create(struct event_header *oheader, unsigned int type, unsigned int target, unsigned int session);
struct event_header *event_createfile(struct event_header *oheader, unsigned int target, unsigned int session, unsigned int descriptor);
struct event_header *event_createdata(struct event_header *oheader, unsigned int target, unsigned int session);
struct event_header *event_createstop(struct event_header *oheader, unsigned int target, unsigned int session);
struct event_header *event_createinit(struct event_header *oheader, unsigned int target, unsigned int session);
struct event_header *event_createconsoledata(struct event_header *oheader, char data);
struct event_header *event_createkeypress(struct event_header *oheader, unsigned char scancode);
struct event_header *event_createkeyrelease(struct event_header *oheader, unsigned char scancode);
struct event_header *event_createmousepress(struct event_header *oheader, unsigned int button);
struct event_header *event_createmouserelease(struct event_header *oheader, unsigned int button);
struct event_header *event_createmousemove(struct event_header *oheader, char relx, char rely);
struct event_header *event_createtimertick(struct event_header *oheader, unsigned int counter);
struct event_header *event_createvideomode(struct event_header *oheader, unsigned int w, unsigned int h, unsigned int bpp);
struct event_header *event_forward(struct event_header *oheader, struct event_header *iheader, unsigned int type, unsigned int target);
struct event_header *event_forwardfile(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int descriptor);
struct event_header *event_forwardstop(struct event_header *oheader, struct event_header *iheader, unsigned int target);
struct event_header *event_forwardinit(struct event_header *oheader, struct event_header *iheader, unsigned int target);
struct event_header *event_request(struct event_header *oheader, struct event_header *iheader, unsigned int type, unsigned int target, unsigned int session);
struct event_header *event_requestfile(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int session, unsigned int descriptor);
struct event_header *event_requestdata(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int session);
struct event_header *event_requeststop(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int session);
struct event_header *event_requestinit(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int session);
struct event_header *event_requestkill(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int session);
struct event_header *event_requestwmshow(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int session);
struct event_header *event_requestwmhide(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int session);
struct event_header *event_requestwmmap(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int session);
struct event_header *event_requestwmunmap(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int session);
struct event_header *event_requestwmkeypress(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int session, unsigned char scancode);
struct event_header *event_requestwmkeyrelease(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int session, unsigned char scancode);
struct event_header *event_requestwmmousepress(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int session, unsigned int button);
struct event_header *event_requestwmmouserelease(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int session, unsigned int button);
struct event_header *event_requestwmmousemove(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int session, char relx, char rely);
struct event_header *event_requestwmconfigure(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int session, unsigned int rendertarget, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int padding, unsigned int lineheight);
struct event_header *event_reply(struct event_header *oheader, struct event_header *iheader, unsigned int type);
struct event_header *event_replydata(struct event_header *oheader, struct event_header *iheader);
struct event_header *event_replystop(struct event_header *oheader, struct event_header *iheader);
