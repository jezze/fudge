struct blit_display
{

    void *framebuffer;
    struct util_size size;
    unsigned int bpp;
    unsigned int *linebuffer;

};

void blit_line(struct blit_display *display, unsigned int color, int x0, int x2);
void blit_alphaline(struct blit_display *display, unsigned int color, int x0, int x2);
void blit_text(struct blit_display *display, struct text_font *font, unsigned int invert, char *text, unsigned int length, int rx, int ry, int line, int x0, int x2, unsigned int *cmap);
void blit_iconhamburger(struct blit_display *display, int x, int y, int w, int h, int line, int x0, int x2, unsigned int *cmap);
void blit_iconminimize(struct blit_display *display, int x, int y, int w, int h, int line, int x0, int x2, unsigned int *cmap);
void blit_iconx(struct blit_display *display, int x, int y, int w, int h, int line, int x0, int x2, unsigned int *cmap);
void blit_mouse(struct blit_display *display, int x, int y, int w, int h, int line, int x0, int x2, unsigned int *cmap);
void blit_panel(struct blit_display *display, int x, int y, int w, int h, int line, int x0, int x2, unsigned int *cmap);
void blit_rect(struct blit_display *display, int x, int y, int w, int h, int line, int x0, int x2, unsigned int *cmap);
void blit_frame(struct blit_display *display, int x, int y, int w, int h, int line, int x0, int x2, unsigned int *cmap);
void blit_pcx(struct blit_display *display, int line, char *source, int x, int y, int x0, int x2);
void blit_initdisplay(struct blit_display *display, void *framebuffer, unsigned int w, unsigned int h, unsigned int bpp, unsigned int *linebuffer);
void blit(struct blit_display *display, int line, int dx0, int dx2);
