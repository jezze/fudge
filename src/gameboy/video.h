#define LCD_WIDTH                       160
#define LCD_HEIGHT                      144
#define LCD_COLOUR                      0x03
#define LCD_PALETTE_OBJ                 0x10
#define LCD_PALETTE_BG                  0x20
#define LCD_PALETTE_ALL                 0x30

void video_setpalette(void);
void video_drawline(const unsigned char pixels[160], const unsigned char line);
void video_render(void *framebuffer, unsigned int w, unsigned int scalew, unsigned int scaleh, unsigned int totw, unsigned int toth, unsigned int offx, unsigned int offy);
