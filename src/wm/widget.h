#define WIDGET_TYPE_BUTTON              1
#define WIDGET_TYPE_FILL                3
#define WIDGET_TYPE_MEDIA               4
#define WIDGET_TYPE_LAYOUT              5
#define WIDGET_TYPE_BOX                 6
#define WIDGET_TYPE_PANEL               7
#define WIDGET_TYPE_SELECT              8
#define WIDGET_TYPE_TEXT                9
#define WIDGET_TYPE_ITEM                11
#define WIDGET_TYPE_WINDOW              12
#define WIDGET_TYPE_SWATCH              16
#define WIDGET_TYPE_CYCLE               18
#define WIDGET_TYPE_TAB                 20
#define WIDGET_TYPE_EACH                21

struct widget_attributes
{

    unsigned int checked;
    unsigned int color;
    unsigned int data;
    unsigned int display;
    unsigned int extent;
    unsigned int flow;
    unsigned int halign;
    unsigned int height;
    unsigned int id;
    unsigned int in;
    unsigned int label;
    unsigned int mimetype;
    unsigned int mode;
    unsigned int onchange;
    unsigned int onclick;
    unsigned int onenter;
    unsigned int onresize;
    unsigned int onscroll;
    unsigned int onselect;
    unsigned int overflow;
    unsigned int padding;
    unsigned int source;
    unsigned int spacing;
    unsigned int span;
    unsigned int valign;
    unsigned int weight;
    unsigned int width;
    unsigned int wrap;

};

struct widget
{

    unsigned int source;
    unsigned int type;
    unsigned int destroyed;
    unsigned int raw;
    unsigned int instance;
    unsigned int rowfirst;
    unsigned int rowcount;
    unsigned int rowstride;
    unsigned int rebind;
    struct widget *parent;
    unsigned int markstart;
    unsigned int markend;
    unsigned int cursor;
    unsigned int followcursor;
    unsigned int sticky;
    unsigned int stickyindex;
    int stickyx;
    void *resource;
    unsigned int appcursor;
    unsigned int columns;
    unsigned int rows;
    unsigned int viewwidth;
    unsigned int viewheight;
    unsigned int selected;
    unsigned int anchor;
    unsigned int textrow;
    unsigned int textoffset;
    unsigned int idhash;
    unsigned int serial;
    int scrollmax;
    struct util_region thumb;
    struct util_position position;
    struct util_size size;
    struct util_position scroll;
    struct util_position cursorposition;
    struct util_size cursorsize;
    unsigned int changed;
    struct util_region drawn;
    struct util_region drawncursor;
    struct util_position drawnscroll;
    int reportedscroll;
    int drawnscrollmax;
    struct util_region placement;
    struct util_region cplacement;
    struct util_region clip;
    struct util_region cclip;
    struct util_position rowstart;
    struct util_position rowstop;
    struct widget_attributes attributes;

};

void widget_setattribute(struct widget *widget, unsigned int attribute, char *value);
void widget_unsetattributes(struct widget *widget);
unsigned int widget_intersectsy(struct widget *widget, int y);
unsigned int widget_intersects(struct widget *widget, int x, int y);
unsigned int widget_isdragable(struct widget *widget);
unsigned int widget_isresizable(struct widget *widget);
unsigned int widget_islink(struct widget *widget);
unsigned int widget_isinteractive(struct widget *widget);
unsigned int widget_istemplate(struct widget *widget);
unsigned int widget_iscopyof(struct widget *widget, char *key);
char *widget_getdata(struct widget *widget);
unsigned int widget_isshown(struct widget *widget);
unsigned int widget_ishidden(struct widget *widget);
void widget_copyattributes(struct widget *widget, struct widget *from);
unsigned int widget_iseditable(struct widget *widget);
unsigned int widget_isscrollable(struct widget *widget);
unsigned int widget_hasscrollbar(struct widget *widget);
struct util_region widget_getview(struct widget *widget);
void widget_scrollto(struct widget *widget, struct util_region *region);
void widget_init_attributes(struct widget_attributes *attributes, unsigned type);
void widget_init(struct widget *widget, unsigned int source, unsigned int type);
