#define SCREEN_WIDTH                    320
#define SCREEN_HEIGHT                   200
#define SCREEN_BORDER                   2

enum
{

    WM_COLOR_DARK                       = 0x00,
    WM_COLOR_LIGHT                      = 0x01,
    WM_COLOR_BODY                       = 0x02,
    WM_COLOR_PASSIVEFRAMETITLE          = 0x03,
    WM_COLOR_PASSIVEFRAME               = 0x04,
    WM_COLOR_ACTIVEFRAMETITLE           = 0x05,
    WM_COLOR_ACTIVEFRAME                = 0x06

};

void draw_setmode();
void draw_setcolormap();
void draw_begin();
void draw_end();
void draw_buffer(unsigned int offset, unsigned int count, unsigned int o, unsigned char *buffer);
void backbuffer_drawnormal(struct box *box);
void backbuffer_drawrepeated(struct box *box);
void backbuffer_fill(unsigned char color, unsigned int count);
void backbuffer_blitbuffer(const unsigned char *buffer, unsigned int count);
void backbuffer_fillbox(struct box *box, unsigned char color);
