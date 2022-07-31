#define WIDGET_COMMAND_NONE                     1
#define WIDGET_COMMAND_COMMENT                  2
#define WIDGET_COMMAND_DELETE                   3
#define WIDGET_COMMAND_INSERT                   4
#define WIDGET_COMMAND_UPDATE                   5
#define WIDGET_TYPE_BUTTON                      1
#define WIDGET_TYPE_FILL                        2
#define WIDGET_TYPE_GRID                        3
#define WIDGET_TYPE_IMAGE                       4
#define WIDGET_TYPE_CONTAINER                   5
#define WIDGET_TYPE_TEXT                        6
#define WIDGET_TYPE_TEXTBOX                     7
#define WIDGET_TYPE_WINDOW                      8
#define WIDGET_STATE_NORMAL                     0
#define WIDGET_STATE_HOVER                      1
#define WIDGET_STATE_FOCUS                      2
#define WIDGET_ATTR_ALIGN                       1
#define WIDGET_ATTR_COLOR                       2
#define WIDGET_ATTR_COLUMNS                     3
#define WIDGET_ATTR_CONTENT                     4
#define WIDGET_ATTR_ID                          5
#define WIDGET_ATTR_IN                          6
#define WIDGET_ATTR_LABEL                       7
#define WIDGET_ATTR_LAYOUT                      8
#define WIDGET_ATTR_MODE                        9
#define WIDGET_ATTR_PADDING                     10
#define WIDGET_ATTR_PLACEMENT                   11
#define WIDGET_ATTR_TITLE                       12
#define WIDGET_CONTAINER_LAYOUT_FLOAT           0
#define WIDGET_CONTAINER_LAYOUT_HORIZONTAL      1
#define WIDGET_CONTAINER_LAYOUT_MAXIMIZE        2
#define WIDGET_CONTAINER_LAYOUT_VERTICAL        3
#define WIDGET_CONTAINER_PLACEMENT_NORMAL       0
#define WIDGET_CONTAINER_PLACEMENT_STRETCHED    1
#define WIDGET_GRID_PLACEMENT_NORMAL            0
#define WIDGET_GRID_PLACEMENT_STRETCHED         1
#define WIDGET_TEXT_ALIGN_LEFT                  0
#define WIDGET_TEXT_ALIGN_CENTER                1
#define WIDGET_TEXT_ALIGN_RIGHT                 2
#define WIDGET_TEXTBOX_MODE_NORMAL              0
#define WIDGET_TEXTBOX_MODE_READONLY            1
#define WIDGET_TEXTBOX_ALIGN_LEFT               0
#define WIDGET_TEXTBOX_ALIGN_CENTER             1
#define WIDGET_TEXTBOX_ALIGN_RIGHT              2

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
    unsigned int state;
    char *id;
    char *in;
    void *data;
    struct position position;
    struct size size;

};

struct widget_button
{

    char *label;

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

    char *content;
    unsigned int length;
    unsigned int align;
    int scroll;

};

struct widget_textbox
{

    char *content;
    unsigned int length;
    unsigned int align;
    int scroll;
    unsigned int mode;
    unsigned int cursor;

};

struct widget_window
{

    char *title;

};

void widget_setimage(struct widget_image *image, void *data, void *cmap);
void widget_setattribute(struct widget *widget, unsigned int attribute, char *value);
unsigned int widget_getattribute(char *value);
unsigned int widget_getcommand(char *value);
unsigned int widget_gettype(char *value);
void widget_init(struct widget *widget, unsigned int source, unsigned int type, char *id, char *in, void *data);
