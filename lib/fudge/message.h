#define MESSAGE_SIZE                    0x1000
#define MESSAGE_SLOTS                   8
#define MESSAGE_CAPACITY                (MESSAGE_SIZE * MESSAGE_SLOTS)
#define MESSAGE_OK                      0x01
#define MESSAGE_RETRY                   0x02
#define MESSAGE_FAILED                  0x03
#define MESSAGE_FULL                    0x04
#define MESSAGE_FLAG_BLOCK              0x01
#define MESSAGE_ROOT                    0x01
#define MESSAGE_TYPE_MAIN               0x01
#define MESSAGE_TYPE_TERM               0x02
#define MESSAGE_TYPE_INTERRUPT          0x03
#define MESSAGE_TYPE_OPTION             0x04
#define MESSAGE_TYPE_PATH               0x05
#define MESSAGE_TYPE_DATA               0x06
#define MESSAGE_TYPE_ERROR              0x07
#define MESSAGE_TYPE_STATUS             0x08
#define MESSAGE_TYPE_LINK               0x09
#define MESSAGE_TYPE_UNLINK             0x0A
#define MESSAGE_TYPE_INFO               0x0B
#define MESSAGE_TYPE_QUERYREQUEST       0x0C
#define MESSAGE_TYPE_QUERYRESPONSE      0x0D
#define MESSAGE_TYPE_READY              0x0E
#define MESSAGE_TYPE_KEYPRESS           0x20
#define MESSAGE_TYPE_KEYRELEASE         0x21
#define MESSAGE_TYPE_MOUSEMOVE          0x22
#define MESSAGE_TYPE_MOUSESCROLL        0x23
#define MESSAGE_TYPE_MOUSEPRESS         0x24
#define MESSAGE_TYPE_MOUSERELEASE       0x25
#define MESSAGE_TYPE_CONSOLEDATA        0x26
#define MESSAGE_TYPE_TIMERTICK          0x27
#define MESSAGE_TYPE_VIDEOCMAP          0x28
#define MESSAGE_TYPE_VIDEOCONF          0x29
#define MESSAGE_TYPE_VIDEOINFO          0x2A
#define MESSAGE_TYPE_BLOCKINFO          0x2B
#define MESSAGE_TYPE_BLOCKREADREQUEST   0x2C
#define MESSAGE_TYPE_BLOCKREADRESPONSE  0x2D
#define MESSAGE_TYPE_BLOCKWRITEREQUEST  0x2E
#define MESSAGE_TYPE_BLOCKWRITERESPONSE 0x2F
#define MESSAGE_TYPE_CLOCKINFO          0x30
#define MESSAGE_TYPE_ETHERNETINFO       0x31
#define MESSAGE_TYPE_LOGINFO            0x32
#define MESSAGE_TYPE_AUDIOCONF          0x33
#define MESSAGE_TYPE_AUDIOINFO          0x34
#define MESSAGE_TYPE_VIDEOFLUSH         0x35
#define MESSAGE_TYPE_TIMERREQUEST       0x36
#define MESSAGE_TYPE_TIMEREXPIRE        0x37
#define MESSAGE_TYPE_TIMERINFO          0x38
#define MESSAGE_TYPE_MOUSEPOSITION      0x39
#define MESSAGE_TYPE_TOUCHCONTACT       0x3A
#define MESSAGE_TYPE_WALKREQUEST        0x40
#define MESSAGE_TYPE_WALKRESPONSE       0x41
#define MESSAGE_TYPE_READREQUEST        0x42
#define MESSAGE_TYPE_READRESPONSE       0x43
#define MESSAGE_TYPE_WRITEREQUEST       0x44
#define MESSAGE_TYPE_WRITERESPONSE      0x45
#define MESSAGE_TYPE_STATREQUEST        0x46
#define MESSAGE_TYPE_STATRESPONSE       0x47
#define MESSAGE_TYPE_MAPREQUEST         0x48
#define MESSAGE_TYPE_MAPRESPONSE        0x49
#define MESSAGE_TYPE_CREATEREQUEST      0x4A
#define MESSAGE_TYPE_CREATERESPONSE     0x4B
#define MESSAGE_TYPE_REMOVEREQUEST      0x4C
#define MESSAGE_TYPE_REMOVERESPONSE     0x4D
#define MESSAGE_TYPE_MMAPREQUEST        0x4E
#define MESSAGE_TYPE_MMAPRESPONSE       0x4F
#define MESSAGE_TYPE_MUNMAPREQUEST      0x50
#define MESSAGE_TYPE_MUNMAPRESPONSE     0x52
#define MESSAGE_TYPE_WMUNMAP            0x51
#define MESSAGE_TYPE_WMGRAB             0x53
#define MESSAGE_TYPE_WMUNGRAB           0x54
#define MESSAGE_TYPE_WMKEYPRESS         0x55
#define MESSAGE_TYPE_WMKEYRELEASE       0x56
#define MESSAGE_TYPE_WMMOUSEMOVE        0x57
#define MESSAGE_TYPE_WMMOUSESCROLL      0x58
#define MESSAGE_TYPE_WMMOUSEPRESS       0x59
#define MESSAGE_TYPE_WMMOUSERELEASE     0x5A
#define MESSAGE_TYPE_WMRENDERDATA       0x5B
#define MESSAGE_TYPE_WMRENDERFILE       0x5C
#define MESSAGE_TYPE_WMFLUSH            0x5D
#define MESSAGE_TYPE_WMEVENT            0x5E
#define MESSAGE_TYPE_WMACK              0x5F
#define MESSAGE_TYPE_WMREAD             0x60
#define MESSAGE_TYPE_WMTEXT             0x61
#define MESSAGE_TYPE_WMCONFIG           0x62
#define MESSAGE_TYPE_P9P                0x70
#define PIPE_ROUTES                     10
#define READ_FLAG_BUFFER                0x01

