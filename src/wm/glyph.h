struct glyph
{

    struct renderable base;
    unsigned char *data;
    char value;

};

void glyph_init(struct glyph *glyph, void *data);
