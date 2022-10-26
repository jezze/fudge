struct blit_damage
{

    unsigned int state;
    struct widget_position position0;
    struct widget_position position1;

};

struct blit_display
{

    void *framebuffer;
    struct widget_size size;
    unsigned int bpp;
    struct blit_damage damage;

};

struct blit_font
{

    unsigned char *data;
    unsigned char *bitmapdata;
    unsigned int bitmapalign;
    unsigned int lineheight;
    unsigned int padding;

};

void blit_line(struct blit_display *display, unsigned int color, int line, int x0, int x1);
void blit_alphaline(struct blit_display *display, unsigned int color, int line, int x0, int x1);
void blit_cmap32line(struct blit_display *display, int x, void *idata, unsigned int iwidth, unsigned int *cmap, int lline);
void blit_textnormal(struct blit_display *display, struct blit_font *font, unsigned int color, char *text, unsigned int length, int rx, int ry, int line, int x0, int x1);
void blit_textinverted(struct blit_display *display, struct blit_font *font, unsigned int color, char *text, unsigned int length, int rx, int ry, int line, int x0, int x1);
void blit_panel(struct blit_display *display, int x, int y, int w, int h, int line, int x0, int x1, unsigned int *cmap);
void blit_frame(struct blit_display *display, int x, int y, int w, int h, int line, int x0, int x1, unsigned int *cmap);
void blit_pcx(struct blit_display *display, int line, char *source, int x, int y, int x0, int x1);
void blit(struct blit_display *display, int line);
