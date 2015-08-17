struct text
{

    struct renderable base;
    struct glyph glyph;
    void *data;
    unsigned int count;
    char *string;

};

void text_assign(struct text *text, void *data, unsigned int count, char *string);
void text_init(struct text *text, unsigned int color);
