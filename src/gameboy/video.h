#define LCD_WIDTH                       160
#define LCD_HEIGHT                      144
#define LCD_COLOUR                      0x03
#define LCD_PALETTE_OBJ                 0x10
#define LCD_PALETTE_BG                  0x20
#define LCD_PALETTE_ALL                 0x30

void video_setpalette(void);
void video_drawline(const unsigned char pixels[160], const unsigned char line);
unsigned int *video_getfb(void);
