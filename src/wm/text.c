#include <fudge.h>
#include <abi.h>
#include "util.h"
#include "attr.h"
#include "text.h"

unsigned int text_getrowinfo(struct text_rowinfo *rowinfo, struct text_font *font, char *text, unsigned int length, unsigned int wrap, unsigned int maxw, unsigned int offset)
{

    unsigned int foundspace = 0;
    unsigned int foundchar = 0;
    unsigned int si = 0;
    unsigned int sw = 0;
    unsigned int sh = 0;
    unsigned int i;

    if (offset >= length)
        return 0;

    rowinfo->istart = offset;
    rowinfo->iend = offset;
    rowinfo->length = 0;
    rowinfo->width = 0;
    rowinfo->height = 0;
    rowinfo->linebreak = 0;
    rowinfo->lineheight = font->lineheight;

    for (i = offset; i < length; i++)
    {

        unsigned short index = pcf_getindex(font->data, text[i]);
        struct pcf_metricsdata metricsdata;

        pcf_readmetricsdata(font->data, index, &metricsdata);

        if (text[i] == '\n')
        {

            rowinfo->iend = i;
            rowinfo->length = rowinfo->iend - rowinfo->istart;
            rowinfo->linebreak = 1;

            return i + 1;

        }

        if (wrap == ATTR_WRAP_WORD)
        {

            if (text[i] == ' ')
            {

                if (foundchar)
                {

                    foundspace = 1;
                    si = i;
                    sw = rowinfo->width;
                    sh = rowinfo->height;

                }

            }

            else
            {

                if (!foundchar)
                {

                    foundchar = 1;
                    rowinfo->istart = i;
                    rowinfo->iend = i;

                }

            }

        }

        if (rowinfo->width + metricsdata.width > maxw)
        {

            switch (wrap)
            {

            case ATTR_WRAP_WORD:
                if (foundspace)
                {

                    rowinfo->width = sw;
                    rowinfo->height = sh;
                    rowinfo->iend = si;
                    rowinfo->length = rowinfo->iend - rowinfo->istart;
                    rowinfo->linebreak = 1;

                    return si;

                }

                else
                {

                    rowinfo->iend = i;
                    rowinfo->length = rowinfo->iend - rowinfo->istart;
                    rowinfo->linebreak = 1;

                    return i;

                }

                break;

            case ATTR_WRAP_CHAR:
                rowinfo->iend = i;
                rowinfo->length = rowinfo->iend - rowinfo->istart;
                rowinfo->linebreak = 1;

                return i;

            default:
                break;

            }

        }

        rowinfo->width = rowinfo->width + metricsdata.width;
        rowinfo->height = util_max(rowinfo->height, metricsdata.ascent + metricsdata.descent);

    }

    rowinfo->iend = i;
    rowinfo->length = rowinfo->iend - rowinfo->istart;

    return length;

}

unsigned int text_gettextinfo(struct text_info *textinfo, struct text_font *font, char *text, unsigned int length, unsigned int wrap, unsigned int maxw, int firstrowx)
{

    struct text_rowinfo rowinfo;
    unsigned int offset = 0;

    textinfo->lastrowx = firstrowx;
    textinfo->lastrowy = 0;
    textinfo->width = 0;
    textinfo->height = 0;
    textinfo->rows = 0;

    while ((offset = text_getrowinfo(&rowinfo, font, text, length, wrap, maxw - firstrowx, offset)))
    {

        textinfo->lastrowx = (rowinfo.linebreak) ? 0 : rowinfo.width + firstrowx;
        textinfo->lastrowy += (rowinfo.linebreak) ? rowinfo.lineheight : 0;
        textinfo->width = util_max(textinfo->width, rowinfo.width + firstrowx);
        textinfo->height = textinfo->lastrowy + rowinfo.lineheight;
        textinfo->rows++;

        firstrowx = 0;

    }

    return length;

}

unsigned int text_getrowstart(struct text_font *font, char *text, unsigned int length, unsigned int rownum, unsigned int wrap, unsigned int maxw, int firstrowx)
{

    struct text_rowinfo rowinfo;
    unsigned int offset = 0;
    unsigned int row = 0;

    if (!rownum)
        return 0;

    while ((offset = text_getrowinfo(&rowinfo, font, text, length, wrap, maxw - firstrowx, offset)))
    {

        if (++row == rownum)
            return offset;

        firstrowx = 0;

    }

    return length;

}

unsigned int text_getoffsetat(struct text_font *font, char *text, unsigned int length, unsigned int wrap, unsigned int maxw, int firstrowx, int x, int y)
{

    struct text_rowinfo rowinfo;
    unsigned int offset = 0;
    int cy = 0;

    while ((offset = text_getrowinfo(&rowinfo, font, text, length, wrap, maxw - firstrowx, offset)))
    {

        if (y >= cy && y < cy + rowinfo.lineheight)
            return text_getrowinfo(&rowinfo, font, text, length, ATTR_WRAP_CHAR, x - firstrowx, rowinfo.istart);

        cy += rowinfo.lineheight;

        firstrowx = 0;

    }

    return 0;

}

int text_getrowx(struct text_rowinfo *rowinfo, unsigned int halign, int x, int w)
{

    switch (halign)
    {

    case ATTR_HALIGN_LEFT:
        return x;

    case ATTR_HALIGN_CENTER:
        return x + w / 2 - rowinfo->width / 2;

    case ATTR_HALIGN_RIGHT:
        return x + w - rowinfo->width;

    }

    return 0;

}

int text_getrowy(struct text_rowinfo *rowinfo, unsigned int valign, int y, int h)
{

    switch (valign)
    {

    case ATTR_VALIGN_TOP:
        return y;

    case ATTR_VALIGN_MIDDLE:
        return y + h / 2 - rowinfo->height / 2 - (rowinfo->lineheight - rowinfo->height) / 2;

    case ATTR_VALIGN_BOTTOM:
        return y + h - rowinfo->height;

    }

    return 0;

}

