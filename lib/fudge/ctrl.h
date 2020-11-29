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

void ctrl_setpartsettings(struct ctrl_partsettings *settings, unsigned int interface, unsigned int start, unsigned int end);
void ctrl_setvideosettings(struct ctrl_videosettings *settings, unsigned int w, unsigned int h, unsigned int bpp);
