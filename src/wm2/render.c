#include <fudge.h>
#include <abi.h>
#include "config.h"
#include "util.h"
#include "widget.h"
#include "pool.h"
#include "blit.h"
#include "text.h"
#include "render.h"

#define DAMAGE_STATE_NONE               0
#define DAMAGE_STATE_MADE               1
#define CMAP_TEXT_COLOR                 0

static unsigned int *getcmap(unsigned int state, unsigned int *normal, unsigned int *hover, unsigned int *focus)
{

    switch (state)
    {

    case WIDGET_STATE_FOCUS:
        return focus;

    case WIDGET_STATE_HOVER:
        return hover;

    case WIDGET_STATE_NORMAL:
    default:
        return normal;

    }

}

static void renderbutton(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    struct widget_button *button = widget->data;
    struct render_rowinfo rowinfo;
    static unsigned int cmapnormal[3] = {
        0xE8101010,
        0xE8484848,
        0xE8888888,
    };
    static unsigned int cmaphover[3] = {
        0xE8101010,
        0xE8505050,
        0xE8888888,
    };
    static unsigned int cmapfocus[3] = {
        0xE8101010,
        0xE8585858,
        0xE8888888,
    };
    static unsigned int cmaptext[1] = {
        0xE8FFFFFF,
    };

    blit_panel(display, widget->position.x, widget->position.y, widget->size.w, widget->size.h, line, x0, x2, getcmap(widget->state, cmapnormal, cmaphover, cmapfocus));

    if (text_getrowinfo(pool_getfont(POOL_FONTBOLD), pool_getstring(button->label), pool_getcstringlength(button->label), &rowinfo, WIDGET_TEXT_WRAP_NONE, 0, 0))
    {

        int rx = text_getrowx(&rowinfo, WIDGET_TEXT_HALIGN_CENTER, widget->position.x, widget->size.w);
        int ry = text_getrowy(&rowinfo, WIDGET_TEXT_VALIGN_MIDDLE, widget->position.y, widget->size.h);

        if (util_intersects(line, ry, ry + rowinfo.lineheight))
            blit_textnormal(display, pool_getfont(POOL_FONTBOLD), getcmap(widget->state, cmaptext, cmaptext, cmaptext)[CMAP_TEXT_COLOR], pool_getstring(button->label), rowinfo.chars, rx, ry, line, x0, x2);

    }

}

static void renderchoice(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    struct widget_choice *choice = widget->data;
    struct render_rowinfo rowinfo;
    static unsigned int cmapnormal[3] = {
        0xE8101010,
        0xE8484848,
        0xE8888888,
    };
    static unsigned int cmaphover[3] = {
        0xE8101010,
        0xE8505050,
        0xE8888888,
    };
    static unsigned int cmapfocus[3] = {
        0xE8101010,
        0xE8585858,
        0xE8888888,
    };
    static unsigned int cmaptext[1] = {
        0xE8FFFFFF,
    };

    blit_panel(display, widget->position.x, widget->position.y, widget->size.w, widget->size.h, line, x0, x2, getcmap(widget->state, cmapnormal, cmaphover, cmapfocus));

    if (text_getrowinfo(pool_getfont(POOL_FONTNORMAL), pool_getstring(choice->label), pool_getcstringlength(choice->label), &rowinfo, WIDGET_TEXT_WRAP_NONE, 0, 0))
    {

        int rx = text_getrowx(&rowinfo, WIDGET_TEXT_HALIGN_LEFT, widget->position.x + CONFIG_CHOICE_PADDING_WIDTH, widget->size.w);
        int ry = text_getrowy(&rowinfo, WIDGET_TEXT_VALIGN_MIDDLE, widget->position.y, widget->size.h);

        if (util_intersects(line, ry, ry + rowinfo.lineheight))
            blit_textnormal(display, pool_getfont(POOL_FONTBOLD), getcmap(widget->state, cmaptext, cmaptext, cmaptext)[CMAP_TEXT_COLOR], pool_getstring(choice->label), rowinfo.chars, rx, ry, line, x0, x2);

    }

}

