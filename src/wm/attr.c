#include <fudge.h>
#include "util.h"
#include "strpool.h"
#include "attr.h"

#define NUM_DISPLAYS                    4
#define NUM_MODES                       4

static struct util_token displays[NUM_DISPLAYS] =
{
    {ATTR_DISPLAY_BLOCK, "block"},
    {ATTR_DISPLAY_INLINE, "inline"},
    {ATTR_DISPLAY_FIXED, "fixed"},
    {ATTR_DISPLAY_NONE, "none"}
};

static struct util_token flows[6] =
{
    {ATTR_FLOW_DEFAULT, "default"},
    {ATTR_FLOW_STRETCH, "stretch"},
    {ATTR_FLOW_HORIZONTAL, "horizontal"},
    {ATTR_FLOW_HORIZONTALSTRETCH, "horizontal-stretch"},
    {ATTR_FLOW_VERTICAL, "vertical"},
    {ATTR_FLOW_VERTICALSTRETCH, "vertical-stretch"}
};

static struct util_token haligns[3] =
{
    {ATTR_HALIGN_LEFT, "left"},
    {ATTR_HALIGN_CENTER, "center"},
    {ATTR_HALIGN_RIGHT, "right"}
};

static struct util_token mimetypes[1] =
{
    {ATTR_MIMETYPE_RAW, "image/raw"}
};

static struct util_token modes[NUM_MODES] =
{
    {ATTR_MODE_NORMAL, "normal"},
    {ATTR_MODE_EDIT, "edit"},
    {ATTR_MODE_LINK, "link"},
    {ATTR_MODE_DISABLED, "disabled"}
};

static struct util_token overflows[2] =
{
    {ATTR_OVERFLOW_NONE, "none"},
    {ATTR_OVERFLOW_SCROLL, "scroll"}
};

static struct util_token valigns[3] =
{
    {ATTR_VALIGN_TOP, "top"},
    {ATTR_VALIGN_MIDDLE, "middle"},
    {ATTR_VALIGN_BOTTOM, "bottom"}
};

static struct util_token weights[2] =
{
    {ATTR_WEIGHT_NORMAL, "normal"},
    {ATTR_WEIGHT_BOLD, "bold"}
};

static struct util_token wraps[3] =
{
    {ATTR_WRAP_NONE, "none"},
    {ATTR_WRAP_CHAR, "char"},
    {ATTR_WRAP_WORD, "word"}
};

static unsigned int getnum(char *value, unsigned int base)
{

    return (value) ? cstring_read_value(value, cstring_length(value), base) : 0;

}

static unsigned int getstring(unsigned int current, char *value)
{

    return strpool_updatestring(current, (value && value[0]) ? value : 0);

}

unsigned int attr_update(unsigned int attribute, char *value, unsigned int current)
{

    switch (attribute)
    {

    case ATTR_CHECKED:
        return getnum(value, 10);

    case ATTR_COLOR:
        return getnum(value, 16);

    case ATTR_DATA:
        return getstring(current, value);

    case ATTR_DISPLAY:
        return util_getkey(displays, NUM_DISPLAYS, value);

    case ATTR_FLOW:
        return util_getkey(flows, 6, value);

    case ATTR_HEIGHT:
        return getnum(value, 10);

    case ATTR_HALIGN:
        return util_getkey(haligns, 3, value);

    case ATTR_ID:
        return getstring(current, value);

    case ATTR_IN:
        return getstring(current, value);

    case ATTR_LABEL:
        return getstring(current, value);

    case ATTR_MIMETYPE:
        return util_getkey(mimetypes, 1, value);

    case ATTR_MODE:
        return util_getkey(modes, NUM_MODES, value);

    case ATTR_ONCHANGE:
        return getstring(current, value);

    case ATTR_ONCLICK:
        return getstring(current, value);

    case ATTR_ONENTER:
        return getstring(current, value);

    case ATTR_ONRESIZE:
        return getstring(current, value);

    case ATTR_ONSELECT:
        return getstring(current, value);

    case ATTR_OVERFLOW:
        return util_getkey(overflows, 2, value);

    case ATTR_PADDING:
        return getnum(value, 10);

    case ATTR_SOURCE:
        return getstring(current, value);

    case ATTR_SPACING:
        return getnum(value, 10);

    case ATTR_SPAN:
        return getnum(value, 10);

    case ATTR_VALIGN:
        return util_getkey(valigns, 3, value);

    case ATTR_WIDTH:
        return getnum(value, 10);

    case ATTR_WEIGHT:
        return util_getkey(weights, 2, value);

    case ATTR_WRAP:
        return util_getkey(wraps, 3, value);

    }

    return 0;

}

