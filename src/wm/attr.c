#include <fudge.h>
#include "config.h"
#include "util.h"
#include "pool2.h"
#include "attr.h"

static struct util_token blits[2] =
{
    {ATTR_BLIT_NORMAL, "normal"},
    {ATTR_BLIT_INVERTED, "inverted"}
};

static struct util_token fits[2] =
{
    {ATTR_FIT_NORMAL, "normal"},
    {ATTR_FIT_STRETCHED, "stretched"}
};

static struct util_token forms[4] =
{
    {ATTR_FORM_FLOAT, "float"},
    {ATTR_FORM_MAXIMIZE, "maximize"},
    {ATTR_FORM_HORIZONTAL, "horizontal"},
    {ATTR_FORM_VERTICAL, "vertical"}
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

unsigned int attr_update(unsigned int attribute, char *value, unsigned int current)
{

    switch (attribute)
    {

    case ATTR_BLIT:
        return util_getkey(blits, 2, value);

    case ATTR_COLOR:
        return cstring_readvalue(value, cstring_length(value), 16);

    case ATTR_COLUMNS:
        return cstring_readvalue(value, cstring_length(value), 10);

    case ATTR_CONTENT:
        return pool_updatestring(current, value);

    case ATTR_FIT:
        return util_getkey(fits, 2, value);

    case ATTR_FORM:
        return util_getkey(forms, 4, value);

    case ATTR_HALIGN:
        return util_getkey(haligns, 3, value);

    case ATTR_ID:
        return pool_updatestring(current, value);

    case ATTR_IN:
        return pool_updatestring(current, value);

    case ATTR_LABEL:
        return pool_updatestring(current, value);

    case ATTR_MIMETYPE:
        return util_getkey(mimetypes, 2, value);

    case ATTR_MODE:
        return util_getkey(modes, 2, value);

    case ATTR_PADDING:
        return cstring_readvalue(value, cstring_length(value), 10);

    case ATTR_SOURCE:
        return pool_updatestring(current, value);

    case ATTR_TITLE:
        return pool_updatestring(current, value);

    case ATTR_VALIGN:
        return util_getkey(valigns, 3, value);

    case ATTR_WEIGHT:
        return util_getkey(weights, 2, value);

    case ATTR_WRAP:
        return util_getkey(wraps, 3, value);

    }

    return 0;

}

