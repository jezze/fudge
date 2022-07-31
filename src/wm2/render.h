#define RENDER_FONTNORMAL               0
#define RENDER_FONTBOLD                 1
#define RENDER_BUTTON_PADDING_WIDTH     40
#define RENDER_BUTTON_PADDING_HEIGHT    24
#define RENDER_TEXTBOX_PADDING_WIDTH    24
#define RENDER_TEXTBOX_PADDING_HEIGHT   24
#define RENDER_WINDOW_BORDER_WIDTH      5
#define RENDER_WINDOW_BORDER_HEIGHT     5
#define RENDER_WINDOW_TITLE_HEIGHT      36

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

unsigned int render_getrowwidth(unsigned int index, char *text, unsigned int length);
unsigned int render_getrowheight(unsigned int index, char *text, unsigned int length);
unsigned int render_gettextwidth(unsigned int index, char *text, unsigned int length);
unsigned int render_gettextheight(unsigned int index, char *text, unsigned int length, unsigned int lineheight);
void render_damage(struct render_display *display, int x0, int y0, int x1, int y1);
void render_damagebywidget(struct render_display *display, struct widget *widget);
void render_paint(struct render_display *display, struct widget *rootwidget);
void render_setfont(unsigned int index, void *data, unsigned int lineheight, unsigned int padding);
void render_setup(struct render_display *display, void *framebuffer, unsigned int w, unsigned int h, unsigned int bpp);
