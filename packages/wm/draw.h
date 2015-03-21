#ifdef HIRES

#define SCREEN_WIDTH                    1024
#define SCREEN_HEIGHT                   768
#define SCREEN_BPP                      4

#else

#define SCREEN_WIDTH                    320
#define SCREEN_HEIGHT                   200
#define SCREEN_BPP                      1

#endif

#define BOXSIZE                         32
#define WM_COLOR_DARK                   0x00
#define WM_COLOR_LIGHT                  0x01
#define WM_COLOR_BODY                   0x02
#define WM_COLOR_PASSIVEBACK            0x03
#define WM_COLOR_PASSIVEFRAME           0x04
#define WM_COLOR_ACTIVEBACK             0x05
#define WM_COLOR_ACTIVEFRAME            0x06
#define WM_COLOR_POINTERBACK            0x07
#define WM_COLOR_POINTERFRAME           0x08

void draw_setmode();
void draw_setcolormap();
void draw_begin();
void draw_end();
void backbuffer_drawline(unsigned int line, unsigned int count);
void backbuffer_fillbox(struct box *box, unsigned int color, unsigned int line);
void backbuffer_fillcount(unsigned int color, unsigned int offset, unsigned int count);