static void renderfill(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    struct widget_fill *fill = widget->data;

    blit_line(display, fill->color, x0, x2);

}

/*
static void rendergradient(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    struct widget_gradient *gradient = widget->data;
    unsigned int color0 = fill->color0;
    unsigned int color1 = fill->color1;
    unsigned int color;

    char a0 = color0 >> 24;
    char r0 = color0 >> 16;
    char g0 = color0 >> 8;
    char b0 = color0 >> 0;
    char a1 = color1 >> 24;
    char r1 = color1 >> 16;
    char g1 = color1 >> 8;
    char b1 = color1 >> 0;

    r0 = util_max(0, r0 - line / 64);
    g0 = util_max(0, g0 - line / 64);
    b0 = util_max(0, b0 - line / 64);

    color = (a0 << 24) | (r0 << 16) | (g0 << 8) | (b0 << 0);

    blit_line(display, color, x0, x2);

}
*/

static void renderimage(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    struct widget_image *image = widget->data;

    switch (image->type)
    {

    case WIDGET_IMAGE_TYPE_FUDGEMOUSE:
        {

        static unsigned int cmap[3] = {
            0xE8101010,
            0xE8202020,
            0xE8F0F0F0,
        };

        blit_mouse(display, widget->position.x, widget->position.y, widget->size.w, widget->size.h, line, x0, x2, getcmap(widget->state, cmap, cmap, cmap));

        }

        break;

    case WIDGET_IMAGE_TYPE_PCX:
        blit_pcx(display, line, pool_getstring(image->source), widget->position.x, widget->position.y, x0, x2);

        break;

    }

}

static void renderselect(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    struct widget_select *select = widget->data;
    struct render_rowinfo rowinfo;
    unsigned int extra = 16 + CONFIG_SELECT_PADDING_WIDTH * 2;
    static unsigned int cmapnormal[3] = {
        0xE8101010,
        0xE8484848,
        0xE8888888,
    };
    static unsigned int cmaphover[3] = {
        0xE8101010,
        0xE8505050,
        0xE8888888,
    };
    static unsigned int cmapfocus[3] = {
        0xE8101010,
        0xE8805050,
        0xE8E0B0B0,
    };
    static unsigned int cmaptext[1] = {
        0xE8FFFFFF,
    };
    static unsigned int cmapicon[1] = {
        0xE8FFFFFF,
    };

    blit_panel(display, widget->position.x, widget->position.y, extra, widget->size.h, line, x0, x2, getcmap(widget->state, cmapnormal, cmaphover, cmapfocus));
    blit_iconhamburger(display, widget->position.x, widget->position.y, extra, widget->size.h, line, x0, x2, cmapicon);
    blit_panel(display, widget->position.x + extra, widget->position.y, widget->size.w - extra, widget->size.h, line, x0, x2, getcmap(widget->state, cmapnormal, cmaphover, cmapfocus));

    if (text_getrowinfo(pool_getfont(POOL_FONTNORMAL), pool_getstring(select->label), pool_getcstringlength(select->label), &rowinfo, WIDGET_TEXT_WRAP_NONE, 0, 0))
    {

        int rx = text_getrowx(&rowinfo, WIDGET_TEXT_HALIGN_CENTER, widget->position.x + extra, widget->size.w - extra);
        int ry = text_getrowy(&rowinfo, WIDGET_TEXT_VALIGN_MIDDLE, widget->position.y, widget->size.h);

        if (util_intersects(line, ry, ry + rowinfo.lineheight))
            blit_textnormal(display, pool_getfont(POOL_FONTBOLD), getcmap(widget->state, cmaptext, cmaptext, cmaptext)[CMAP_TEXT_COLOR], pool_getstring(select->label), rowinfo.chars, rx, ry, line, x0, x2);

    }

}

