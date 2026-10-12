#include <fudge.h>
#include <abi.h>
#include "cmap.h"

#define NUM_COLORS                      97

struct color
{

    char *name;
    unsigned int value;

};

/* frames read four colors in a row: shadow, body, dark and light */
static struct color colors[NUM_COLORS] = {
    {"button-shadow",           0xE8101010},
    {"button-body",             0xE8484848},
    {"button-dark",             0xE8404040},
    {"button-light",            0xE8888888},
    {"button-hover-shadow",     0xE8101010},
    {"button-hover-body",       0xE8505050},
    {"button-hover-dark",       0xE8484848},
    {"button-hover-light",      0xE8888888},
    {"button-focus-shadow",     0xE8101010},
    {"button-focus-body",       0xE8585858},
    {"button-focus-dark",       0xE8505050},
    {"button-focus-light",      0xE8888888},
    {"button-label",            0xE8FFFFFF},
    {"checkbox-shadow",         0xE8101010},
    {"checkbox-body",           0xE8484848},
    {"checkbox-dark",           0xE8404040},
    {"checkbox-light",          0xE8888888},
    {"checkbox-hover-shadow",   0xE8101010},
    {"checkbox-hover-body",     0xE8505050},
    {"checkbox-hover-dark",     0xE8484848},
    {"checkbox-hover-light",    0xE8888888},
    {"checkbox-focus-shadow",   0xE8101010},
    {"checkbox-focus-body",     0xE8585858},
    {"checkbox-focus-dark",     0xE8505050},
    {"checkbox-focus-light",    0xE8888888},
    {"checkbox-icon",           0xE8FFFFFF},
    {"item-shadow",             0x00242424},
    {"item-body",               0x00242424},
    {"item-dark",               0x00242424},
    {"item-light",              0x00242424},
    {"item-hover-shadow",       0xE8242424},
    {"item-hover-body",         0xE8242424},
    {"item-hover-dark",         0xE8242424},
    {"item-hover-light",        0xE8242424},
    {"item-focus-shadow",       0xE81A1A1A},
    {"item-focus-body",         0xE81A1A1A},
    {"item-focus-dark",         0xE81A1A1A},
    {"item-focus-light",        0xE81A1A1A},
    {"link",                    0xE8C07878},
    {"listbox-shadow",          0xE8101010},
    {"listbox-body",            0xE82A2A2A},
    {"listbox-dark",            0xE8101010},
    {"listbox-light",           0xE8686868},
    {"listbox-hover-shadow",    0xE8101010},
    {"listbox-hover-body",      0xE82A2A2A},
    {"listbox-hover-dark",      0xE8101010},
    {"listbox-hover-light",     0xE8787878},
    {"listbox-focus-shadow",    0xE8101010},
    {"listbox-focus-body",      0xE82A2A2A},
    {"listbox-focus-dark",      0xE8101010},
    {"listbox-focus-light",     0xE8888888},
    {"mouse-shadow",            0xE8101010},
    {"mouse-body",              0xE8202020},
    {"mouse-dark",              0x00000000},
    {"mouse-light",             0xE8F0F0F0},
    {"panel-shadow",            0xE8101010},
    {"panel-body",              0xE8303030},
    {"panel-dark",              0xE8303030},
    {"panel-light",             0xE8585858},
    {"select-shadow",           0xE8101010},
    {"select-body",             0xE82A2A2A},
    {"select-dark",             0xE8101010},
    {"select-light",            0xE8686868},
    {"select-hover-shadow",     0xE8101010},
    {"select-hover-body",       0xE82A2A2A},
    {"select-hover-dark",       0xE8101010},
    {"select-hover-light",      0xE8787878},
    {"select-focus-shadow",     0xE8101010},
    {"select-focus-body",       0xE8805050},
    {"select-focus-dark",       0xE8805050},
    {"select-focus-light",      0xE8E0B0B0},
    {"select-icon",             0xE8FFFFFF},
    {"select-label",            0xE8FFFFFF},
    {"text",                    0xE8E0E0E0},
    {"textbox-shadow",          0xE8101010},
    {"textbox-body",            0xE82A2A2A},
    {"textbox-dark",            0xE8101010},
    {"textbox-light",           0xE8686868},
    {"textbox-hover-shadow",    0xE8101010},
    {"textbox-hover-body",      0xE82A2A2A},
    {"textbox-hover-dark",      0xE8101010},
    {"textbox-hover-light",     0xE8787878},
    {"textbox-focus-shadow",    0xE8101010},
    {"textbox-focus-body",      0xE82A2A2A},
    {"textbox-focus-dark",      0xE8101010},
    {"textbox-focus-light",     0xE8888888},
    {"textbox-readonly-shadow", 0xE8101010},
    {"textbox-readonly-body",   0xE82A2A2A},
    {"textbox-readonly-dark",   0xE8101010},
    {"textbox-readonly-light",  0xE8686868},
    {"window-shadow",           0xE8101010},
    {"window-body",             0xE8805050},
    {"window-dark",             0xE8805050},
    {"window-light",            0xE8E0B0B0},
    {"window-icon",             0xA8FFFFFF},
    {"window-icon-hover",       0xE8FFFFFF},
    {"window-title",            0xE8FFFFFF}
};

static unsigned int values[NUM_COLORS];
static unsigned int missing[4];

unsigned int *cmap_get(char *name)
{

    unsigned int i;

    for (i = 0; i < NUM_COLORS; i++)
    {

        if (cstring_match(colors[i].name, name))
            return &values[i];

    }

    return missing;

}

void cmap_init(void)
{

    unsigned int i;

    for (i = 0; i < NUM_COLORS; i++)
        values[i] = colors[i].value;

}

