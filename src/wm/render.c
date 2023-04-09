#include <fudge.h>
#include <abi.h>
#include "config.h"
#include "util.h"
#include "attr.h"
#include "widget.h"
#include "text.h"
#include "strpool.h"
#include "pool.h"
#include "blit.h"
#include "render.h"

#define DAMAGE_STATE_NONE               0
#define DAMAGE_STATE_MADE               1

static unsigned int *getcmap(unsigned int state, unsigned int *cmap, unsigned int step)
{

    switch (state)
    {

    case WIDGET_STATE_FOCUS:
        return cmap + step * 2;

    case WIDGET_STATE_HOVER:
        return cmap + step * 1;

    case WIDGET_STATE_NORMAL:
    default:
        return cmap + step * 0;

    }

}

static void renderbutton(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    struct widget_button *button = widget->data;
    static unsigned int cmappanel[9] = {
        0xE8101010, 0xE8484848, 0xE8888888,
        0xE8101010, 0xE8505050, 0xE8888888,
        0xE8101010, 0xE8585858, 0xE8888888,
    };
    static unsigned int cmaptext[1] = {
        0xE8FFFFFF,
    };

    blit_panel(display, widget->position.x, widget->position.y, widget->size.w, widget->size.h, line, x0, x2, getcmap(widget->state, cmappanel, 3));

    if (util_intersects(line, widget->position.y + button->cacherow.ry, widget->position.y + button->cacherow.ry + button->cacherow.font->lineheight))
        blit_text(display, button->cacherow.font, ATTR_BLIT_NORMAL, strpool_getstring(button->label) + button->cacherow.istart, button->cacherow.length, widget->position.x + button->cacherow.rx, widget->position.y + button->cacherow.ry, line, x0, x2, getcmap(widget->state, cmaptext, 0));

}

static void renderchoice(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    struct widget_choice *choice = widget->data;
    static unsigned int cmappanel[9] = {
        0xE8101010, 0xE8484848, 0xE8888888,
        0xE8101010, 0xE8505050, 0xE8888888,
        0xE8101010, 0xE8585858, 0xE8888888,
    };
    static unsigned int cmaptext[1] = {
        0xE8FFFFFF,
    };

    blit_panel(display, widget->position.x, widget->position.y, widget->size.w, widget->size.h, line, x0, x2, getcmap(widget->state, cmappanel, 3));

    if (util_intersects(line, widget->position.y + choice->cacherow.ry, widget->position.y + choice->cacherow.ry + choice->cacherow.font->lineheight))
        blit_text(display, choice->cacherow.font, ATTR_BLIT_NORMAL, strpool_getstring(choice->label) + choice->cacherow.istart, choice->cacherow.length, widget->position.x + choice->cacherow.rx, widget->position.y + choice->cacherow.ry, line, x0, x2, getcmap(widget->state, cmaptext, 0));

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

    switch (image->mimetype)
    {

    case ATTR_MIMETYPE_FUDGEMOUSE:
        {

        static unsigned int cmap[3] = {
            0xE8101010, 0xE8202020, 0xE8F0F0F0,
        };

        blit_mouse(display, widget->position.x, widget->position.y, widget->size.w, widget->size.h, line, x0, x2, getcmap(widget->state, cmap, 0));

        }

        break;

    case ATTR_MIMETYPE_PCX:
        blit_pcx(display, line, strpool_getstring(image->source), widget->position.x, widget->position.y, x0, x2);

        break;

    }

}

static void renderlistbox(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    struct widget_listbox *listbox = widget->data;
    static unsigned int cmapframe[9] = {
        0xE8101010, 0xE8282828, 0xE8686868,
        0xE8101010, 0xE8282828, 0xE8787878,
        0xE8101010, 0xE8282828, 0xE8888888,
    };

    blit_frame(display, widget->position.x, widget->position.y, widget->size.w, widget->size.h, line, x0, x2, getcmap(widget->state, cmapframe, (listbox->mode == ATTR_MODE_READONLY) ? 0 : 3));

}