static void rendertext(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    struct widget_text *text = widget->data;
    unsigned int fontindex = (text->weight == WIDGET_TEXT_WEIGHT_BOLD) ? POOL_FONTBOLD : POOL_FONTNORMAL;
    struct blit_font *font = pool_getfont(fontindex);
    unsigned int rownum = (line - widget->position.y) / font->lineheight;
    struct render_rowinfo rowinfo;
    unsigned int roff = (rownum) ? 0 : text->firstrowoffset;
    unsigned int rw = widget->size.w - roff;
    static unsigned int cmapnormal[1] = {
        0xE8E0E0E0,
    };
    static unsigned int cmaphover[1] = {
        0xE8E0E0E0,
    };
    static unsigned int cmapfocus[1] = {
        0xE8F0F0F0,
    };

    /* Rudimentary caching */
    if (text->rownum == 0x00FFFFFF || text->rownum != rownum)
    {

        if (rownum > text->rownum)
            text->rowstart = text_getrowinfo(font, pool_getstring(text->content), pool_getcstringlength(text->content), &rowinfo, text->wrap, rw, text->rowstart);
        else
            text->rowstart = text_getrowstart(font, pool_getstring(text->content), pool_getcstringlength(text->content), rownum, text->wrap, widget->size.w);

        text->rownum = rownum;

    }

    if (text_getrowinfo(font, pool_getstring(text->content), pool_getcstringlength(text->content), &rowinfo, text->wrap, rw, text->rowstart))
    {

        int rx = text_getrowx(&rowinfo, text->halign, widget->position.x + roff, rw);
        int ry = text_getrowy(&rowinfo, text->valign, widget->position.y + rownum * rowinfo.lineheight, widget->size.h);

        switch (text->mode)
        {

        case WIDGET_TEXT_MODE_NORMAL:
            blit_textnormal(display, font, getcmap(widget->state, cmapnormal, cmaphover, cmapfocus)[CMAP_TEXT_COLOR], pool_getstring(text->content) + text->rowstart, rowinfo.chars, rx, ry, line, x0, x2);

            break;

        case WIDGET_TEXT_MODE_INVERTED:
            blit_textinverted(display, font, getcmap(widget->state, cmapnormal, cmaphover, cmapfocus)[CMAP_TEXT_COLOR], pool_getstring(text->content) + text->rowstart, rowinfo.chars, rx, ry, line, x0, x2);

            break;

        }

    }

}

static void rendertextbox(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    struct widget_textbox *textbox = widget->data;
    static unsigned int cmapnormal[3] = {
        0xE8101010,
        0xE8282828,
        0xE8686868,
    };
    static unsigned int cmaphover[3] = {
        0xE8101010,
        0xE8282828,
        0xE8787878,
    };
    static unsigned int cmapfocus[3] = {
        0xE8101010,
        0xE8282828,
        0xE8888888,
    };

    if (textbox->mode == WIDGET_TEXTBOX_MODE_READONLY)
        blit_frame(display, widget->position.x, widget->position.y, widget->size.w, widget->size.h, line, x0, x2, cmapnormal);
    else
        blit_frame(display, widget->position.x, widget->position.y, widget->size.w, widget->size.h, line, x0, x2, getcmap(widget->state, cmapnormal, cmaphover, cmapfocus));

}

