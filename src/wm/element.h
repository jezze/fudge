#define ELEMENT_TYPE_FILL               1
#define ELEMENT_TYPE_WINDOW             2
#define ELEMENT_TYPE_TEXT               3
#define ELEMENT_TYPE_PANEL              4
#define ELEMENT_TYPE_MOUSE              5
#define ELEMENT_TEXTTYPE_NORMAL         0x00
#define ELEMENT_TEXTTYPE_HIGHLIGHT      0x01
#define ELEMENT_TEXTFLOW_NORMAL         0x00
#define ELEMENT_TEXTFLOW_INPUT          0x01

struct element
{

    unsigned int id;
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

};

struct element_window
{

    struct box size;
    unsigned int active;

};

void element_init(struct element *element, unsigned int id, unsigned int type, unsigned int source, unsigned int z, unsigned int count);
void element_initfill(struct element_fill *fill, unsigned int color);
void element_initmouse(struct element_mouse *mouse, unsigned int x, unsigned int y);
void element_initpanel(struct element_panel *panel, unsigned int active);
void element_inittext(struct element_text *text, unsigned int type, unsigned int flow);
void element_initwindow(struct element_window *window, unsigned int active);
