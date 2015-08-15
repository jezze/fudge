struct glyph
{

    struct renderable base;
    unsigned char *data;
    char value;
    unsigned int color;

};

void glyph_init(struct glyph *glyph, void *data);
