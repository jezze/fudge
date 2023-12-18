struct cache_row
{

    int num;
    int rx;
    int ry;
    unsigned int istart;
    unsigned int iend;
    unsigned int length;
    struct text_font *font;
    struct widget *widget;

};

struct cache_text
{

    unsigned int rows;
    int lastrowx;
    int lastrowy;

};

struct cache_image
{

    unsigned int loaded;
    struct util_size size;

};

void cache_initrow(struct cache_row *cacherow, struct text_rowinfo *rowinfo, unsigned int num, struct text_font *font, unsigned int paddingx, unsigned int paddingy, unsigned int halign, unsigned int valign, int w, int h, int offx, int offy);
void cache_inittext(struct cache_text *cachetext, unsigned int rows, int lastrowx, int lastrowy);
