#define WM_COLOR_DARK                   0x00
#define WM_COLOR_LIGHT                  0x01
#define WM_COLOR_BODY                   0x02
#define WM_COLOR_PASSIVEBACK            0x03
#define WM_COLOR_PASSIVEFRAME           0x04
#define WM_COLOR_ACTIVEBACK             0x05
#define WM_COLOR_ACTIVEFRAME            0x06
#define WM_COLOR_POINTERBACK            0x07
#define WM_COLOR_POINTERFRAME           0x08
#define WM_COLOR_TEXTDARK               0x09
#define WM_COLOR_TEXTLIGHT              0x0A

void draw_fill(void *buffer, unsigned int bpp, unsigned int color, unsigned int offset, unsigned int count);
void draw_flush(void *buffer, unsigned int line, unsigned int bpp, unsigned int offset, unsigned int count);
void draw_init(void);
