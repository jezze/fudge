#define TTF_POINTS                      2048
#define TTF_CONTOURS                    256
#define TTF_DEPTH                       4
#define TTF_SIZE                        128
#define TTF_CELLS                       (TTF_SIZE * TTF_SIZE + 2)

struct ttf_font
{

    unsigned char *data;
    unsigned int size;
    unsigned int cmap;
    unsigned int cmapformat;
    unsigned int loca;
    unsigned int glyf;
    unsigned int hmtx;
    unsigned int kern;
    unsigned int kernpairs;
    unsigned int longloca;
    unsigned int glyphs;
    unsigned int hmetrics;
    unsigned int unitsperem;
    int ascender;
    int descender;
    int linegap;
    int averagewidth;
    int boxwidth;

};

struct ttf_point
{

    int x;
    int y;
    unsigned int on;

};

struct ttf_outline
{

    struct ttf_point points[TTF_POINTS];
    unsigned int ends[TTF_CONTOURS];
    unsigned int pointcount;
    unsigned int contourcount;

};

struct ttf_box
{

    int left;
    int top;
    unsigned int width;
    unsigned int height;

};

/* coordinates while rasterizing are 1/256 pixel, the glyph box is at most TTF_SIZE pixels each way */
struct ttf_raster
{

    int cells[TTF_CELLS];
    unsigned int width;
    unsigned int height;

};

unsigned int ttf_init(struct ttf_font *font, char *data, unsigned int size);
unsigned int ttf_findglyph(struct ttf_font *font, unsigned int codepoint);
unsigned int ttf_getadvance(struct ttf_font *font, unsigned int glyph);
int ttf_getkerning(struct ttf_font *font, unsigned int left, unsigned int right);
unsigned int ttf_loadoutline(struct ttf_font *font, struct ttf_outline *outline, unsigned int glyph);
struct ttf_box ttf_render(struct ttf_raster *raster, struct ttf_outline *outline, unsigned int size, unsigned int unitsperem, unsigned char *coverage, unsigned int limit);
