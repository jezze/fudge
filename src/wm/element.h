#define ELEMENT_TYPE_WINDOW             1
#define ELEMENT_TYPE_TEXT               2
#define ELEMENT_TYPE_PANEL              3
#define ELEMENT_TYPE_MOUSE              4
#define ELEMENT_TEXTTYPE_NORMAL         0x00
#define ELEMENT_TEXTTYPE_HIGHLIGHT      0x01

struct element
{

    struct box size;
    unsigned int id;
    unsigned int type;
    unsigned int source;
    unsigned int z;
    unsigned int count;

};

struct element_mouse
{

    struct element base;

};

struct element_panel
{

    struct element base;
    unsigned int active;

};

struct element_text
{

    struct element base;
    unsigned int type;

};

struct element_window
{

    struct element base;
    unsigned int active;

};

void element_init(struct element *element, unsigned int type, unsigned int count);
void element_initmouse(struct element_mouse *mouse);
void element_initpanel(struct element_panel *panel);
void element_inittext(struct element_text *text, unsigned int type);
void element_initwindow(struct element_window *window);
