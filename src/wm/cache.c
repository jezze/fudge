#include <fudge.h>
#include <abi.h>
#include "config.h"
#include "util.h"
#include "text.h"
#include "cache.h"

void cache_initrow(struct cache_row *cacherow, struct text_rowinfo *rowinfo, unsigned int num, struct text_font *font, unsigned int paddingx, unsigned int paddingy, unsigned int halign, unsigned int valign, int w, int h, int offx, int offy)
{

    cacherow->num = num;
    cacherow->rx = text_getrowx(rowinfo, halign, paddingx, w - offx) + offx;
    cacherow->ry = text_getrowy(rowinfo, valign, paddingy, h - offy) + offy;
    cacherow->istart = rowinfo->istart;
    cacherow->iend = rowinfo->iend;
    cacherow->length = rowinfo->length;
    cacherow->font = font;

}

