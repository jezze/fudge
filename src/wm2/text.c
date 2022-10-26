#include <fudge.h>
#include <abi.h>
#include "util.h"
#include "widget.h"
#include "blit.h"
#include "text.h"

int text_getrowx(struct render_rowinfo *rowinfo, unsigned int halign, int x, int w)
{

    switch (halign)
    {

    case WIDGET_TEXT_HALIGN_LEFT:
        return x;

    case WIDGET_TEXT_HALIGN_CENTER:
        return x + w / 2 - rowinfo->width / 2;

    case WIDGET_TEXT_HALIGN_RIGHT:
        return x + w - rowinfo->width;

    }

    return 0;

}

int text_getrowy(struct render_rowinfo *rowinfo, unsigned int valign, int y, int h)
{

    switch (valign)
    {

    case WIDGET_TEXT_VALIGN_TOP:
        return y;

    case WIDGET_TEXT_VALIGN_MIDDLE:
        return y + h / 2 - rowinfo->height / 2 - (rowinfo->lineheight - rowinfo->height) / 2;

    case WIDGET_TEXT_VALIGN_BOTTOM:
        return y + h - rowinfo->height;

    }

    return 0;

}

unsigned int text_getrowstart(struct blit_font *font, char *text, unsigned int length, unsigned int rownum, unsigned int wrap, unsigned int maxw)
{

    unsigned int offset = 0;
    unsigned int rows;
    struct render_rowinfo rowinfo;

    if (!rownum)
        return 0;

    for (rows = 1; (offset = text_getrowinfo(font, text, length, &rowinfo, wrap, maxw, offset)); rows++)
    {

        if (rows == rownum)
            return offset;

    }

    return length;

}

unsigned int text_getrowinfo(struct blit_font *font, char *text, unsigned int length, struct render_rowinfo *rowinfo, unsigned int wrap, unsigned int maxw, unsigned int offset)
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

        if (wrap != WIDGET_TEXT_WRAP_NONE && w + metricsdata.width >= maxw)
        {

            if (wrap == WIDGET_TEXT_WRAP_WORD && si)
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

unsigned int text_gettextinfo(struct blit_font *font, char *text, unsigned int length, struct render_textinfo *textinfo, unsigned int wrap, unsigned int offw, unsigned int maxw)
{

    unsigned int offset = 0;

    if ((offset = text_getrowinfo(font, text, length, &textinfo->last, wrap, maxw - offw, offset)))
    {

        textinfo->last.width += offw;
        textinfo->lineheight = textinfo->last.lineheight;
        textinfo->width = textinfo->last.width;
        textinfo->height = textinfo->last.height;
        textinfo->rows = 1;

        while ((offset = text_getrowinfo(font, text, length, &textinfo->last, wrap, maxw, offset)))
        {

            textinfo->width = util_max(textinfo->width, textinfo->last.width);
            textinfo->height += textinfo->last.height;
            textinfo->rows++;

        }

    }

    return length;

}

