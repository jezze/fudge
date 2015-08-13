struct glyph
{

    struct renderable base;
    unsigned char *data;

};

void glyph_init(struct glyph *glyph, void *data);
