struct render_rowinfo
{

    unsigned int chars;
    unsigned int width;
    unsigned int height;
    unsigned int lineheight;
    unsigned int newline;

};

struct render_textinfo
{

    unsigned int rows;
    unsigned int width;
    unsigned int height;
    unsigned int lineheight;
    struct render_rowinfo last;

};

int text_getrowx(struct render_rowinfo *rowinfo, unsigned int halign, int x, int w);
int text_getrowy(struct render_rowinfo *rowinfo, unsigned int valign, int y, int h);
unsigned int text_getrowstart(struct blit_font *font, char *text, unsigned int length, unsigned int rownum, unsigned int wrap, unsigned int maxw);
unsigned int text_getrowinfo(struct blit_font *font, char *text, unsigned int length, struct render_rowinfo *rowinfo, unsigned int wrap, unsigned int maxw, unsigned int offset);
unsigned int text_gettextinfo(struct blit_font *font, char *text, unsigned int length, struct render_textinfo *textinfo, unsigned int wrap, unsigned int offw, unsigned int maxw);
