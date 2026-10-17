#define POOL_RAWRESOURCES               8
#define POOL_WIDGETS                    1536

struct pool_rawresource
{

    unsigned int used;
    unsigned int target;
    unsigned int id;
    unsigned int *data;

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
struct pool_rawresource *pool_createraw(struct widget *widget, char *source);
struct text_font *pool_getfont(unsigned int index);
void pool_setfont(unsigned int index, unsigned int lineheight, unsigned int padding);
void pool_loadfont(unsigned int index, char *path);
void pool_loadresources(void);
void pool_setup(void);
