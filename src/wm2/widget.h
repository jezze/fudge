#define WIDGET_TYPE_BUTTON              1
#define WIDGET_TYPE_FILL                2
#define WIDGET_TYPE_GRID                3
#define WIDGET_TYPE_IMAGE               4
#define WIDGET_TYPE_CONTAINER           5
#define WIDGET_TYPE_TEXT                6
#define WIDGET_TYPE_TEXTBOX             7
#define WIDGET_TYPE_WINDOW              8
#define CONTAINER_LAYOUT_FLOAT          0
#define CONTAINER_LAYOUT_HORIZONTAL     1
#define CONTAINER_LAYOUT_VERTICAL       2
#define GRID_LAYOUT_NORMAL              0
#define GRID_LAYOUT_STRETCHED           1
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
    unsigned int padding;

};

struct widget_fill
{

    unsigned int color;

};

struct widget_grid
{

    unsigned int columns;
    unsigned int layout;
    unsigned int padding;

};

struct widget_image
{

    void *data;
    void *cmap;

};

struct widget_text
{

    unsigned int length;
    int scroll;
    char *content;

};

struct widget_textbox
{

    char *content;
    unsigned int length;
    int scroll;
    unsigned int cursor;

};

struct widget_window
{

    char *title;
    unsigned int focus;

};

void widget_initbutton(struct widget_button *button, char *label);
void widget_initcontainer(struct widget_container *container, unsigned int layout, unsigned int padding);
void widget_initfill(struct widget_fill *fill, unsigned int color);
void widget_initgrid(struct widget_grid *grid, unsigned int columns, unsigned int layout, unsigned int padding);
void widget_initimage(struct widget_image *image, void *data, void *cmap);
void widget_inittext(struct widget_text *text, unsigned int length, char *content);
void widget_inittextbox(struct widget_textbox *textbox, unsigned int length, char *content);
void widget_initwindow(struct widget_window *window, char *title);
void widget_init(struct widget *widget, unsigned int type, char *id, char *in, void *data);
