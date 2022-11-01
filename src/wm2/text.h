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

    unsigned int chars;
    unsigned int width;
    unsigned int height;
    unsigned int lineheight;
    unsigned int newline;

};

struct text_info
{

    unsigned int rows;
    unsigned int width;
    unsigned int height;
    unsigned int lineheight;
    struct text_rowinfo last;

};

int text_getrowx(struct text_rowinfo *rowinfo, unsigned int halign, int x, int w);
int text_getrowy(struct text_rowinfo *rowinfo, unsigned int valign, int y, int h);
unsigned int text_getrowstart(struct text_font *font, char *text, unsigned int length, unsigned int rownum, unsigned int wrap, unsigned int maxw, int crow, int offset);
unsigned int text_getrowinfo(struct text_font *font, char *text, unsigned int length, struct text_rowinfo *rowinfo, unsigned int wrap, unsigned int maxw, unsigned int offset);
unsigned int text_gettextinfo(struct text_font *font, char *text, unsigned int length, struct text_info *textinfo, unsigned int wrap, unsigned int offw, unsigned int maxw);