static void renderselect(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    struct widget_select *select = widget->data;
    unsigned int extra = CONFIG_SELECT_EXTRA + CONFIG_SELECT_PADDING_WIDTH * 2;
    static unsigned int cmappanel[9] = {
        0xE8101010, 0xE8484848, 0xE8888888,
        0xE8101010, 0xE8505050, 0xE8888888,
        0xE8101010, 0xE8805050, 0xE8E0B0B0,
    };
    static unsigned int cmapicon[1] = {
        0xE8FFFFFF,
    };
    static unsigned int cmaptext[1] = {
        0xE8FFFFFF,
    };

    blit_panel(display, widget->position.x, widget->position.y, extra, widget->size.h, line, x0, x2, getcmap(widget->state, cmappanel, 3));
    blit_panel(display, widget->position.x + extra, widget->position.y, widget->size.w - extra, widget->size.h, line, x0, x2, getcmap(widget->state, cmappanel, 3));
    blit_iconhamburger(display, widget->position.x, widget->position.y, extra, widget->size.h, line, x0, x2, getcmap(widget->state, cmapicon, 0));

    if (util_intersects(line, widget->position.y + select->cacherow.ry, widget->position.y + select->cacherow.ry + select->cacherow.font->lineheight))
        blit_text(display, select->cacherow.font, ATTR_BLIT_NORMAL, strpool_getstring(select->label) + select->cacherow.istart, select->cacherow.length, widget->position.x + select->cacherow.rx, widget->position.y + select->cacherow.ry, line, x0, x2, getcmap(widget->state, cmaptext, 0));

}

static unsigned int updatetextcache(struct widget *widget, struct widget_text *text, int line)
{

    unsigned int rownum = (line - widget->position.y) / text->cacherow.font->lineheight;

    if (rownum >= text->cachetext.rows)
        return 0;

    if (!text->cachetext.exist || text->cachetext.rownum != rownum)
    {

        unsigned int rowx = (rownum) ? 0 : text->cachetext.firstrowx;
        struct text_rowinfo rowinfo;
        unsigned int offset = 0;
        unsigned int frownum = 0;

        if (rownum > text->cachetext.rownum)
        {

            offset = text->cachetext.icurrent;
            frownum = text->cachetext.rownum;

        }

        text->cachetext.exist = 1;
        text->cachetext.rownum = rownum;
        text->cachetext.icurrent = text_getrowstart(text->cacherow.font, strpool_getstring(text->content), strpool_getcstringlength(text->content), frownum, rownum, text->wrap, widget->size.w, text->cachetext.firstrowx, offset);

        text_getrowinfo(&rowinfo, text->cacherow.font, strpool_getstring(text->content), strpool_getcstringlength(text->content), text->wrap, widget->size.w, text->cachetext.icurrent);

        text->cacherow.rx = text_getrowx(&rowinfo, text->halign, rowx, widget->size.w - rowx);
        text->cacherow.ry = text_getrowy(&rowinfo, text->valign, text->cachetext.rownum * rowinfo.lineheight, widget->size.h);
        text->cacherow.istart = rowinfo.istart;
        text->cacherow.length = rowinfo.length;

    }

    return text->cacherow.length;

}

static void rendertext(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    struct widget_text *text = widget->data;

    if (updatetextcache(widget, text, line))
    {

        static unsigned int cmaptext[3] = {
            0xE8E0E0E0, 0xE8E0E0E0, 0xE8F0F0F0,
        };

        blit_text(display, text->cacherow.font, text->blit, strpool_getstring(text->content) + text->cacherow.istart, text->cacherow.length, widget->position.x + text->cacherow.rx, widget->position.y + text->cacherow.ry, line, x0, x2, getcmap(widget->state, cmaptext, 1));

    }

}

static void rendertextbox(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    struct widget_textbox *textbox = widget->data;
    static unsigned int cmapframe[9] = {
        0xE8101010, 0xE8282828, 0xE8686868,
        0xE8101010, 0xE8282828, 0xE8787878,
        0xE8101010, 0xE8282828, 0xE8888888,
    };

    blit_frame(display, widget->position.x, widget->position.y, widget->size.w, widget->size.h, line, x0, x2, getcmap(widget->state, cmapframe, (textbox->mode == ATTR_MODE_READONLY) ? 0 : 3));

}

