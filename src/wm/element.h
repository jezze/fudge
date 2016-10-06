#define ELEMENT_FUNC_INSERT             0
#define ELEMENT_FUNC_UPDATE             1
#define ELEMENT_FUNC_REMOVE             2
#define ELEMENT_TYPE_FILL               0
#define ELEMENT_TYPE_WINDOW             1
#define ELEMENT_TYPE_TEXT               2
#define ELEMENT_TYPE_PANEL              3
#define ELEMENT_TYPE_MOUSE              4
#define ELEMENT_TEXTTYPE_NORMAL         0
#define ELEMENT_TEXTTYPE_HIGHLIGHT      1
#define ELEMENT_TEXTFLOW_NORMAL         0
#define ELEMENT_TEXTFLOW_BOTTOM         1
#define ELEMENT_TEXTFLOW_INPUT          2

struct element
{

    unsigned int id;
    unsigned int func;
    unsigned int type;
    unsigned int source;
    unsigned int z;
    unsigned int damaged;
    unsigned int count;

};

struct element_fill
{

    struct box size;
    unsigned int color;

};

struct element_mouse
{

    unsigned int x;
    unsigned int y;

};

struct element_panel
{

    struct box size;
    unsigned int active;

};

struct element_text
{

    struct box size;
    unsigned int type;
    unsigned int flow;
    unsigned int cursor;

};

struct element_window
{

    struct box size;
    unsigned int active;

};

void element_init(struct element *element, unsigned int id, unsigned int func, unsigned int type, unsigned int source, unsigned int z, unsigned int count);
void element_initfill(struct element_fill *fill, unsigned int color);
void element_initmouse(struct element_mouse *mouse, unsigned int x, unsigned int y);
void element_initpanel(struct element_panel *panel, unsigned int active);
void element_inittext(struct element_text *text, unsigned int type, unsigned int flow);
void element_initwindow(struct element_window *window, unsigned int active);
