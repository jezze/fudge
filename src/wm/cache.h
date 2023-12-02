struct cache_row
{

    int rx;
    int ry;
    unsigned int istart;
    unsigned int length;
    struct text_font *font;

};

struct cache_text
{

    unsigned int rows;
    int lastrowx;
    int lastrowy;
    unsigned int rownum;
    unsigned int icurrent;
    unsigned int exist;
    unsigned int offx;
    unsigned int markstart;
    unsigned int marklength;

};

struct cache_image
{

    unsigned int loaded;
    struct util_size size;

};

void cache_updatetext(struct cache_text *cachetext, struct text_font *font, unsigned int rownum, char *content, unsigned int length, unsigned int wrap, int w, int h, int offx);
void cache_initrow(struct cache_row *cacherow, struct text_rowinfo *rowinfo, struct text_font *font, unsigned int paddingx, unsigned int paddingy, unsigned int halign, unsigned int valign, int w, int h, int offx, int offy);
void cache_inittext(struct cache_text *cachetext, unsigned int rows, int lastrowx, int lastrowy);