struct message
{

    unsigned int type;
    unsigned int source;
    unsigned int length;
    void *data;
    unsigned int slot;

};

struct message_route
{

    unsigned int event;
    unsigned int to;
    unsigned int target;

};

/* a complete routing table for one stage; target 0 means the owner */
struct message_pipe
{

    unsigned int prev;
    unsigned int nroutes;
    struct message_route routes[PIPE_ROUTES];
    unsigned int nterms;
    struct message_route terms[PIPE_ROUTES];

};

struct message_loginfo
{

    unsigned int level;
    unsigned int count;

};

struct message_keypress
{

    unsigned char scancode;

};

struct message_keyrelease
{

    unsigned char scancode;

};

struct message_mousemove
{

    char relx;
    char rely;
    unsigned char buttons;

};

struct message_mouseposition
{

    unsigned short x;
    unsigned short y;
    unsigned char buttons;

};

struct message_touchcontact
{

    unsigned short x;
    unsigned short y;
    unsigned short xmax;
    unsigned short ymax;
    unsigned char id;
    unsigned char tip;
    unsigned char count;
    unsigned char button;
    unsigned int time;

};

struct message_mousescroll
{

    char relz;

};

struct message_mousepress
{

    unsigned int button;

};

struct message_mouserelease
{

    unsigned int button;

};

struct message_timertick
{

    unsigned int counter;

};

struct message_timerrequest
{

    unsigned int id;
    unsigned int ms;
    unsigned int repeat;

};

struct message_timerexpire
{

    unsigned int id;

};

struct message_timerinfo
{

    unsigned int counter;
    unsigned int hz;

};

struct message_ethernetinfo
{

    unsigned char address[6];

};

struct message_videoconf
{

    unsigned int width;
    unsigned int height;
    unsigned int bpp;

};

struct message_videoflush
{

    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;

};

struct message_videoinfo
{

    unsigned long framebuffer;
    unsigned int width;
    unsigned int height;
    unsigned int bpp;

};

struct message_audioconf
{

    unsigned int rate;
    unsigned int channels;
    unsigned int bits;

};

struct message_audioinfo
{

    unsigned int rate;
    unsigned int channels;
    unsigned int bits;

};

struct message_blockinfo
{

    unsigned long buffer;
    unsigned int buffersize;
    unsigned int blocksize;
    unsigned int blockcount;

};

struct message_blockrequest
{

    unsigned int count;
    unsigned int offset;
    unsigned int target;

};

struct message_blockresponse
{

    unsigned int count;

};

struct message_wmkeypress
{

    unsigned int id;
    unsigned char scancode;
    unsigned int keymod;
    unsigned int unicode;
    unsigned int length;

};

struct message_wmkeyrelease
{

    unsigned int id;
    unsigned char scancode;
    unsigned int keymod;
    unsigned int unicode;
    unsigned int length;

};

struct message_wmmousemove
{

    char relx;
    char rely;

};

struct message_wmmousescroll
{

    char relz;

};

struct message_wmmousepress
{

    unsigned int button;

};

struct message_wmmouserelease
{

    unsigned int button;

};

struct message_wmevent
{

    unsigned int type;
    unsigned int length;

};

struct message_createrequest
{

    unsigned int parent;
    unsigned int count;

};

struct message_createresponse
{

    unsigned int id;

};

struct message_maprequest
{

    unsigned int id;

};

struct message_mapresponse
{

    unsigned int address;

};

struct message_mmaprequest
{

    unsigned int id;
    unsigned int size;

};

struct message_mmapresponse
{

    unsigned int address;

};

struct message_munmaprequest
{

    unsigned int id;

};

struct message_munmapresponse
{

    unsigned int status;

};

struct message_readrequest
{

    unsigned int id;
    unsigned int offset;
    unsigned int count;
    unsigned int flags;

};

struct message_readresponse
{

    unsigned int count;
    unsigned long buffer;

};

struct message_removerequest
{

    unsigned int parent;
    unsigned int id;

};

struct message_removeresponse
{

    unsigned int status;

};

struct message_statrequest
{

    unsigned int id;

};

struct message_statresponse
{

    unsigned int count;

};

struct message_walkrequest
{

    unsigned int parent;
    unsigned int length;

};

struct message_walkresponse
{

    unsigned int id;

};

struct message_writerequest
{

    unsigned int id;
    unsigned int offset;
    unsigned int count;

};

struct message_writeresponse
{

    unsigned int count;

};

struct message_clockinfo
{

    unsigned char seconds;
    unsigned char minutes;
    unsigned char hours;
    unsigned char weekday;
    unsigned char day;
    unsigned char month;
    unsigned short year;

};

struct message_coreinfo
{

    unsigned int id;
    unsigned int sp;
    unsigned int task;

};

struct message_taskinfo
{

    unsigned int id;
    unsigned int state;
    unsigned int signals_kills;
    unsigned int signals_blocks;
    unsigned int thread_ip;
    unsigned int thread_sp;

};

struct message_mailboxinfo
{

    unsigned int address;

};

void message_init(struct message *message, unsigned int event, unsigned int source, unsigned int length, void *data);
void message_initpipe(struct message_pipe *pipe);
void *message_validate(unsigned int type, void *data, unsigned int count);
