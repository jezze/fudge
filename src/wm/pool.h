#define POOL_PCXRESOURCES               32
#define POOL_PCXROWSIZE                 2048
#define POOL_PCXOFFSETS                 1024
#define POOL_PNGRESOURCES               4
#define POOL_PNGROWSIZE                 2048
#define POOL_WIDGETS                    1536

struct pool_pcxresource
{

    unsigned int used;
    unsigned int target;
    unsigned int id;
    unsigned char colormap[768];
    unsigned int width;
    unsigned int height;
    unsigned int bpl;
    unsigned int row;
    unsigned int offset;
    unsigned int rowstep;
    unsigned int noffsets;
    unsigned int rowoffsets[POOL_PCXOFFSETS];
    unsigned char rowdata[POOL_PCXROWSIZE];

};

struct pool_pngresource
{

    struct png png;
    unsigned int used;
    unsigned int target;
    unsigned int id;
    unsigned int loaded;

};

struct list_item *pool_prev(struct list_item *current);
struct list_item *pool_next(struct list_item *current);
struct list_item *pool_nextin(struct list_item *current, struct widget *parent);
struct list_item *pool_nextsource(struct list_item *current, unsigned int source);
struct widget *pool_getwidgetbyid(unsigned int source, char *id);
void pool_setparent(struct widget *widget);
void pool_setorphans(void);
unsigned int pool_gettextlength(struct widget *widget);
struct widget *pool_gettextat(struct widget *widget, unsigned int index, unsigned int *offset);
void pool_bump(struct widget *widget);
struct widget *pool_create(unsigned int source, unsigned int type, char *id, char *in);
void pool_destroy(struct widget *widget);
struct pool_pcxresource *pool_createpcx(struct widget *widget, char *source);
unsigned char *pool_pcxreadline(struct pool_pcxresource *resource, unsigned int row);
struct pool_pngresource *pool_createpng(struct widget *widget, char *source);
unsigned int *pool_pngreadrow(struct pool_pngresource *resource, unsigned int row);
struct text_font *pool_getfont(unsigned int index);
void pool_setfont(unsigned int index, unsigned int lineheight, unsigned int padding);
void pool_loadfont(unsigned int index, char *path);
void pool_loadresources(void);
void pool_setup(void);
