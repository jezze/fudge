#define CTRL_TYPE_NULL                  0
#define CTRL_TYPE_CLOCK                 1
#define CTRL_TYPE_CON                   2
#define CTRL_TYPE_CONSOLE               3
#define CTRL_TYPE_VIDEO                 4

struct ctrl_header
{

    unsigned int type;

};

struct ctrl_clocksettings
{

    struct ctrl_header header;
    unsigned char seconds;
    unsigned char minutes;
    unsigned char hours;
    unsigned char weekday;
    unsigned char day;
    unsigned char month;
    unsigned short year;

};

struct ctrl_consettings
{

    struct ctrl_header header;
    unsigned int interface;
    unsigned int protocol;
    unsigned int port;

};

struct ctrl_consolesettings
{

    struct ctrl_header header;
    unsigned char scroll;

};

struct ctrl_videosettings
{

    struct ctrl_header header;
    unsigned int w;
    unsigned int h;
    unsigned int bpp;

};

void ctrl_setheader(struct ctrl_header *header, unsigned int type);
void ctrl_setclocksettings(struct ctrl_clocksettings *settings, unsigned char seconds, unsigned char minutes, unsigned char hours, unsigned char weekday, unsigned char day, unsigned char month, unsigned short year);
void ctrl_setconsettings(struct ctrl_consettings *settings, unsigned int interface, unsigned int protocol, unsigned int port);
void ctrl_setconsolesettings(struct ctrl_consolesettings *settings, unsigned char scroll);
void ctrl_setvideosettings(struct ctrl_videosettings *settings, unsigned int w, unsigned int h, unsigned int bpp);
