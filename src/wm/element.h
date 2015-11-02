#define ELEMENT_TYPE_WINDOW             1
#define ELEMENT_TYPE_TEXT               2
#define ELEMENT_TYPE_PANEL              3
#define ELEMENT_TYPE_MOUSE              4
#define ELEMENT_TEXTTYPE_NORMAL         0x00
#define ELEMENT_TEXTTYPE_HIGHLIGHT      0x01

struct element
{

    unsigned int id;
    unsigned int type;
    unsigned int source;
    unsigned int z;
    unsigned int damaged;
    unsigned int count;

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

};

struct element_window
{

    struct box size;
    unsigned int active;

};

void element_init(struct element *element, unsigned int id, unsigned int type, unsigned int source, unsigned int z, unsigned int count);
void element_initmouse(struct element_mouse *mouse);
void element_initpanel(struct element_panel *panel);
void element_inittext(struct element_text *text, unsigned int type);
void element_initwindow(struct element_window *window);
