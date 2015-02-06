struct glyph
{

    struct box size;
    const char c;
    const unsigned char *buffer;

};

struct text
{

    struct box size;
    const char *content;

};
