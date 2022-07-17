#define WIDGET_TYPE_WINDOW              1
#define WIDGET_TYPE_LAYOUT              2
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

struct layout
{

    unsigned int type;
    struct position position;
    struct size size;

};

struct image
{

    struct size size;
    void *data;

};

struct mouse
{

    struct position position;
    struct image image;
    unsigned int drag;
    unsigned int resize;

};

struct window
{

    char *title;
    unsigned int focus;
    struct position position;
    struct size size;

};

void widget_init(struct widget *widget, unsigned int type, char *id, char *in, void *data);
void layout_init(struct layout *layout, unsigned int type);
void window_init(struct window *window, char *title, int x, int y, unsigned int w, unsigned int h);
