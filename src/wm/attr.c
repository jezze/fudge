#include <fudge.h>
#include "config.h"
#include "util.h"
#include "strpool.h"
#include "attr.h"

static struct util_token flows[5] =
{
    {ATTR_FLOW_DEFAULT, "default"},
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

static struct util_token mimetypes[2] =
{
    {ATTR_MIMETYPE_FUDGEMOUSE, "image/fudge-icon-mouse"},
    {ATTR_MIMETYPE_PCX, "image/pcx"}
};

static struct util_token modes[2] =
{
    {ATTR_MODE_NORMAL, "normal"},
    {ATTR_MODE_READONLY, "readonly"}
};

static struct util_token overflows[4] =
{
    {ATTR_OVERFLOW_NONE, "none"},
    {ATTR_OVERFLOW_SCROLL, "scroll"},
    {ATTR_OVERFLOW_HSCROLL, "hscroll"},
    {ATTR_OVERFLOW_VSCROLL, "vscroll"}
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

unsigned int attr_isvalue(unsigned int attribute)
{

    switch (attribute)
    {

    case ATTR_COLOR:
    case ATTR_COLUMNS:
    case ATTR_CURSOR:
    case ATTR_PADDING:
    case ATTR_SPAN:
        return 1;

    }

    return 0;

}

unsigned int attr_isstring(unsigned int attribute)
{

    switch (attribute)
    {

    case ATTR_CONTENT:
    case ATTR_ID:
    case ATTR_IN:
    case ATTR_LABEL:
    case ATTR_ONCLICK:
    case ATTR_SOURCE:
    case ATTR_TITLE:
        return 1;

    }

    return 0;

}

unsigned int attr_isenum(unsigned int attribute)
{

    switch (attribute)
    {

    case ATTR_FLOW:
    case ATTR_HALIGN:
    case ATTR_MIMETYPE:
    case ATTR_MODE:
    case ATTR_OVERFLOW:
    case ATTR_VALIGN:
    case ATTR_WEIGHT:
    case ATTR_WRAP:
        return 1;

    }

    return 0;

}

unsigned int attr_update(unsigned int attribute, char *value, unsigned int current)
{

    switch (attribute)
    {

    case ATTR_COLOR:
        return cstring_read_value(value, cstring_length(value), 16);

    case ATTR_COLUMNS:
        return cstring_read_value(value, cstring_length(value), 10);

    case ATTR_CONTENT:
        return strpool_updatestring(current, value);

    case ATTR_CURSOR:
        return cstring_read_value(value, cstring_length(value), 10);

    case ATTR_FLOW:
        return util_getkey(flows, 5, value);

    case ATTR_HALIGN:
        return util_getkey(haligns, 3, value);

    case ATTR_ID:
        return strpool_updatestring(current, value);

    case ATTR_IN:
        return strpool_updatestring(current, value);

    case ATTR_LABEL:
        return strpool_updatestring(current, value);

    case ATTR_MIMETYPE:
        return util_getkey(mimetypes, 2, value);

    case ATTR_MODE:
        return util_getkey(modes, 2, value);

    case ATTR_ONCLICK:
        return strpool_updatestring(current, value);

    case ATTR_OVERFLOW:
        return util_getkey(overflows, 4, value);

    case ATTR_PADDING:
        return cstring_read_value(value, cstring_length(value), 10);

    case ATTR_SOURCE:
        return strpool_updatestring(current, value);

    case ATTR_SPAN:
        return cstring_read_value(value, cstring_length(value), 10);

    case ATTR_TITLE:
        return strpool_updatestring(current, value);

    case ATTR_VALIGN:
        return util_getkey(valigns, 3, value);

    case ATTR_WEIGHT:
        return util_getkey(weights, 2, value);

    case ATTR_WRAP:
        return util_getkey(wraps, 3, value);

    }

    return 0;

}

