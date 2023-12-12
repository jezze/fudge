#include <fudge.h>
#include <abi.h>
#include "config.h"
#include "util.h"
#include "text.h"
#include "cache.h"

void cache_updatetext(struct cache_text *cachetext, struct text_font *font, unsigned int rownum, char *content, unsigned int length, unsigned int wrap, int w, int h, int offx)
{

    cachetext->exist = 1;
    cachetext->rownum = rownum;
    cachetext->icurrent = text_getrowstart(font, content, length, rownum, wrap, w, offx);

}

void cache_initrow(struct cache_row *cacherow, struct text_rowinfo *rowinfo, struct text_font *font, unsigned int paddingx, unsigned int paddingy, unsigned int halign, unsigned int valign, int w, int h, int offx, int offy)
{

    cacherow->rx = text_getrowx(rowinfo, halign, paddingx, w - offx) + offx;
    cacherow->ry = text_getrowy(rowinfo, valign, paddingy, h - offy) + offy;
    cacherow->istart = rowinfo->istart;
    cacherow->iend = rowinfo->iend;
    cacherow->length = rowinfo->length;
    cacherow->font = font;

}

void cache_inittext(struct cache_text *cachetext, unsigned int rows, int lastrowx, int lastrowy)
{

    cachetext->rows = rows;
    cachetext->lastrowx = lastrowx;
    cachetext->lastrowy = lastrowy;
    cachetext->exist = 0;

}

