struct glyph
{

    struct renderable base;
    unsigned int padding;
    unsigned char *data;
    unsigned int color;

};

void glyph_assign(struct glyph *glyph, void *data, unsigned short encoding);
void glyph_init(struct glyph *glyph, unsigned int color);
