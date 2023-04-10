#include <fudge.h>
#include <abi.h>
#include "config.h"
#include "util.h"
#include "text.h"
#include "cache.h"
#include "attr.h"
#include "widget.h"
#include "strpool.h"
#include "pool.h"
#include "blit.h"
#include "render.h"

struct
{

    unsigned int hasdamage;
    struct util_position position0;
    struct util_position position2;

} area;

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

static void renderbutton(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
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

static void renderchoice(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
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

static void renderfill(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_fill *fill = widget->data;

    blit_line(display, fill->color, x0, x2);

}

/*
static void rendergradient(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
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

static void renderimage(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
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

static void renderlistbox(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_listbox *listbox = widget->data;
    static unsigned int cmapframe[9] = {
        0xE8101010, 0xE8282828, 0xE8686868,
        0xE8101010, 0xE8282828, 0xE8787878,
        0xE8101010, 0xE8282828, 0xE8888888,
    };

    blit_frame(display, widget->position.x, widget->position.y, widget->size.w, widget->size.h, line, x0, x2, getcmap(widget->state, cmapframe, (listbox->mode == ATTR_MODE_READONLY) ? 0 : 3));

}

static void renderselect(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_select *select = widget->data;
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

    blit_panel(display, widget->position.x, widget->position.y, widget->size.w, widget->size.h, line, x0, x2, getcmap(widget->state, cmappanel, 3));
    blit_icondownarrow(display, widget->position.x, widget->position.y, widget->size.h, widget->size.h, line, x0, x2, getcmap(widget->state, cmapicon, 0));

    if (util_intersects(line, widget->position.y + select->cacherow.ry, widget->position.y + select->cacherow.ry + select->cacherow.font->lineheight))
        blit_text(display, select->cacherow.font, ATTR_BLIT_NORMAL, strpool_getstring(select->label) + select->cacherow.istart, select->cacherow.length, widget->position.x + select->cacherow.rx, widget->position.y + select->cacherow.ry, line, x0, x2, getcmap(widget->state, cmaptext, 0));

}

static void rendertext(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_text *text = widget->data;
    struct text_font *font = pool_getfont(text->weight);
    unsigned int rownum = (line - widget->position.y) / font->lineheight;

    if (rownum >= text->cachetext.rows)
        return;

    if (!text->cachetext.exist || text->cachetext.rownum != rownum)
    {

        struct text_rowinfo rowinfo;

        cache_updatetext(&text->cachetext, font, rownum, strpool_getstring(text->content), strpool_getcstringlength(text->content), text->wrap, widget->size.w, widget->size.h);
        text_getrowinfo(&rowinfo, font, strpool_getstring(text->content), strpool_getcstringlength(text->content), text->wrap, widget->size.w, text->cachetext.icurrent);
        cache_initrow(&text->cacherow, &rowinfo, font, 0, 0, text->halign, text->valign, widget->size.w, widget->size.h, (rownum) ? 0 : text->cachetext.firstrowx, text->cachetext.rownum * rowinfo.lineheight);

    }

    if (text->cacherow.length)
    {

        static unsigned int cmaptext[1] = {
            0xE8E0E0E0,
        };

        blit_text(display, text->cacherow.font, text->blit, strpool_getstring(text->content) + text->cacherow.istart, text->cacherow.length, widget->position.x + text->cacherow.rx, widget->position.y + text->cacherow.ry, line, x0, x2, getcmap(widget->state, cmaptext, 0));

    }

}

static void rendertextbox(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_textbox *textbox = widget->data;
    static unsigned int cmapframe[9] = {
        0xE8101010, 0xE8282828, 0xE8686868,
        0xE8101010, 0xE8282828, 0xE8787878,
        0xE8101010, 0xE8282828, 0xE8888888,
    };

    blit_frame(display, widget->position.x, widget->position.y, widget->size.w, widget->size.h, line, x0, x2, getcmap(widget->state, cmapframe, (textbox->mode == ATTR_MODE_READONLY) ? 0 : 3));

}

static void rendertextbutton(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_textbutton *textbutton = widget->data;
    static unsigned int cmaprect[3] = {
        0x00202020,
        0xE8202020,
        0xE8202020,
    };
    static unsigned int cmaptext[1] = {
        0xE8FFFFFF,
    };

    blit_rect(display, widget->position.x, widget->position.y, widget->size.w, widget->size.h, line, x0, x2, getcmap(widget->state, cmaprect, 1));

    if (util_intersects(line, widget->position.y + textbutton->cacherow.ry, widget->position.y + textbutton->cacherow.ry + textbutton->cacherow.font->lineheight))
        blit_text(display, textbutton->cacherow.font, ATTR_BLIT_NORMAL, strpool_getstring(textbutton->label) + textbutton->cacherow.istart, textbutton->cacherow.length, widget->position.x + textbutton->cacherow.rx, widget->position.y + textbutton->cacherow.ry, line, x0, x2, getcmap(widget->state, cmaptext, 0));


}

static void renderwindow(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    static unsigned int cmaptop[3] = {
        0xE8101010, 0xE8805050, 0xE8E0B0B0,
    };
    static unsigned int cmapmain[3] = {
        0xE8101010, 0xE8303030, 0xE8585858,
    };
    static unsigned int cmapicon[3] = {
        0xA8FFFFFF,
        0xE8FFFFFF,
        0xE8FFFFFF,
    };

    unsigned int onhamburger = util_intersects(mx, widget->position.x, widget->position.x + CONFIG_WINDOW_BUTTON_WIDTH) && util_intersects(my, widget->position.y, widget->position.y + CONFIG_WINDOW_BUTTON_HEIGHT);
    unsigned int onminimize = util_intersects(mx, widget->position.x + CONFIG_WINDOW_BUTTON_WIDTH, widget->position.x + CONFIG_WINDOW_BUTTON_WIDTH * 2) && util_intersects(my, widget->position.y, widget->position.y + CONFIG_WINDOW_BUTTON_HEIGHT);
    unsigned int onx = util_intersects(mx, widget->position.x + widget->size.w - CONFIG_WINDOW_BUTTON_WIDTH, widget->position.x + widget->size.w) && util_intersects(my, widget->position.y, widget->position.y + CONFIG_WINDOW_BUTTON_HEIGHT);

    blit_panel(display, widget->position.x, widget->position.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, getcmap(widget->state, cmaptop, 0));
    blit_panel(display, widget->position.x + CONFIG_WINDOW_BUTTON_WIDTH, widget->position.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, getcmap(widget->state, cmaptop, 0));
    blit_panel(display, widget->position.x + CONFIG_WINDOW_BUTTON_WIDTH * 2, widget->position.y, widget->size.w - CONFIG_WINDOW_BUTTON_WIDTH * 3, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, getcmap(widget->state, cmaptop, 0));
    blit_panel(display, widget->position.x + widget->size.w - CONFIG_WINDOW_BUTTON_WIDTH, widget->position.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, getcmap(widget->state, cmaptop, 0));
    blit_panel(display, widget->position.x, widget->position.y + CONFIG_WINDOW_BUTTON_HEIGHT, widget->size.w, widget->size.h - CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, getcmap(widget->state, cmapmain, 0));
    blit_iconhamburger(display, widget->position.x, widget->position.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, getcmap(widget->state, cmapicon, (onhamburger) ? 1 : 0));
    blit_iconminimize(display, widget->position.x + CONFIG_WINDOW_BUTTON_WIDTH, widget->position.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, getcmap(widget->state, cmapicon, (onminimize) ? 1 : 0));
    blit_iconx(display, widget->position.x + widget->size.w - CONFIG_WINDOW_BUTTON_WIDTH, widget->position.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, getcmap(widget->state, cmapicon, (onx) ? 1 : 0));

}

static void renderwidget(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    switch (widget->type)
    {

    case WIDGET_TYPE_BUTTON:
        renderbutton(display, widget, line, x0, x2, mx, my);

        break;

    case WIDGET_TYPE_CHOICE:
        renderchoice(display, widget, line, x0, x2, mx, my);

        break;

    case WIDGET_TYPE_FILL:
        renderfill(display, widget, line, x0, x2, mx, my);

        break;

    case WIDGET_TYPE_IMAGE:
        renderimage(display, widget, line, x0, x2, mx, my);

        break;

    case WIDGET_TYPE_LISTBOX:
        renderlistbox(display, widget, line, x0, x2, mx, my);

        break;

    case WIDGET_TYPE_SELECT:
        renderselect(display, widget, line, x0, x2, mx, my);

        break;

    case WIDGET_TYPE_TEXT:
        rendertext(display, widget, line, x0, x2, mx, my);

        break;

    case WIDGET_TYPE_TEXTBOX:
        rendertextbox(display, widget, line, x0, x2, mx, my);

        break;

    case WIDGET_TYPE_TEXTBUTTON:
        rendertextbutton(display, widget, line, x0, x2, mx, my);

        break;

    case WIDGET_TYPE_WINDOW:
        renderwindow(display, widget, line, x0, x2, mx, my);

        break;

    }

}

void render_damage(int x0, int y0, int x2, int y2)
{

    if (area.hasdamage)
    {

        area.position0.x = util_min(x0, area.position0.x);
        area.position0.y = util_min(y0, area.position0.y);
        area.position2.x = util_max(x2, area.position2.x);
        area.position2.y = util_max(y2, area.position2.y);

    }

    else
    {

        area.position0.x = x0;
        area.position0.y = y0;
        area.position2.x = x2;
        area.position2.y = y2;

    }

    area.hasdamage = 1;

}

void render_undamage(void)
{

    area.hasdamage = 0;

}

static unsigned int shouldrender(struct widget *widget, int line)
{

    struct widget *parent;

    if (widget->display == WIDGET_DISPLAY_HIDDEN)
        return 0;

     if (!widget_intersectsy(widget, line))
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

            return util_intersects(line, position.y, position.y + size.h);

        }

    }

    return 1;

}

void render(struct blit_display *display, int mx, int my)
{

    int line;

    for (line = area.position0.y; line < area.position2.y; line++)
    {

        struct list_item *current = 0;

        while ((current = pool_next(current)))
        {

            struct widget *widget = current->data;

            if (shouldrender(widget, line))
            {

                int x0 = util_max(widget->position.x, area.position0.x);
                int x2 = util_min(widget->position.x + widget->size.w, area.position2.x);

                renderwidget(display, widget, line, x0, x2, mx, my);

            }

        }

        blit(display, line, area.position0.x, area.position2.x);

    }

}

