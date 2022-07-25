struct render_damage
{

    unsigned int state;
    struct position position0;
    struct position position1;

};

struct render_display
{

    void *framebuffer;
    struct size size;
    unsigned int bpp;
    struct render_damage damage;

};

unsigned int render_getrowwidth(char *text, unsigned int length);
unsigned int render_getrowheight(char *text, unsigned int length);
unsigned int render_gettextwidth(char *text, unsigned int length);
unsigned int render_gettextheight(char *text, unsigned int length);
void render_damage(struct render_display *display, int x0, int y0, int x1, int y1);
void render_damagebywidget(struct render_display *display, struct widget *widget);
void render_paint(struct render_display *display, struct widget *rootwidget, struct widget_image *image);
void render_setfont(unsigned int index, void *data, unsigned int lineheight, unsigned int padding);
void render_setup(struct render_display *display, void *framebuffer, unsigned int w, unsigned int h, unsigned int bpp);
