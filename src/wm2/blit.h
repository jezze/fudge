struct blit_damage
{

    unsigned int state;
    struct widget_position position0;
    struct widget_position position2;

};

struct blit_display
{

    void *framebuffer;
    struct widget_size size;
    unsigned int bpp;

};

struct blit_font
{

    unsigned char *data;
    unsigned char *bitmapdata;
    unsigned int bitmapalign;
    unsigned int lineheight;
    unsigned int padding;

};

void blit_line(struct blit_display *display, unsigned int color, int x0, int x2);
void blit_alphaline(struct blit_display *display, unsigned int color, int x0, int x2);
void blit_textnormal(struct blit_display *display, struct blit_font *font, unsigned int color, char *text, unsigned int length, int rx, int ry, int line, int x0, int x2);
void blit_textinverted(struct blit_display *display, struct blit_font *font, unsigned int color, char *text, unsigned int length, int rx, int ry, int line, int x0, int x2);
void blit_iconhamburger(struct blit_display *display, int x, int y, int w, int h, int line, int x0, int x2, unsigned int *cmap);
void blit_iconminimize(struct blit_display *display, int x, int y, int w, int h, int line, int x0, int x2, unsigned int *cmap);
void blit_iconx(struct blit_display *display, int x, int y, int w, int h, int line, int x0, int x2, unsigned int *cmap);
void blit_mouse(struct blit_display *display, int x, int y, int w, int h, int line, int x0, int x2, unsigned int *cmap);
void blit_panel(struct blit_display *display, int x, int y, int w, int h, int line, int x0, int x2, unsigned int *cmap);
void blit_frame(struct blit_display *display, int x, int y, int w, int h, int line, int x0, int x2, unsigned int *cmap);
void blit_pcx(struct blit_display *display, int line, char *source, int x, int y, int x0, int x2);
void blit_initdisplay(struct blit_display *display, void *framebuffer, unsigned int w, unsigned int h, unsigned int bpp);
void blit(struct blit_display *display, struct blit_damage *damage, int line);
