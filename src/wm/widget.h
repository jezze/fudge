#define WIDGET_TYPE_BUTTON              1
#define WIDGET_TYPE_CHOICE              2
#define WIDGET_TYPE_FILL                3
#define WIDGET_TYPE_IMAGE               4
#define WIDGET_TYPE_LAYOUT              5
#define WIDGET_TYPE_LISTBOX             6
#define WIDGET_TYPE_PANEL               7
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

struct widget_attributes
{

    unsigned int color;
    unsigned int cursor;
    unsigned int display;
    unsigned int flow;
    unsigned int halign;
    unsigned int id;
    unsigned int in;
    unsigned int label;
    unsigned int mimetype;
    unsigned int mode;
    unsigned int onclick;
    unsigned int overflow;
    unsigned int padding;
    unsigned int source;
    unsigned int span;
    unsigned int valign;
    unsigned int weight;
    unsigned int wrap;

};

struct widget
{

    unsigned int source;
    unsigned int type;
    unsigned int state;
    unsigned int markstart;
    unsigned int markend;
    unsigned int enablecursor;
    void *resource;
    unsigned int loaded;
    struct util_position position;
    struct util_size size;
    struct util_position scroll;
    struct util_position cursorposition;
    struct util_size cursorsize;
    struct util_region placement;
    struct util_region clip;
    struct util_position rowstart;
    struct util_position rowstop;
    struct widget_attributes attributes;

};

void widget_setattribute(struct widget *widget, unsigned int attribute, char *value);
void widget_unsetattributes(struct widget *widget);
unsigned int widget_setstate(struct widget *widget, unsigned int state);
unsigned int widget_region_intersectsx(struct widget *widget, int x);
unsigned int widget_region_intersectsy(struct widget *widget, int y);
unsigned int widget_region_intersects(struct widget *widget, int x, int y);
unsigned int widget_clip_intersectsx(struct widget *widget, int x);
unsigned int widget_clip_intersectsy(struct widget *widget, int y);
unsigned int widget_clip_intersects(struct widget *widget, int x, int y);
unsigned int widget_intersectsx(struct widget *widget, int x);
unsigned int widget_intersectsy(struct widget *widget, int y);
unsigned int widget_intersects(struct widget *widget, int x, int y);
unsigned int widget_isdragable(struct widget *widget);
unsigned int widget_isresizable(struct widget *widget);
unsigned int widget_isinteractive(struct widget *widget);
unsigned int widget_isscrollable(struct widget *widget);
void widget_init_attributes(struct widget_attributes *attributes, unsigned type);
void widget_init(struct widget *widget, unsigned int source, unsigned int type);
