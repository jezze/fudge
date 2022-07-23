#define WIDGET_TYPE_BUTTON              1
#define WIDGET_TYPE_FILL                2
#define WIDGET_TYPE_IMAGE               3
#define WIDGET_TYPE_CONTAINER           4
#define WIDGET_TYPE_TEXTBOX             5
#define WIDGET_TYPE_WINDOW              6
#define CONTAINER_LAYOUT_FLOAT          0
#define CONTAINER_LAYOUT_VERTICAL       1
#define CONTAINER_LAYOUT_GRID           2
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

struct widget_container
{

    unsigned int layout;

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

void widget_initbutton(struct widget_button *button, char *label);
void widget_initcontainer(struct widget_container *container, unsigned int layout);
void widget_initfill(struct widget_fill *fill, unsigned int color);
void widget_initimage(struct widget_image *image, void *data, void *cmap);
void widget_inittextbox(struct widget_textbox *textbox, unsigned int length, char *content);
void widget_initwindow(struct widget_window *window, char *title, unsigned int focus);
void widget_init(struct widget *widget, unsigned int type, char *id, char *in, void *data);