static void renderwindow(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    static unsigned int cmaptop[3] = {
        0xE8101010,
        0xE8805050,
        0xE8E0B0B0,
    };
    static unsigned int cmapmain[3] = {
        0xE8101010,
        0xE8303030,
        0xE8585858,
    };
    static unsigned int cmapicon[1] = {
        0xE8FFFFFF,
    };

    blit_panel(display, widget->position.x + CONFIG_ICON_WIDTH * 0, widget->position.y, CONFIG_ICON_WIDTH, CONFIG_ICON_HEIGHT, line, x0, x2, getcmap(widget->state, cmaptop, cmaptop, cmaptop));
    blit_iconhamburger(display, widget->position.x + CONFIG_ICON_WIDTH * 0, widget->position.y, CONFIG_ICON_WIDTH, CONFIG_ICON_HEIGHT, line, x0, x2, cmapicon);
    blit_panel(display, widget->position.x + CONFIG_ICON_WIDTH * 1, widget->position.y, CONFIG_ICON_WIDTH, CONFIG_ICON_HEIGHT, line, x0, x2, getcmap(widget->state, cmaptop, cmaptop, cmaptop));
    blit_iconminimize(display, widget->position.x + CONFIG_ICON_WIDTH * 1, widget->position.y, CONFIG_ICON_WIDTH, CONFIG_ICON_HEIGHT, line, x0, x2, cmapicon);
    blit_panel(display, widget->position.x + CONFIG_ICON_WIDTH * 2, widget->position.y, widget->size.w - CONFIG_ICON_WIDTH * 3, CONFIG_ICON_HEIGHT, line, x0, x2, getcmap(widget->state, cmaptop, cmaptop, cmaptop));
    blit_panel(display, widget->position.x + widget->size.w - CONFIG_ICON_WIDTH * 1, widget->position.y, CONFIG_ICON_WIDTH, CONFIG_ICON_HEIGHT, line, x0, x2, getcmap(widget->state, cmaptop, cmaptop, cmaptop));
    blit_iconx(display, widget->position.x + widget->size.w - CONFIG_ICON_WIDTH * 1, widget->position.y, CONFIG_ICON_WIDTH, CONFIG_ICON_HEIGHT, line, x0, x2, cmapicon);
    blit_panel(display, widget->position.x, widget->position.y + CONFIG_ICON_HEIGHT, widget->size.w, widget->size.h - CONFIG_ICON_HEIGHT, line, x0, x2, getcmap(widget->state, cmapmain, cmapmain, cmapmain));

}

static void renderwidget(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    switch (widget->type)
    {

    case WIDGET_TYPE_BUTTON:
        renderbutton(display, widget, line, x0, x2);

        break;

    case WIDGET_TYPE_CHOICE:
        renderchoice(display, widget, line, x0, x2);

        break;

    case WIDGET_TYPE_FILL:
        renderfill(display, widget, line, x0, x2);

        break;

    case WIDGET_TYPE_IMAGE:
        renderimage(display, widget, line, x0, x2);

        break;

    case WIDGET_TYPE_SELECT:
        renderselect(display, widget, line, x0, x2);

        break;

    case WIDGET_TYPE_TEXT:
        rendertext(display, widget, line, x0, x2);

        break;

    case WIDGET_TYPE_TEXTBOX:
        rendertextbox(display, widget, line, x0, x2);

        break;

    case WIDGET_TYPE_WINDOW:
        renderwindow(display, widget, line, x0, x2);

        break;

    }

}

void render_damage(struct blit_damage *damage, int x0, int y0, int x2, int y2)
{

    switch (damage->state)
    {

    case DAMAGE_STATE_NONE:
        damage->position0.x = x0;
        damage->position0.y = y0;
        damage->position2.x = x2;
        damage->position2.y = y2;

        break;

    case DAMAGE_STATE_MADE:
        damage->position0.x = util_min(x0, damage->position0.x);
        damage->position0.y = util_min(y0, damage->position0.y);
        damage->position2.x = util_max(x2, damage->position2.x);
        damage->position2.y = util_max(y2, damage->position2.y);

        break;

    }

    damage->state = DAMAGE_STATE_MADE;

}

void render(struct blit_display *display, struct blit_damage *damage)
{

    if (!display->framebuffer)
        return;

    if (damage->state == DAMAGE_STATE_MADE)
    {

        int line;

        for (line = damage->position0.y; line < damage->position2.y; line++)
        {

            struct list_item *current = 0;

            while ((current = pool_next(current)))
            {

                struct widget *widget = current->data;

                if (widget_intersectsy(widget, line))
                {

                    int x0 = util_max(widget->position.x, damage->position0.x);
                    int x2 = util_min(widget->position.x + widget->size.w, damage->position2.x);

                    renderwidget(display, widget, line, x0, x2);

                }

            }

            blit(display, damage, line);

        }

        damage->state = DAMAGE_STATE_NONE;

    }

}

