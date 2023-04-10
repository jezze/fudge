#define WIDGET_TYPE_BUTTON              1
#define WIDGET_TYPE_CHOICE              2
#define WIDGET_TYPE_FILL                3
#define WIDGET_TYPE_IMAGE               5
#define WIDGET_TYPE_LAYOUT              6
#define WIDGET_TYPE_LISTBOX             7
#define WIDGET_TYPE_SELECT              8
#define WIDGET_TYPE_TEXT                9
#define WIDGET_TYPE_TEXTBOX             10
#define WIDGET_TYPE_TEXTBUTTON          11
#define WIDGET_TYPE_WINDOW              12
#define WIDGET_STATE_NORMAL             0
#define WIDGET_STATE_HOVER              1
#define WIDGET_STATE_HOVEROFF           2
#define WIDGET_STATE_FOCUS              3
#define WIDGET_STATE_FOCUSOFF           4
#define WIDGET_STATE_DESTROYED          5
#define WIDGET_DISPLAY_NORMAL           0
#define WIDGET_DISPLAY_HIDDEN           1

struct widget_button
{

    unsigned int label;
    unsigned int onclick;
    struct cache_row cacherow;

};

struct widget_choice
{

    unsigned int label;
    unsigned int onclick;
    struct cache_row cacherow;

};

struct widget_layout
{

    unsigned int form;
    unsigned int padding;

};

struct widget_fill
{

    unsigned int color;

};

struct widget_image
{

    unsigned int mimetype;
    unsigned int source;
    struct cache_image cacheimage;

};

struct widget_listbox
{

    unsigned int mode;
    unsigned int overflow;
    int hscroll;
    int vscroll;

};

struct widget_select
{

    unsigned int label;
    unsigned int onclick;
    struct cache_row cacherow;

};

struct widget_text
{

    unsigned int blit;
    unsigned int content;
    unsigned int halign;
    unsigned int valign;
    unsigned int weight;
    unsigned int wrap;
    struct cache_row cacherow;
    struct cache_text cachetext;

};

struct widget_textbox
{

    unsigned int mode;
    unsigned int overflow;
    int hscroll;
    int vscroll;

};

struct widget_textbutton
{

    unsigned int label;
    unsigned int onclick;
    struct cache_row cacherow;

};

struct widget_window
{

    unsigned int title;

};

struct widget
{

    unsigned int source;
    unsigned int type;
    unsigned int state;
    unsigned int id;
    unsigned int in;
    void *data;
    struct util_position position;
    struct util_size size;
    unsigned int display;
    unsigned int fit;

};

void widget_setattribute(struct widget *widget, unsigned int attribute, char *value);
void widget_unsetattributes(struct widget *widget);
unsigned int widget_setstate(struct widget *widget, unsigned int state);
void widget_getclipping(struct widget *widget, struct util_position *position, struct util_size *size);
unsigned int widget_intersectsx(struct widget *widget, int x);
unsigned int widget_intersectsy(struct widget *widget, int y);
unsigned int widget_intersects(struct widget *widget, int x, int y);
unsigned int widget_isdragable(struct widget *widget);
unsigned int widget_isinteractive(struct widget *widget);
unsigned int widget_isscrollable(struct widget *widget);
void widget_init(struct widget *widget, unsigned int source, unsigned int type, char *id, char *in, void *data);
