#define TEXT_HALIGN_LEFT                0
#define TEXT_HALIGN_CENTER              1
#define TEXT_HALIGN_RIGHT               2
#define TEXT_MODE_NORMAL                0
#define TEXT_MODE_INVERTED              1
#define TEXT_WEIGHT_NORMAL              0
#define TEXT_WEIGHT_BOLD                1
#define TEXT_VALIGN_TOP                 0
#define TEXT_VALIGN_MIDDLE              1
#define TEXT_VALIGN_BOTTOM              2
#define TEXT_WRAP_NONE                  0
#define TEXT_WRAP_CHAR                  1
#define TEXT_WRAP_WORD                  2

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
    unsigned int lastrowx;
    unsigned int lastrowy;

};

int text_getrowx(struct text_rowinfo *rowinfo, unsigned int halign, int x, int w);
int text_getrowy(struct text_rowinfo *rowinfo, unsigned int valign, int y, int h);
unsigned int text_getrowstart(struct text_font *font, char *text, unsigned int length, unsigned int rownum, unsigned int wrap, unsigned int maxw, unsigned int firstrowx);
unsigned int text_getrowinfo(struct text_rowinfo *rowinfo, struct text_font *font, char *text, unsigned int length, unsigned int wrap, unsigned int maxw, unsigned int offset);
unsigned int text_gettextinfo(struct text_info *textinfo, struct text_font *font, char *text, unsigned int length, unsigned int wrap, unsigned int offw, unsigned int maxw);
