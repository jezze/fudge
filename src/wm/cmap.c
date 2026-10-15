#include <fudge.h>
#include <abi.h>
#include "cmap.h"

static char *palettes[NUM_PALETTES] = {"button", "checkbox", "box", "item", "link", "mouse", "panel", "select", "swatch", "text", "window"};
static char *shades[NUM_SHADES] = {"normal", "hover", "focus", "disabled"};
static char *parts[NUM_PARTS] = {"shadow", "body", "dark", "light", "text", "icon"};
static unsigned int values[NUM_PALETTES * NUM_SHADES * NUM_PARTS];

static unsigned int find(char **names, unsigned int count, char *name, unsigned int length)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (cstring_length(names[i]) == length && buffer_match(names[i], name, length))
            return i;

    }

    return count;

}

unsigned int *cmap_get(unsigned int palette, unsigned int shade)
{

    return &values[(palette * NUM_SHADES + shade) * NUM_PARTS];

}

void cmap_set(char *name, unsigned int value)
{

    unsigned int length = cstring_length(name);
    unsigned int p = buffer_findbyte(name, length, '-');
    unsigned int s = (p < length) ? p + 1 + buffer_findbyte(name + p + 1, length - p - 1, '-') : length;
    unsigned int palette = find(palettes, NUM_PALETTES, name, p);
    unsigned int shade = (s < length) ? find(shades, NUM_SHADES, name + p + 1, s - p - 1) : NUM_SHADES;
    unsigned int part = (s < length) ? find(parts, NUM_PARTS, name + s + 1, length - s - 1) : NUM_PARTS;

    if (palette < NUM_PALETTES && shade < NUM_SHADES && part < NUM_PARTS)
        cmap_get(palette, shade)[part] = value;

}

