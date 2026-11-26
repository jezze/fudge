#include <fudge.h>
#include <font.h>
#include "font.h"

static unsigned int mapcodepoint(unsigned int codepoint)
{

    /* spaces of other widths are drawn as plain ones, a tab or newline never reaches a glyph */
    switch (codepoint)
    {

    case 0x00A0:
    case 0x2002:
    case 0x2003:
    case 0x2009:
    case 0x200A:
    case 0x202F:
        return ' ';

    }

    return codepoint;

}

static unsigned int isicon(unsigned int codepoint)
{

    return (codepoint >= 0xE000 && codepoint <= 0xF8FF) || codepoint >= 0xF0000;

}

static int scale(struct font_face *face, struct font_choice choice, int units)
{

    int value = units * (int)choice.size;
    int half = (int)face->ttf.unitsperem / 2;

    return (value >= 0) ? (value + half) / (int)face->ttf.unitsperem : -((-value + half) / (int)face->ttf.unitsperem);

}

struct font_choice font_choose(unsigned int size, unsigned int bold, unsigned int mono)
{

    struct font_choice choice;

    choice.face = ((mono) ? FONT_MONO : 0) + ((bold) ? FONT_BOLD : 0);
    choice.size = (size < 1) ? 1 : (size > FONT_LARGEST) ? FONT_LARGEST : size;

    return choice;

}

int font_getascent(struct font_set *fonts, struct font_choice choice)
{

    return scale(&fonts->faces[choice.face], choice, fonts->faces[choice.face].ttf.ascender);

}

int font_getdescent(struct font_set *fonts, struct font_choice choice)
{

    return scale(&fonts->faces[choice.face], choice, -fonts->faces[choice.face].ttf.descender);

}

int font_getlineheight(struct font_set *fonts, struct font_choice choice)
{

    return font_getascent(fonts, choice) + font_getdescent(fonts, choice) + scale(&fonts->faces[choice.face], choice, fonts->faces[choice.face].ttf.linegap);

}

int font_getaveragewidth(struct font_set *fonts, struct font_choice choice)
{

    return scale(&fonts->faces[choice.face], choice, fonts->faces[choice.face].ttf.averagewidth);

}

int font_getboxwidth(struct font_set *fonts, struct font_choice choice)
{

    return scale(&fonts->faces[choice.face], choice, fonts->faces[choice.face].ttf.boxwidth);

}

static void clearcache(struct font_set *fonts)
{

    unsigned int i;

    for (i = 0; i < FONT_ENTRIES; i++)
        fonts->entries[i].key = 0;

    fonts->entrycount = 0;
    fonts->poolcount = 0;

}

struct font_glyph *font_getglyph(struct font_set *fonts, struct font_choice choice, unsigned int codepoint)
{

    unsigned int key = ((mapcodepoint(codepoint) & 0x1FFFFF) << 11) | (choice.size << 2) | choice.face;
    unsigned int index = (key * 2654435761u) % FONT_ENTRIES;
    struct font_face *face = &fonts->faces[choice.face];
    struct font_glyph *glyph;
    unsigned int id;

    while (fonts->entries[index].key)
    {

        if (fonts->entries[index].key == key)
            return &fonts->entries[index];

        index = (index + 1) % FONT_ENTRIES;

    }

    /* a full table starts over rather than evicting, the page needs few distinct glyphs */
    if (fonts->entrycount >= FONT_ENTRIES * 3 / 4)
    {

        clearcache(fonts);

        index = (key * 2654435761u) % FONT_ENTRIES;

    }

    id = (isicon(codepoint)) ? 0 : ttf_findglyph(&face->ttf, mapcodepoint(codepoint));
    glyph = &fonts->entries[index];
    glyph->key = key;
    glyph->id = id;
    /* glyphs step whole pixels as hinted text does, which keeps every letter on the pixel grid */
    glyph->advance = (face->ttf.unitsperem) ? scale(face, choice, (int)ttf_getadvance(&face->ttf, id)) * 64 : 0;

    /* icon fonts are not loaded, their glyphs keep the square they would take up and draw nothing */
    if (isicon(codepoint))
        glyph->advance = choice.size * 64;
    glyph->rendered = 0;
    glyph->left = 0;
    glyph->top = 0;
    glyph->width = 0;
    glyph->height = 0;
    fonts->entrycount++;

    return glyph;

}

struct font_glyph *font_render(struct font_set *fonts, struct font_choice choice, unsigned int codepoint)
{

    struct font_glyph *glyph = font_getglyph(fonts, choice, codepoint);
    struct font_face *face = &fonts->faces[choice.face];
    struct ttf_box box;

    if (glyph->rendered)
        return glyph;

    if (fonts->poolcount + TTF_SIZE * TTF_SIZE > FONT_POOL)
    {

        clearcache(fonts);

        glyph = font_getglyph(fonts, choice, codepoint);

    }

    glyph->rendered = 1;
    glyph->offset = fonts->poolcount;

    if (isicon(codepoint) || !ttf_loadoutline(&face->ttf, &fonts->outline, glyph->id))
        return glyph;

    box = ttf_render(&fonts->raster, &fonts->outline, choice.size, face->ttf.unitsperem, fonts->pool + fonts->poolcount, FONT_POOL - fonts->poolcount);
    glyph->left = box.left;
    glyph->top = box.top;
    glyph->width = box.width;
    glyph->height = box.height;
    fonts->poolcount += box.width * box.height;

    return glyph;

}

int font_getkerning(struct font_set *fonts, struct font_choice choice, unsigned int left, unsigned int right)
{

    struct font_face *face = &fonts->faces[choice.face];

    if (!left || !right)
        return 0;

    return scale(face, choice, ttf_getkerning(&face->ttf, left, right)) * 64;

}

unsigned char *font_getcoverage(struct font_set *fonts, struct font_glyph *glyph)
{

    return fonts->pool + glyph->offset;

}

unsigned int font_setcase(unsigned int codepoint, unsigned int casing)
{

    unsigned int lower = (codepoint >= 'a' && codepoint <= 'z') || (codepoint >= 0xE0 && codepoint <= 0xFE && codepoint != 0xF7);
    unsigned int upper = (codepoint >= 'A' && codepoint <= 'Z') || (codepoint >= 0xC0 && codepoint <= 0xDE && codepoint != 0xD7);

    if (casing == FONT_UPPERCASE && lower)
        return codepoint - 32;

    if (casing == FONT_LOWERCASE && upper)
        return codepoint + 32;

    return codepoint;

}

unsigned int font_load(struct font_set *fonts, unsigned int face, unsigned int size)
{

    clearcache(fonts);

    return ttf_init(&fonts->faces[face].ttf, fonts->faces[face].data, size);

}

