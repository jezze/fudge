#include <fudge.h>
#include <font.h>
#include "font.h"

static struct font_glyph entries[FONT_ENTRIES];
static unsigned char pool[FONT_POOL];
static unsigned int poolcount;
static unsigned int entrycount;
static struct ttf_outline outline;
static struct ttf_raster raster;

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

int font_getascent(struct font_face *faces, struct font_choice choice)
{

    return scale(&faces[choice.face], choice, faces[choice.face].ttf.ascender);

}

int font_getdescent(struct font_face *faces, struct font_choice choice)
{

    return scale(&faces[choice.face], choice, -faces[choice.face].ttf.descender);

}

int font_getlineheight(struct font_face *faces, struct font_choice choice)
{

    return font_getascent(faces, choice) + font_getdescent(faces, choice) + scale(&faces[choice.face], choice, faces[choice.face].ttf.linegap);

}

int font_getaveragewidth(struct font_face *faces, struct font_choice choice)
{

    return scale(&faces[choice.face], choice, faces[choice.face].ttf.averagewidth);

}

int font_getboxwidth(struct font_face *faces, struct font_choice choice)
{

    return scale(&faces[choice.face], choice, faces[choice.face].ttf.boxwidth);

}

static void clearcache(void)
{

    unsigned int i;

    for (i = 0; i < FONT_ENTRIES; i++)
        entries[i].key = 0;

    entrycount = 0;
    poolcount = 0;

}

struct font_glyph *font_getglyph(struct font_face *faces, struct font_choice choice, unsigned int codepoint)
{

    unsigned int key = ((mapcodepoint(codepoint) & 0x1FFFFF) << 11) | (choice.size << 2) | choice.face;
    unsigned int index = (key * 2654435761u) % FONT_ENTRIES;
    struct font_glyph *glyph;
    struct font_face *face = &faces[choice.face];
    unsigned int id;

    while (entries[index].key)
    {

        if (entries[index].key == key)
            return &entries[index];

        index = (index + 1) % FONT_ENTRIES;

    }

    /* a full table starts over rather than evicting, the page needs few distinct glyphs */
    if (entrycount >= FONT_ENTRIES * 3 / 4)
    {

        clearcache();

        index = (key * 2654435761u) % FONT_ENTRIES;

    }

    id = ttf_findglyph(&face->ttf, mapcodepoint(codepoint));
    glyph = &entries[index];
    glyph->key = key;
    glyph->id = id;
    glyph->advance = (face->ttf.unitsperem) ? (int)(ttf_getadvance(&face->ttf, id) * choice.size * 64 / face->ttf.unitsperem) : 0;
    glyph->rendered = 0;
    glyph->left = 0;
    glyph->top = 0;
    glyph->width = 0;
    glyph->height = 0;
    entrycount++;

    return glyph;

}

struct font_glyph *font_render(struct font_face *faces, struct font_choice choice, unsigned int codepoint)
{

    struct font_glyph *glyph = font_getglyph(faces, choice, codepoint);
    struct font_face *face = &faces[choice.face];
    struct ttf_box box;

    if (glyph->rendered)
        return glyph;

    if (poolcount + TTF_SIZE * TTF_SIZE > FONT_POOL)
    {

        clearcache();

        glyph = font_getglyph(faces, choice, codepoint);

    }

    glyph->rendered = 1;
    glyph->offset = poolcount;

    if (!ttf_loadoutline(&face->ttf, &outline, glyph->id))
        return glyph;

    box = ttf_render(&raster, &outline, choice.size, face->ttf.unitsperem, pool + poolcount, FONT_POOL - poolcount);
    glyph->left = box.left;
    glyph->top = box.top;
    glyph->width = box.width;
    glyph->height = box.height;
    poolcount += box.width * box.height;

    return glyph;

}

int font_getkerning(struct font_face *faces, struct font_choice choice, unsigned int left, unsigned int right)
{

    struct font_face *face = &faces[choice.face];

    if (!left || !right)
        return 0;

    return ttf_getkerning(&face->ttf, left, right) * (int)choice.size * 64 / (int)face->ttf.unitsperem;

}

unsigned char *font_getcoverage(struct font_glyph *glyph)
{

    return pool + glyph->offset;

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

unsigned int font_load(struct font_face *face, char *data, unsigned int size)
{

    clearcache();

    return ttf_init(&face->ttf, data, size);

}

