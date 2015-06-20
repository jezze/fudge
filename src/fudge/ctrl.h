#define CTRL_TYPE_NULL                  0
#define CTRL_TYPE_CONSOLE               1
#define CTRL_TYPE_VIDEO                 2

struct ctrl_header
{

    unsigned int type;

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

void ctrl_initheader(struct ctrl_header *header, unsigned int type);
void ctrl_initconsolesettings(struct ctrl_consolesettings *settings, unsigned char scroll);
void ctrl_initvideosettings(struct ctrl_videosettings *settings, unsigned int w, unsigned int h, unsigned int bpp);
