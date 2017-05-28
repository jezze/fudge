#define WIDGET_TYPE_FILL                0
#define WIDGET_TYPE_WINDOW              1
#define WIDGET_TYPE_TEXT                2
#define WIDGET_TYPE_PANEL               3
#define WIDGET_TYPE_MOUSE               4
#define WIDGET_DAMAGE_NONE              0
#define WIDGET_DAMAGE_UPDATE            1
#define WIDGET_DAMAGE_REMOVE            2
#define WIDGET_MOUSETYPE_DEFAULT        0
#define WIDGET_TEXTTYPE_NORMAL          0
#define WIDGET_TEXTTYPE_HIGHLIGHT       1
#define WIDGET_TEXTFLOW_NORMAL          0
#define WIDGET_TEXTFLOW_BOTTOM          1
#define WIDGET_TEXTFLOW_INPUT           2

struct widget
{

    unsigned int id;
    unsigned int type;
    unsigned int source;
    unsigned int z;
    unsigned int damage;
    unsigned int count;
    struct box size;

};

struct widget_fill
{

    struct widget widget;
    unsigned int color;

};

struct widget_mouse
{

    struct widget widget;
    unsigned int type;

};

struct widget_panel
{

    struct widget widget;
    unsigned int active;

};

struct widget_text
{

    struct widget widget;
    unsigned int type;
    unsigned int flow;
    unsigned int cursor;

};

struct widget_window
{

    struct widget widget;
    unsigned int active;

};

void widget_init(struct widget *widget, unsigned int id, unsigned int type, unsigned int z);
void widget_initfill(struct widget_fill *fill, unsigned int color);
void widget_initmouse(struct widget_mouse *mouse, unsigned int type);
void widget_initpanel(struct widget_panel *panel, unsigned int active);
void widget_inittext(struct widget_text *text, unsigned int type, unsigned int flow);
void widget_initwindow(struct widget_window *window, unsigned int active);
void widget_set(struct widget *widget, unsigned int source, unsigned int damage, unsigned int count);
