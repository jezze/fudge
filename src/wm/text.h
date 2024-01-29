struct text_font
{

    unsigned char *data;
    unsigned char *bitmapdata;
    unsigned int bitmapalign;
    unsigned int lineheight;
    unsigned int padding;

};

struct text_rowinfo
{

    unsigned int length;
    unsigned int width;
    unsigned int height;
    unsigned int lineheight;
    unsigned int linebreak;
    unsigned int istart;
    unsigned int iend;

};

struct text_info
{

    unsigned int rows;
    unsigned int width;
    unsigned int height;
    int lastrowx;
    int lastrowy;

};

unsigned int text_getrowinfo(struct text_rowinfo *rowinfo, struct text_font *font, char *text, unsigned int length, unsigned int wrap, unsigned int maxw, unsigned int offset);
unsigned int text_gettextinfo(struct text_info *textinfo, struct text_font *font, char *text, unsigned int length, unsigned int wrap, unsigned int maxw, int firstrowx);
unsigned int text_getrowstart(struct text_font *font, char *text, unsigned int length, unsigned int rownum, unsigned int wrap, unsigned int maxw, int firstrowx);
unsigned int text_getoffsetat(struct text_font *font, char *text, unsigned int length, unsigned int wrap, unsigned int maxw, int firstrowx, int x, int y);
int text_getrowx(struct text_rowinfo *rowinfo, unsigned int halign, int x, int w);
int text_getrowy(struct text_rowinfo *rowinfo, unsigned int valign, int y, int h);
