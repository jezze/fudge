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

unsigned int text_getrowstart(struct text_font *font, char *text, unsigned int length, unsigned int row, unsigned int rownum, unsigned int wrap, unsigned int maxw, unsigned int firstrowx, unsigned int offset)
{

    struct text_rowinfo rowinfo;

    if (!rownum)
        return 0;

    if ((offset = text_getrowinfo(&rowinfo, font, text, length, wrap, maxw - firstrowx, offset)))
    {

        if (++row == rownum)
            return offset;

        while ((offset = text_getrowinfo(&rowinfo, font, text, length, wrap, maxw, offset)))
        {

            if (++row == rownum)
                return offset;

        }

    }

    return length;

}

unsigned int text_getrowinfo(struct text_rowinfo *rowinfo, struct text_font *font, char *text, unsigned int length, unsigned int wrap, unsigned int maxw, unsigned int offset)
{

    unsigned int foundspace = 0;
    unsigned int foundchar = 0;
    unsigned int foundmaxwidth = 0;
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
    rowinfo->newline = 0;
    rowinfo->lineheight = font->lineheight;

    for (i = offset; i < length; i++)
    {

        unsigned short index = pcf_getindex(font->data, text[i]);
        struct pcf_metricsdata metricsdata;

        pcf_readmetricsdata(font->data, index, &metricsdata);

        if (rowinfo->width + metricsdata.width > maxw)
            foundmaxwidth = 1;

        switch (text[i])
        {

        case '\n':
            rowinfo->iend = i;
            rowinfo->length = rowinfo->iend - rowinfo->istart;
            rowinfo->newline = 1;

            return i + 1;

        case ' ':
            foundspace = 1;
            si = i;
            sw = rowinfo->width;
            sh = rowinfo->height;

            break;

        default:
            if (wrap == TEXT_WRAP_WORD)
            {

                if (!foundchar)
                {

                    rowinfo->istart = i;
                    rowinfo->iend = i;

                }

            }

            foundchar = 1;

            break;

        }

        if (foundmaxwidth)
        {

            switch (wrap)
            {

            case TEXT_WRAP_WORD:
                if (foundspace)
                {

                    rowinfo->width = sw;
                    rowinfo->height = sh;
                    rowinfo->iend = si;
                    rowinfo->length = rowinfo->iend - rowinfo->istart;

                    return si;

                }

                else
                {

                    rowinfo->iend = i;
                    rowinfo->length = rowinfo->iend - rowinfo->istart;

                    return i;

                }

                break;

            case TEXT_WRAP_CHAR:
                rowinfo->iend = i;
                rowinfo->length = rowinfo->iend - rowinfo->istart;

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

unsigned int text_gettextinfo(struct text_info *textinfo, struct text_font *font, char *text, unsigned int length, unsigned int wrap, unsigned int maxw, unsigned int firstrowx)
{

    struct text_rowinfo rowinfo;
    unsigned int offset = 0;

    textinfo->lastrowx = firstrowx;
    textinfo->lastrowy = 0;
    textinfo->width = 0;
    textinfo->height = 0;
    textinfo->rows = 0;

    if ((offset = text_getrowinfo(&rowinfo, font, text, length, wrap, maxw - firstrowx, offset)))
    {

        textinfo->lastrowx = (rowinfo.newline) ? 0 : rowinfo.width + firstrowx;
        textinfo->lastrowy += (rowinfo.newline) ? rowinfo.lineheight : 0;
        textinfo->width = util_max(textinfo->width, rowinfo.width + firstrowx);
        textinfo->height += rowinfo.lineheight;
        textinfo->rows++;

        while ((offset = text_getrowinfo(&rowinfo, font, text, length, wrap, maxw, offset)))
        {

            textinfo->lastrowx = (rowinfo.newline) ? 0 : rowinfo.width;
            textinfo->lastrowy += rowinfo.lineheight;
            textinfo->width = util_max(textinfo->width, rowinfo.width);
            textinfo->height += rowinfo.lineheight;
            textinfo->rows++;

        }

    }

    return length;

}

