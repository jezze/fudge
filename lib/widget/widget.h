#define WIDGET_TYPE_NULL                0
#define WIDGET_TYPE_FILL                1
#define WIDGET_TYPE_WINDOW              2
#define WIDGET_TYPE_TEXTBOX             3
#define WIDGET_TYPE_PANEL               4
#define WIDGET_TYPE_MOUSE               5
#define WIDGET_DAMAGE_NONE              0
#define WIDGET_DAMAGE_REPLACE           1
#define WIDGET_DAMAGE_REMOVE            2
#define WIDGET_MOUSETYPE_DEFAULT        0
#define WIDGET_TEXTTYPE_NORMAL          0
#define WIDGET_TEXTTYPE_HIGHLIGHT       1
#define WIDGET_Z_BOTTOM                 0
#define WIDGET_Z_TOP                    1

struct widget
{

    unsigned int id;
    unsigned int type;
    unsigned int source;
    unsigned int z;
    unsigned int damage;
    unsigned int count;

};

struct widget_fill
{

    struct box size;
    unsigned int color;

};

struct widget_mouse
{

    struct box size;
    unsigned int type;

};

struct widget_panel
{

    struct box size;
    unsigned int active;
    unsigned int length;

};

struct widget_textbox
{

    struct box size;
    unsigned int length;
    unsigned int cursor;
    unsigned int offset;
    int scroll;

};

struct widget_window
{

    struct box size;
    unsigned int active;

};

void widget_initfill(struct widget_fill *fill, unsigned int color);
void widget_initmouse(struct widget_mouse *mouse, unsigned int type);
void widget_initpanel(struct widget_panel *panel, unsigned int active);
void widget_inittextbox(struct widget_textbox *textbox);
void widget_initwindow(struct widget_window *window, unsigned int active);
void widget_update(struct ring *ring, void *item, unsigned int z, unsigned int type, unsigned int count);
void widget_remove(struct ring *ring, void *item, unsigned int z);
