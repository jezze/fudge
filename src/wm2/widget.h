#define WIDGET_TYPE_BUTTON              1
#define WIDGET_TYPE_FILL                2
#define WIDGET_TYPE_IMAGE               3
#define WIDGET_TYPE_LAYOUT              4
#define WIDGET_TYPE_TEXTBOX             5
#define WIDGET_TYPE_WINDOW              6
#define LAYOUT_TYPE_FLOAT               0
#define LAYOUT_TYPE_VERTICAL            1
#define WINDOW_MIN_WIDTH                128
#define WINDOW_MIN_HEIGHT               128

struct position
{

    int x;
    int y;

};

struct size
{

    unsigned int w;
    unsigned int h;

};

struct widget
{

    unsigned int type;
    char *id;
    char *in;
    void *data;
    struct position position;
    struct size size;

};

struct widget_button
{

    char *label;
    unsigned int focus;

};

struct widget_fill
{

    unsigned int color;

};

struct widget_image
{

    void *data;
    void *cmap;

};

struct widget_layout
{

    unsigned int type;

};

struct widget_textbox
{

    unsigned int length;
    unsigned int cursor;
    unsigned int offset;
    int scroll;
    char *content;

};

struct widget_window
{

    char *title;
    unsigned int focus;

};

void widget_init(struct widget *widget, unsigned int type, char *id, char *in, void *data);
void widget_initbutton(struct widget_button *button, char *label);
void widget_initfill(struct widget_fill *fill, unsigned int color);
void widget_initimage(struct widget_image *image, void *data, void *cmap);
void widget_initlayout(struct widget_layout *layout, unsigned int type);
void widget_inittextbox(struct widget_textbox *textbox, unsigned int length, char *content);
void widget_initwindow(struct widget_window *window, char *title, unsigned int focus);
