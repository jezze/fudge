#ifdef HIRES

#define SCREEN_WIDTH                    1024
#define SCREEN_HEIGHT                   768
#define SCREEN_BPP                      4
#define BORDERSIZE                      1
#define BOXSIZE                         24
#define WM_COLOR_DARK                   0x000000
#define WM_COLOR_LIGHT                  0xFFFFFF
#define WM_COLOR_BODY                   0x181014
#define WM_COLOR_PASSIVEFRAMETITLE      0x20181C
#define WM_COLOR_PASSIVEFRAME           0x282024
#define WM_COLOR_ACTIVEFRAMETITLE       0x206080
#define WM_COLOR_ACTIVEFRAME            0x286888

#else

#define SCREEN_WIDTH                    320
#define SCREEN_HEIGHT                   200
#define SCREEN_BPP                      1
#define BORDERSIZE                      1
#define BOXSIZE                         16
#define WM_COLOR_DARK                   0x00
#define WM_COLOR_LIGHT                  0x01
#define WM_COLOR_BODY                   0x02
#define WM_COLOR_PASSIVEFRAMETITLE      0x03
#define WM_COLOR_PASSIVEFRAME           0x04
#define WM_COLOR_ACTIVEFRAMETITLE       0x05
#define WM_COLOR_ACTIVEFRAME            0x06

#endif

void draw_setmode();
void draw_setcolormap();
void draw_begin();
void draw_end();
void backbuffer_drawrepeated(struct box *box);
void backbuffer_fill(unsigned int color, unsigned int count);
void backbuffer_fillbox(struct box *box, unsigned int color);
