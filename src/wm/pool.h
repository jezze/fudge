struct pool_pcxresource
{

    unsigned char data[0x10000];
    unsigned char colormap[768];
    unsigned int width;
    unsigned int height;
    unsigned int lastoffset;
    unsigned int lastline;
    unsigned int cached;

};

struct list_item *pool_prev(struct list_item *current);
struct list_item *pool_next(struct list_item *current);
struct list_item *pool_nextin(struct list_item *current, struct widget *parent);
struct list_item *pool_nextsource(struct list_item *current, unsigned int source);
struct widget *pool_getwidgetbyid(unsigned int source, char *id);
void pool_bump(struct widget *widget);
struct widget *pool_create(unsigned int source, unsigned int type, char *id, char *in);
void pool_destroy(struct widget *widget);
void pool_pcxload(struct pool_pcxresource *pcxresource, char *source);
void pool_pcxreadline(struct pool_pcxresource *pcxresource, int line, int y, unsigned char *buffer);
struct text_font *pool_getfont(unsigned int index);
void pool_setfont(unsigned int index, void *data, unsigned int lineheight, unsigned int padding);
void pool_loadfont(unsigned int factor);
void pool_setup(void);
