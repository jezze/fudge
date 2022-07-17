#define WIDGET_TYPE_IMAGE               1
#define WIDGET_TYPE_LAYOUT              2
#define WIDGET_TYPE_WINDOW              3
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

};

struct widget_image
{

    struct position position;
    struct size size;
    void *data;
    void *cmap;

};

struct widget_layout
{

    unsigned int type;
    struct position position;
    struct size size;

};

struct widget_window
{

    char *title;
    unsigned int focus;
    struct position position;
    struct size size;

};

void widget_init(struct widget *widget, unsigned int type, char *id, char *in, void *data);
void widget_initimage(struct widget_image *image, void *data, void *cmap);
void widget_initlayout(struct widget_layout *layout, unsigned int type);
void widget_initwindow(struct widget_window *window, char *title, int x, int y, unsigned int w, unsigned int h);
