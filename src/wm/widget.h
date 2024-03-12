#define WIDGET_TYPE_BUTTON              1
#define WIDGET_TYPE_CHOICE              2
#define WIDGET_TYPE_FILL                3
#define WIDGET_TYPE_IMAGE               4
#define WIDGET_TYPE_LAYOUT              5
#define WIDGET_TYPE_LISTBOX             6
#define WIDGET_TYPE_SELECT              7
#define WIDGET_TYPE_TEXT                8
#define WIDGET_TYPE_TEXTBOX             9
#define WIDGET_TYPE_TEXTBUTTON          10
#define WIDGET_TYPE_WINDOW              11
#define WIDGET_STATE_NORMAL             0
#define WIDGET_STATE_HOVER              1
#define WIDGET_STATE_HOVEROFF           2
#define WIDGET_STATE_FOCUS              3
#define WIDGET_STATE_FOCUSOFF           4
#define WIDGET_STATE_DESTROYED          5

struct widget_button
{

    unsigned int label;
    unsigned int onclick;

};

struct widget_choice
{

    unsigned int label;
    unsigned int onclick;

};

struct widget_layout
{

    unsigned int flow;
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
    unsigned int loaded;
    struct util_size size;

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

};

struct widget_text
{

    unsigned int content;
    unsigned int halign;
    unsigned int valign;
    unsigned int weight;
    unsigned int wrap;
    unsigned int offx;
    unsigned int markstart;
    unsigned int markend;
    unsigned int rows;
    int lastrowx;
    int lastrowy;

};

struct widget_textbox
{

    unsigned int mode;
    unsigned int overflow;
    int hscroll;
    int vscroll;
    unsigned int enablecursor;
    unsigned int cursor;
    int cursorx;
    int cursory;
    unsigned int cursorheight;
    unsigned int cursorwidth;

};

struct widget_textbutton
{

    unsigned int label;
    unsigned int onclick;

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
    unsigned int span;
    void *data;
    struct util_box bb;
    struct util_box clip;

};

void widget_setattribute(struct widget *widget, unsigned int attribute, char *value);
void widget_unsetattributes(struct widget *widget);
unsigned int widget_setstate(struct widget *widget, unsigned int state);
unsigned int widget_intersectsx(struct widget *widget, int x);
unsigned int widget_intersectsy(struct widget *widget, int y);
unsigned int widget_intersects(struct widget *widget, int x, int y);
unsigned int widget_isdragable(struct widget *widget);
unsigned int widget_isresizable(struct widget *widget);
unsigned int widget_isinteractive(struct widget *widget);
unsigned int widget_isscrollable(struct widget *widget);
void widget_init(struct widget *widget, unsigned int source, unsigned int type, char *id, char *in, void *data);
