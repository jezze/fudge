#define EVENT_MAIN                      0x01
#define EVENT_EXIT                      0x02
#define EVENT_END                       0x03
#define EVENT_TERM                      0x04
#define EVENT_DONE                      0x05
#define EVENT_OPTION                    0x06
#define EVENT_PATH                      0x07
#define EVENT_DATA                      0x08
#define EVENT_ERROR                     0x09
#define EVENT_STATUS                    0x0A
#define EVENT_LINK                      0x0B
#define EVENT_UNLINK                    0x0C
#define EVENT_INFO                      0x0D
#define EVENT_QUERYREQUEST              0x0E
#define EVENT_QUERYRESPONSE             0x0F
#define EVENT_READY                     0x10
#define EVENT_KEYPRESS                  0x20
#define EVENT_KEYRELEASE                0x21
#define EVENT_MOUSEMOVE                 0x22
#define EVENT_MOUSESCROLL               0x23
#define EVENT_MOUSEPRESS                0x24
#define EVENT_MOUSERELEASE              0x25
#define EVENT_CONSOLEDATA               0x26
#define EVENT_TIMERTICK                 0x27
#define EVENT_VIDEOCMAP                 0x28
#define EVENT_VIDEOCONF                 0x29
#define EVENT_VIDEOINFO                 0x2A
#define EVENT_BLOCKREQUEST              0x2B
#define EVENT_BLOCKRESPONSE             0x2C
#define EVENT_CLOCKINFO                 0x2D
#define EVENT_ETHERNETINFO              0x2E
#define EVENT_LOGINFO                   0x2F
#define EVENT_WALKREQUEST               0x40
#define EVENT_WALKRESPONSE              0x41
#define EVENT_READREQUEST               0x42
#define EVENT_READRESPONSE              0x43
#define EVENT_LISTREQUEST               0x44
#define EVENT_LISTRESPONSE              0x45
#define EVENT_WRITEREQUEST              0x46
#define EVENT_WRITERESPONSE             0x47
#define EVENT_STATREQUEST               0x48
#define EVENT_STATRESPONSE              0x49
#define EVENT_MAPREQUEST                0x4A
#define EVENT_MAPRESPONSE               0x4B
#define EVENT_WMMAP                     0x50
#define EVENT_WMUNMAP                   0x51
#define EVENT_WMGRAB                    0x52
#define EVENT_WMUNGRAB                  0x53
#define EVENT_WMSHOW                    0x54
#define EVENT_WMHIDE                    0x55
#define EVENT_WMKEYPRESS                0x56
#define EVENT_WMKEYRELEASE              0x57
#define EVENT_WMMOUSEMOVE               0x58
#define EVENT_WMMOUSESCROLL             0x59
#define EVENT_WMMOUSEPRESS              0x5A
#define EVENT_WMMOUSERELEASE            0x5B
#define EVENT_WMRENDERDATA              0x5C
#define EVENT_WMINIT                    0x5D
#define EVENT_WMEVENT                   0x5E
#define EVENT_WMACK                     0x5F
#define EVENT_P9P                       0x60

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

struct event_videoconf
{

    unsigned int width;
    unsigned int height;
    unsigned int bpp;

};

struct event_videoinfo
{

    unsigned int framebuffer;
    unsigned int width;
    unsigned int height;
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
    unsigned int offset;
    unsigned int nrecords;

};

struct event_listresponse
{

    unsigned int session;
    unsigned int nrecords;

};

struct event_maprequest
{

    unsigned int session;
    unsigned int id;

};

struct event_mapresponse
{

    unsigned int session;
    unsigned int address;

};

struct event_readrequest
{

    unsigned int session;
    unsigned int id;
    unsigned int offset;
    unsigned int count;
    void *buffer;

};

struct event_readresponse
{

    unsigned int session;
    unsigned int count;

};

struct event_statrequest
{

    unsigned int session;
    unsigned int id;

};

struct event_statresponse
{

    unsigned int session;
    unsigned int nrecords;

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
    void *buffer;

};

struct event_writeresponse
{

    unsigned int session;
    unsigned int count;

};

struct event_clockinfo
{

    unsigned char seconds;
    unsigned char minutes;
    unsigned char hours;
    unsigned char weekday;
    unsigned char day;
    unsigned char month;
    unsigned short year;

};

struct event_coreinfo
{

    unsigned int id;
    unsigned int sp;
    unsigned int task;

};

struct event_taskinfo
{

    unsigned int id;
    unsigned int state;
    unsigned int signals_kills;
    unsigned int signals_blocks;
    unsigned int thread_ip;
    unsigned int thread_sp;

};

struct event_mailboxinfo
{

    unsigned int address;

};
