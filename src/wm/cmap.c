#include <fudge.h>
#include <abi.h>
#include "cmap.h"

#define NUM_COLORS                      103

/* frames read four colors in a row: shadow, body, dark and light */
static char *names[NUM_COLORS] = {
    "button-shadow",
    "button-body",
    "button-dark",
    "button-light",
    "button-hover-shadow",
    "button-hover-body",
    "button-hover-dark",
    "button-hover-light",
    "button-focus-shadow",
    "button-focus-body",
    "button-focus-dark",
    "button-focus-light",
    "button-label",
    "checkbox-shadow",
    "checkbox-body",
    "checkbox-dark",
    "checkbox-light",
    "checkbox-hover-shadow",
    "checkbox-hover-body",
    "checkbox-hover-dark",
    "checkbox-hover-light",
    "checkbox-focus-shadow",
    "checkbox-focus-body",
    "checkbox-focus-dark",
    "checkbox-focus-light",
    "checkbox-icon",
    "frame-shadow",
    "frame-body",
    "frame-dark",
    "frame-light",
    "frame-title",
    "item-shadow",
    "item-body",
    "item-dark",
    "item-light",
    "item-hover-shadow",
    "item-hover-body",
    "item-hover-dark",
    "item-hover-light",
    "item-focus-shadow",
    "item-focus-body",
    "item-focus-dark",
    "item-focus-light",
    "link",
    "listbox-shadow",
    "listbox-body",
    "listbox-dark",
    "listbox-light",
    "listbox-hover-shadow",
    "listbox-hover-body",
    "listbox-hover-dark",
    "listbox-hover-light",
    "listbox-focus-shadow",
    "listbox-focus-body",
    "listbox-focus-dark",
    "listbox-focus-light",
    "mouse-shadow",
    "mouse-body",
    "mouse-dark",
    "mouse-light",
    "panel-shadow",
    "panel-body",
    "panel-dark",
    "panel-light",
    "select-shadow",
    "select-body",
    "select-dark",
    "select-light",
    "select-hover-shadow",
    "select-hover-body",
    "select-hover-dark",
    "select-hover-light",
    "select-focus-shadow",
    "select-focus-body",
    "select-focus-dark",
    "select-focus-light",
    "select-icon",
    "select-label",
    "swatch-border",
    "text",
    "textbox-shadow",
    "textbox-body",
    "textbox-dark",
    "textbox-light",
    "textbox-hover-shadow",
    "textbox-hover-body",
    "textbox-hover-dark",
    "textbox-hover-light",
    "textbox-focus-shadow",
    "textbox-focus-body",
    "textbox-focus-dark",
    "textbox-focus-light",
    "textbox-readonly-shadow",
    "textbox-readonly-body",
    "textbox-readonly-dark",
    "textbox-readonly-light",
    "window-shadow",
    "window-body",
    "window-dark",
    "window-light",
    "window-icon",
    "window-icon-hover",
    "window-title"
};

static unsigned int values[NUM_COLORS];
static unsigned int missing[4];

static unsigned int find(char *name)
{

    unsigned int i;

    for (i = 0; i < NUM_COLORS; i++)
    {

        if (cstring_match(names[i], name))
            return i;

    }

    return NUM_COLORS;

}

unsigned int *cmap_get(char *name)
{

    unsigned int i = find(name);

    return (i < NUM_COLORS) ? &values[i] : missing;

}

void cmap_set(char *name, unsigned int value)
{

    unsigned int i = find(name);

    if (i < NUM_COLORS)
        values[i] = value;

}