static void rendertextbutton(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    struct widget_textbutton *textbutton = widget->data;
    static unsigned int cmaprect[3] = {
        0x00000000, 0xE8242424, 0xE8202020,
    };
    static unsigned int cmaptext[1] = {
        0xE8FFFFFF,
    };

    blit_rect(display, widget->position.x, widget->position.y, widget->size.w, widget->size.h, line, x0, x2, getcmap(widget->state, cmaprect, 1));

    if (util_intersects(line, widget->position.y + textbutton->cacherow.ry, widget->position.y + textbutton->cacherow.ry + textbutton->cacherow.font->lineheight))
        blit_text(display, textbutton->cacherow.font, ATTR_BLIT_NORMAL, strpool_getstring(textbutton->label) + textbutton->cacherow.istart, textbutton->cacherow.length, widget->position.x + textbutton->cacherow.rx, widget->position.y + textbutton->cacherow.ry, line, x0, x2, getcmap(widget->state, cmaptext, 0));

}

static void renderwindow(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    static unsigned int cmaptop[3] = {
        0xE8101010, 0xE8805050, 0xE8E0B0B0,
    };
    static unsigned int cmapmain[3] = {
        0xE8101010, 0xE8303030, 0xE8585858,
    };
    static unsigned int cmapicon[1] = {
        0xE8FFFFFF,
    };

    blit_panel(display, widget->position.x + CONFIG_WINDOW_BUTTON_WIDTH * 0, widget->position.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, getcmap(widget->state, cmaptop, 0));
    blit_iconhamburger(display, widget->position.x + CONFIG_WINDOW_BUTTON_WIDTH * 0, widget->position.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, cmapicon);
    blit_panel(display, widget->position.x + CONFIG_WINDOW_BUTTON_WIDTH * 1, widget->position.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, getcmap(widget->state, cmaptop, 0));
    blit_iconminimize(display, widget->position.x + CONFIG_WINDOW_BUTTON_WIDTH * 1, widget->position.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, cmapicon);
    blit_panel(display, widget->position.x + CONFIG_WINDOW_BUTTON_WIDTH * 2, widget->position.y, widget->size.w - CONFIG_WINDOW_BUTTON_WIDTH * 3, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, getcmap(widget->state, cmaptop, 0));
    blit_panel(display, widget->position.x + widget->size.w - CONFIG_WINDOW_BUTTON_WIDTH * 1, widget->position.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, getcmap(widget->state, cmaptop, 0));
    blit_iconx(display, widget->position.x + widget->size.w - CONFIG_WINDOW_BUTTON_WIDTH * 1, widget->position.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, cmapicon);
    blit_panel(display, widget->position.x, widget->position.y + CONFIG_WINDOW_BUTTON_HEIGHT, widget->size.w, widget->size.h - CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, getcmap(widget->state, cmapmain, 0));

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

    case WIDGET_TYPE_LISTBOX:
        renderlistbox(display, widget, line, x0, x2);

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

    case WIDGET_TYPE_TEXTBUTTON:
        rendertextbutton(display, widget, line, x0, x2);

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

static unsigned int shouldrender(struct widget *widget, int line)
{

    struct widget *parent;

    if (widget->display == WIDGET_DISPLAY_HIDDEN)
        return 0;

    switch (widget->type)
    {

    case WIDGET_TYPE_TEXT:
    case WIDGET_TYPE_TEXTBUTTON:
        parent = pool_getwidgetbyid(widget->source, strpool_getstring(widget->in));

        if (parent)
        {

            struct util_position position;
            struct util_size size;

            widget_getclipping(parent, &position, &size);

            return widget_intersectsy(widget, line) && util_intersects(line, position.y, position.y + size.h);

        }

    default:
        return widget_intersectsy(widget, line);

    }

    return 0;

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

                if (shouldrender(widget, line))
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

