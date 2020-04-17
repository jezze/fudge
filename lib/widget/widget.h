#define WIDGET_TYPE_NULL                0
#define WIDGET_TYPE_FILL                1
#define WIDGET_TYPE_WINDOW              2
#define WIDGET_TYPE_TEXT                3
#define WIDGET_TYPE_TEXTBOX             4
#define WIDGET_TYPE_PANEL               5
#define WIDGET_TYPE_MOUSE               6
#define WIDGET_DAMAGE_NONE              0
#define WIDGET_DAMAGE_REPLACE           1
#define WIDGET_DAMAGE_REMOVE            2
#define WIDGET_MOUSETYPE_DEFAULT        0
#define WIDGET_TEXTTYPE_NORMAL          0
#define WIDGET_TEXTTYPE_HIGHLIGHT       1
#define WIDGET_Z_REGULAR                0
#define WIDGET_Z_FLOAT                  1

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

struct widget_text
{

    unsigned int type;
    unsigned int length;
    unsigned int offset;

};

struct widget_textbox
{

    unsigned int length;
    unsigned int cursor;
    unsigned int offset;

};

struct widget_window
{

    struct box size;
    unsigned int active;

};

void widget_initfill(struct widget_fill *fill, unsigned int color);
void widget_initmouse(struct widget_mouse *mouse, unsigned int type);
void widget_initpanel(struct widget_panel *panel, unsigned int active);
void widget_inittext(struct widget_text *text, unsigned int type);
void widget_inittextbox(struct widget_textbox *textbox);
void widget_initwindow(struct widget_window *window, unsigned int active);
void widget_update(struct ring *ring, void *item, unsigned int z, unsigned int type, unsigned int count, struct box *size);
void widget_updateX(struct ring *ring, void *item, unsigned int z, unsigned int type, unsigned int count);
void widget_remove(struct ring *ring, void *item, unsigned int z);
