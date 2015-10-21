#define RENDERABLE_TYPE_WINDOW          1
#define RENDERABLE_TYPE_TEXT            2
#define RENDERABLE_TYPE_PANEL           3
#define RENDERABLE_TYPE_MOUSE           4
#define RENDERABLE_TEXTTYPE_NORMAL      0x00
#define RENDERABLE_TEXTTYPE_HIGHLIGHT   0x01

struct renderable
{

    struct box size;
    unsigned int id;
    unsigned int type;
    unsigned int source;
    unsigned int z;
    unsigned int count;

};

struct renderable_mouse
{

    struct renderable base;

};

struct renderable_panelheader
{

    unsigned int active;

};

struct renderable_panel
{

    struct renderable base;
    struct renderable_panelheader header;

};

struct renderable_textheader
{

    unsigned int type;

};

struct renderable_text
{

    struct renderable base;
    struct renderable_textheader header;

};

struct renderable_windowheader
{

    unsigned int active;

};

struct renderable_window
{

    struct renderable base;
    struct renderable_windowheader header;

};

void renderable_init(struct renderable *renderable, unsigned int type, unsigned int count);
void renderable_initmouse(struct renderable_mouse *mouse);
void renderable_initpanel(struct renderable_panel *panel);
void renderable_inittext(struct renderable_text *text, unsigned int type);
void renderable_initwindow(struct renderable_window *window);
