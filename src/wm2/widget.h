#define WIDGET_COMMAND_NONE                     1
#define WIDGET_COMMAND_COMMENT                  2
#define WIDGET_COMMAND_DELETE                   3
#define WIDGET_COMMAND_INSERT                   4
#define WIDGET_COMMAND_UPDATE                   5
#define WIDGET_TYPE_BUTTON                      1
#define WIDGET_TYPE_FILL                        2
#define WIDGET_TYPE_GRID                        3
#define WIDGET_TYPE_IMAGE                       4
#define WIDGET_TYPE_CHOICE                      5
#define WIDGET_TYPE_CONTAINER                   6
#define WIDGET_TYPE_SELECT                      7
#define WIDGET_TYPE_TEXT                        8
#define WIDGET_TYPE_TEXTBOX                     9
#define WIDGET_TYPE_WINDOW                      10
#define WIDGET_STATE_NORMAL                     0
#define WIDGET_STATE_HOVER                      1
#define WIDGET_STATE_HOVEROFF                   2
#define WIDGET_STATE_FOCUS                      3
#define WIDGET_STATE_FOCUSOFF                   4
#define WIDGET_ATTR_COLOR                       1
#define WIDGET_ATTR_COLUMNS                     2
#define WIDGET_ATTR_CONTENT                     3
#define WIDGET_ATTR_HALIGN                      4
#define WIDGET_ATTR_ID                          5
#define WIDGET_ATTR_IN                          6
#define WIDGET_ATTR_LABEL                       7
#define WIDGET_ATTR_LAYOUT                      8
#define WIDGET_ATTR_MODE                        9
#define WIDGET_ATTR_PADDING                     10
#define WIDGET_ATTR_PLACEMENT                   11
#define WIDGET_ATTR_SOURCE                      12
#define WIDGET_ATTR_TITLE                       13
#define WIDGET_ATTR_TYPE                        14
#define WIDGET_ATTR_WEIGHT                      15
#define WIDGET_ATTR_VALIGN                      16
#define WIDGET_ATTR_WRAP                        17
#define WIDGET_CONTAINER_LAYOUT_FLOAT           0
#define WIDGET_CONTAINER_LAYOUT_HORIZONTAL      1
#define WIDGET_CONTAINER_LAYOUT_MAXIMIZE        2
#define WIDGET_CONTAINER_LAYOUT_VERTICAL        3
#define WIDGET_CONTAINER_PLACEMENT_NORMAL       0
#define WIDGET_CONTAINER_PLACEMENT_STRETCHED    1
#define WIDGET_GRID_PLACEMENT_NORMAL            0
#define WIDGET_GRID_PLACEMENT_STRETCHED         1
#define WIDGET_IMAGE_TYPE_INTERNAL              1
#define WIDGET_IMAGE_TYPE_PCX                   2
#define WIDGET_TEXT_HALIGN_LEFT                 0
#define WIDGET_TEXT_HALIGN_CENTER               1
#define WIDGET_TEXT_HALIGN_RIGHT                2
#define WIDGET_TEXT_MODE_NORMAL                 0
#define WIDGET_TEXT_MODE_INVERTED               1
#define WIDGET_TEXT_WEIGHT_NORMAL               0
#define WIDGET_TEXT_WEIGHT_BOLD                 1
#define WIDGET_TEXT_VALIGN_TOP                  0
#define WIDGET_TEXT_VALIGN_MIDDLE               1
#define WIDGET_TEXT_VALIGN_BOTTOM               2
#define WIDGET_TEXT_WRAP_NONE                   0
#define WIDGET_TEXT_WRAP_CHAR                   1
#define WIDGET_TEXT_WRAP_WORD                   2
#define WIDGET_TEXTBOX_MODE_NORMAL              0
#define WIDGET_TEXTBOX_MODE_READONLY            1

struct widget_position
{

    int x;
    int y;

};

struct widget_size
{

    unsigned int w;
    unsigned int h;

};

struct widget
{

    unsigned int source;
    unsigned int type;
    unsigned int state;
    unsigned int id;
    unsigned int in;
    void *data;
    struct widget_position position;
    struct widget_size size;

};

struct widget_button
{

    unsigned int label;

};

struct widget_choice
{

    unsigned int label;

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

    unsigned int type;
    unsigned int source;
    void *data;
    void *cmap;

};

struct widget_select
{

    unsigned int label;

};

struct widget_text
{

    unsigned int content;
    unsigned int halign;
    unsigned int valign;
    unsigned int mode;
    unsigned int weight;
    unsigned int wrap;
    unsigned int firstrowoffset;
    unsigned int rownum;
    unsigned int rowstart;

};

struct widget_textbox
{

    unsigned int mode;
    int scroll;

};

struct widget_window
{

    unsigned int title;

};

void widget_setattribute(struct widget *widget, unsigned int attribute, char *value);
unsigned int widget_getattribute(char *value);
unsigned int widget_getcommand(char *value);
unsigned int widget_gettype(char *value);
unsigned int widget_setstate(struct widget *widget, unsigned int state);
unsigned int widget_intersectsx(struct widget *widget, int x);
unsigned int widget_intersectsy(struct widget *widget, int y);
void widget_init(struct widget *widget, unsigned int source, unsigned int type, char *id, char *in, void *data);
