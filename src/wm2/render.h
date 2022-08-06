#define RENDER_FONTNORMAL               0
#define RENDER_FONTBOLD                 1
#define RENDER_BUTTON_PADDING_WIDTH     36
#define RENDER_BUTTON_PADDING_HEIGHT    20
#define RENDER_CHOICE_PADDING_WIDTH     36
#define RENDER_CHOICE_PADDING_HEIGHT    20
#define RENDER_SELECT_PADDING_WIDTH     36
#define RENDER_SELECT_PADDING_HEIGHT    20
#define RENDER_TEXTBOX_PADDING_WIDTH    20
#define RENDER_TEXTBOX_PADDING_HEIGHT   20
#define RENDER_WINDOW_BORDER_WIDTH      5
#define RENDER_WINDOW_BORDER_HEIGHT     5
#define RENDER_WINDOW_TITLE_HEIGHT      36

struct render_damage
{

    unsigned int state;
    struct widget_position position0;
    struct widget_position position1;

};

struct render_display
{

    void *framebuffer;
    struct widget_size size;
    unsigned int bpp;
    struct render_damage damage;

};

struct render_rowinfo
{

    unsigned int chars;
    unsigned int width;
    unsigned int height;
    unsigned int lineheight;
    unsigned int newline;

};

struct render_textinfo
{

    unsigned int rows;
    unsigned int width;
    unsigned int height;
    unsigned int lineheight;
    struct render_rowinfo last;

};

unsigned int render_getrowinfo(unsigned int index, char *text, unsigned int length, struct render_rowinfo *rowinfo, unsigned int wrap, unsigned int maxw, unsigned int offset);
unsigned int render_gettextinfo(unsigned int index, char *text, unsigned int length, struct render_textinfo *textinfo, unsigned int wrap, unsigned int offw, unsigned int maxw);
void render_damage(struct render_display *display, int x0, int y0, int x1, int y1);
void render_damagebywidget(struct render_display *display, struct widget *widget);
void render_render(struct render_display *display);
void render_setfont(unsigned int index, void *data, unsigned int lineheight, unsigned int padding);
void render_setup(struct render_display *display, void *framebuffer, unsigned int w, unsigned int h, unsigned int bpp);
