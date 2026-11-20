#define FONT_GLYPHS                     256
#define FONT_ROWS                       18
#define FONT_ROWBYTES                   2
#define FONT_SIZES                      4
#define FONT_FACES                      8
#define FONT_SCALES                     3

struct font_glyph
{

    unsigned char width;
    unsigned char height;
    unsigned char rows[FONT_ROWS * FONT_ROWBYTES];

};

struct font_face
{

    unsigned int height;
    unsigned int ascent;
    struct font_glyph glyphs[FONT_GLYPHS];

};

struct font_choice
{

    unsigned int face;
    unsigned int scale;

};

struct font_glyph *font_getglyph(struct font_face *face, unsigned int codepoint);
struct font_choice font_choose(unsigned int size, unsigned int bold);
unsigned int font_load(struct font_face *face, char *data, unsigned int size);
