#define WIDGET_TYPE_BUTTON              1
#define WIDGET_TYPE_FILL                2
#define WIDGET_TYPE_GRID                3
#define WIDGET_TYPE_IMAGE               4
#define WIDGET_TYPE_CONTAINER           5
#define WIDGET_TYPE_TEXT                6
#define WIDGET_TYPE_TEXTBOX             7
#define WIDGET_TYPE_WINDOW              8
#define WIDGET_ATTR_COLOR               1
#define WIDGET_ATTR_COLUMNS             2
#define WIDGET_ATTR_CONTENT             3
#define WIDGET_ATTR_ID                  4
#define WIDGET_ATTR_IN                  5
#define WIDGET_ATTR_LABEL               6
#define WIDGET_ATTR_LAYOUT              7
#define WIDGET_ATTR_MODE                8
#define WIDGET_ATTR_PADDING             9
#define WIDGET_ATTR_PLACEMENT           10
#define WIDGET_ATTR_TITLE               11
#define CONTAINER_LAYOUT_FLOAT          0
#define CONTAINER_LAYOUT_HORIZONTAL     1
#define CONTAINER_LAYOUT_MAXIMIZE       2
#define CONTAINER_LAYOUT_VERTICAL       3
#define CONTAINER_PLACEMENT_NORMAL      0
#define CONTAINER_PLACEMENT_STRETCHED   1
#define GRID_PLACEMENT_NORMAL           0
#define GRID_PLACEMENT_STRETCHED        1
#define TEXTBOX_MODE_NORMAL             0
#define TEXTBOX_MODE_READONLY           1
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

    unsigned int source;
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
    unsigned int placement;
    unsigned int padding;

};

struct widget_fill
{

    unsigned int color;

};

struct widget_grid
{

    unsigned int columns;
    unsigned int placement;
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
    unsigned int mode;
    int scroll;
    unsigned int cursor;

};

struct widget_window
{

    char *title;
    unsigned int focus;

};

void widget_setimage(struct widget_image *image, void *data, void *cmap);
void widget_setattribute(struct widget *widget, unsigned int attribute, char *value);
unsigned int widget_gettype(char *value);
unsigned int widget_getattribute(char *value);
void widget_init(struct widget *widget, unsigned int source, unsigned int type, char *id, char *in, void *data);
