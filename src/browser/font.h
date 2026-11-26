#define FONT_FACES                      4
#define FONT_MONO                       2
#define FONT_BOLD                       1
#define FONT_UPPERCASE                  1
#define FONT_LOWERCASE                  2
#define FONT_LARGEST                    96
#define FONT_ENTRIES                    4096
#define FONT_POOL                       0x100000
#define FONT_DATA                       0x80000

struct font_glyph
{

    unsigned int key;
    unsigned int id;
    int advance;
    int left;
    int top;
    unsigned int width;
    unsigned int height;
    unsigned int offset;
    unsigned int rendered;

};

struct font_face
{

    struct ttf_font ttf;

};

struct font_choice
{

    unsigned int face;
    unsigned int size;

};

/* advances are 1/64 pixel so text measured in pieces adds up the same as when it is drawn */
struct font_choice font_choose(unsigned int size, unsigned int bold, unsigned int mono);
int font_getascent(struct font_face *faces, struct font_choice choice);
int font_getdescent(struct font_face *faces, struct font_choice choice);
int font_getlineheight(struct font_face *faces, struct font_choice choice);
int font_getaveragewidth(struct font_face *faces, struct font_choice choice);
int font_getboxwidth(struct font_face *faces, struct font_choice choice);
struct font_glyph *font_getglyph(struct font_face *faces, struct font_choice choice, unsigned int codepoint);
int font_getkerning(struct font_face *faces, struct font_choice choice, unsigned int left, unsigned int right);
struct font_glyph *font_render(struct font_face *faces, struct font_choice choice, unsigned int codepoint);
unsigned char *font_getcoverage(struct font_glyph *glyph);
unsigned int font_setcase(unsigned int codepoint, unsigned int casing);
unsigned int font_load(struct font_face *face, char *data, unsigned int size);
