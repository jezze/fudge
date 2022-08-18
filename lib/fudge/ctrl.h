struct ctrl_clocksettings
{

    unsigned char seconds;
    unsigned char minutes;
    unsigned char hours;
    unsigned char weekday;
    unsigned char day;
    unsigned char month;
    unsigned short year;

};

struct ctrl_consolesettings
{

    unsigned char scroll;

};

struct ctrl_partsettings
{

    unsigned int interface;
    unsigned int start;
    unsigned int end;

};

struct ctrl_videosettings
{

    unsigned int w;
    unsigned int h;
    unsigned int bpp;

};

struct ctrl_timertick
{

    unsigned int jiffies;
    unsigned int repeat;

};

struct ctrl_core
{

    unsigned int id;

};

struct ctrl_task
{

    unsigned int id;
    unsigned int state;
    unsigned int signals_kills;
    unsigned int signals_blocks;
    unsigned int signals_unblocks;
    unsigned int thread_ip;
    unsigned int thread_sp;

};

struct ctrl_mailbox
{

    unsigned int address;

};
