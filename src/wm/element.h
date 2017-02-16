#define ELEMENT_TYPE_FILL               0
#define ELEMENT_TYPE_WINDOW             1
#define ELEMENT_TYPE_TEXT               2
#define ELEMENT_TYPE_PANEL              3
#define ELEMENT_TYPE_MOUSE              4
#define ELEMENT_DAMAGE_NONE             0
#define ELEMENT_DAMAGE_UPDATE           1
#define ELEMENT_DAMAGE_REMOVE           2
#define ELEMENT_MOUSETYPE_DEFAULT       0
#define ELEMENT_TEXTTYPE_NORMAL         0
#define ELEMENT_TEXTTYPE_HIGHLIGHT      1
#define ELEMENT_TEXTFLOW_NORMAL         0
#define ELEMENT_TEXTFLOW_BOTTOM         1
#define ELEMENT_TEXTFLOW_INPUT          2

struct element
{

    unsigned int id;
    unsigned int type;
    unsigned int source;
    unsigned int z;
    unsigned int damage;
    unsigned int count;
    struct box size;

};

struct element_fill
{

    struct element element;
    unsigned int color;

};

struct element_mouse
{

    struct element element;
    unsigned int type;

};

struct element_panel
{

    struct element element;
    unsigned int active;

};

struct element_text
{

    struct element element;
    unsigned int type;
    unsigned int flow;
    unsigned int cursor;

};

struct element_window
{

    struct element element;
    unsigned int active;

};

void element_init(struct element *element, unsigned int id, unsigned int type, unsigned int z);
void element_initfill(struct element_fill *fill, unsigned int color);
void element_initmouse(struct element_mouse *mouse, unsigned int type);
void element_initpanel(struct element_panel *panel, unsigned int active);
void element_inittext(struct element_text *text, unsigned int type, unsigned int flow);
void element_initwindow(struct element_window *window, unsigned int active);
void element_set(struct element *element, unsigned int source, unsigned int damage, unsigned int count);
