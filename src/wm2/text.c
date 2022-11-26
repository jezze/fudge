#include <fudge.h>
#include <abi.h>
#include "util.h"
#include "text.h"

int text_getrowx(struct text_rowinfo *rowinfo, unsigned int halign, int x, int w)
{

    switch (halign)
    {

    case TEXT_HALIGN_LEFT:
        return x;

    case TEXT_HALIGN_CENTER:
        return x + w / 2 - rowinfo->width / 2;

    case TEXT_HALIGN_RIGHT:
        return x + w - rowinfo->width;

    }

    return 0;

}

int text_getrowy(struct text_rowinfo *rowinfo, unsigned int valign, int y, int h)
{

    switch (valign)
    {

    case TEXT_VALIGN_TOP:
        return y;

    case TEXT_VALIGN_MIDDLE:
        return y + h / 2 - rowinfo->height / 2 - (rowinfo->lineheight - rowinfo->height) / 2;

    case TEXT_VALIGN_BOTTOM:
        return y + h - rowinfo->height;

    }

    return 0;

}

unsigned int text_getrowstart(struct text_font *font, char *text, unsigned int length, unsigned int rownum, unsigned int wrap, unsigned int maxw, int crow, int offset)
{

    unsigned int rows;
    struct text_rowinfo rowinfo;

    if (!rownum)
        return 0;

    for (rows = crow; (offset = text_getrowinfo(&rowinfo, font, text, length, wrap, maxw, offset)); rows++)
    {

        if (rows == rownum)
            return offset;

    }

    return length;

}

unsigned int text_getrowinfo(struct text_rowinfo *rowinfo, struct text_font *font, char *text, unsigned int length, unsigned int wrap, unsigned int maxw, unsigned int offset)
{

    unsigned int si = 0;
    unsigned int w = 0;
    unsigned int sw = 0;
    unsigned int h = 0;
    unsigned int sh = 0;
    unsigned int i;

    if (offset >= length)
        return 0;

    rowinfo->chars = 0;
    rowinfo->width = 0;
    rowinfo->height = 0;
    rowinfo->newline = 0;
    rowinfo->lineheight = font->lineheight;

    for (i = offset; i < length; i++)
    {

        struct pcf_metricsdata metricsdata;
        unsigned short index;

        if (text[i] == ' ')
        {

            si = i;
            sw = w;
            sh = h;

        }

        if (text[i] == '\n')
        {

            rowinfo->newline = 1;

            break;

        }

        index = pcf_getindex(font->data, text[i]);

        pcf_readmetricsdata(font->data, index, &metricsdata);

        if (wrap != TEXT_WRAP_NONE && w + metricsdata.width >= maxw)
        {

            if (wrap == TEXT_WRAP_WORD && si)
            {

                i = si;
                w = sw;
                h = sh;

            }

            else
            {

                i--;

            }

            break;

        }

        w += metricsdata.width;
        h = util_max(h, metricsdata.ascent + metricsdata.descent);

    }

    rowinfo->width = w;
    rowinfo->height = h;
    rowinfo->chars = i - offset;

    return i + 1;

}

unsigned int text_gettextinfo(struct text_info *textinfo, struct text_font *font, char *text, unsigned int length, unsigned int wrap, unsigned int firstrowoffset, unsigned int maxw)
{

    struct text_rowinfo rowinfo;
    unsigned int offset = 0;

    textinfo->lastrowx = firstrowoffset;
    textinfo->lastrowy = 0;
    textinfo->width = 0;
    textinfo->height = 0;
    textinfo->rows = 0;

    if ((offset = text_getrowinfo(&rowinfo, font, text, length, wrap, util_max(0, maxw - textinfo->lastrowx), offset)))
    {

        textinfo->lastrowx = (rowinfo.newline) ? 0 : rowinfo.width + textinfo->lastrowx;
        textinfo->lastrowy += rowinfo.height;
        textinfo->width = util_max(textinfo->width, rowinfo.width + textinfo->lastrowx);
        textinfo->height += rowinfo.height;
        textinfo->rows++;

        while ((offset = text_getrowinfo(&rowinfo, font, text, length, wrap, util_max(0, maxw - textinfo->lastrowx), offset)))
        {

            textinfo->lastrowx = (rowinfo.newline) ? 0 : rowinfo.width + textinfo->lastrowx;
            textinfo->lastrowy += rowinfo.height;
            textinfo->width = util_max(textinfo->width, rowinfo.width + textinfo->lastrowx);
            textinfo->height += rowinfo.height;
            textinfo->rows++;

        }

    }

    return length;

}

