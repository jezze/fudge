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
#include "cmap.h"
#include "render.h"

struct
{

    unsigned int hasdamage;
    struct util_position position0;
    struct util_position position2;

} area;

static void renderbutton(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_button *button = widget->data;

    blit_frame(display, widget->position.x, widget->position.y, widget->size.w, widget->size.h, line, x0, x2, cmap_get(widget->state, widget->type, 0, 4));

    if (util_intersects(line, widget->position.y + button->cacherow.ry, widget->position.y + button->cacherow.ry + button->cacherow.font->lineheight))
        blit_text(display, button->cacherow.font, strpool_getstring(button->label) + button->cacherow.istart, button->cacherow.length, widget->position.x + button->cacherow.rx, widget->position.y + button->cacherow.ry, line, x0, x2, 0, 0, cmap_get(widget->state, widget->type, 12, 0));

}

static void renderchoice(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_choice *choice = widget->data;

    blit_frame(display, widget->position.x, widget->position.y, widget->size.w, widget->size.h, line, x0, x2, cmap_get(widget->state, widget->type, 0, 4));

    if (util_intersects(line, widget->position.y + choice->cacherow.ry, widget->position.y + choice->cacherow.ry + choice->cacherow.font->lineheight))
        blit_text(display, choice->cacherow.font, strpool_getstring(choice->label) + choice->cacherow.istart, choice->cacherow.length, widget->position.x + choice->cacherow.rx, widget->position.y + choice->cacherow.ry, line, x0, x2, 0, 0, cmap_get(widget->state, widget->type, 12, 0));

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
        blit_mouse(display, widget->position.x, widget->position.y, widget->size.w, widget->size.h, line, x0, x2, cmap_get(widget->state, widget->type, 0, 0));

        break;

    case ATTR_MIMETYPE_PCX:
        blit_pcx(display, line, strpool_getstring(image->source), widget->position.x, widget->position.y, x0, x2);

        break;

    }

}

static void renderlistbox(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_listbox *listbox = widget->data;

    blit_frame(display, widget->position.x, widget->position.y, widget->size.w, widget->size.h, line, x0, x2, cmap_get(widget->state, widget->type, 0, (listbox->mode == ATTR_MODE_READONLY) ? 0 : 4));

}

static void renderselect(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_select *select = widget->data;

    blit_frame(display, widget->position.x, widget->position.y, widget->size.w, widget->size.h, line, x0, x2, cmap_get(widget->state, widget->type, 0, 4));
    blit_iconarrowdown(display, widget->position.x + widget->size.w - CONFIG_SELECT_PADDING_WIDTH - widget->size.h / 2, widget->position.y, widget->size.h, widget->size.h, line, x0, x2, cmap_get(widget->state, widget->type, 12, 0));

    if (util_intersects(line, widget->position.y + select->cacherow.ry, widget->position.y + select->cacherow.ry + select->cacherow.font->lineheight))
        blit_text(display, select->cacherow.font, strpool_getstring(select->label) + select->cacherow.istart, select->cacherow.length, widget->position.x + select->cacherow.rx, widget->position.y + select->cacherow.ry, line, x0, x2, 0, 0, cmap_get(widget->state, widget->type, 13, 0));

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

        cache_updatetext(&text->cachetext, font, rownum, strpool_getstring(text->content), strpool_getcstringlength(text->content), text->wrap, widget->size.w, widget->size.h, text->cachetext.offx);
        text_getrowinfo(&rowinfo, font, strpool_getstring(text->content), strpool_getcstringlength(text->content), text->wrap, widget->size.w, text->cachetext.icurrent);
        cache_initrow(&text->cacherow, &rowinfo, font, 0, 0, text->halign, text->valign, widget->size.w, widget->size.h, text->cachetext.offx, text->cachetext.rownum * rowinfo.lineheight);

    }

    if (text->cacherow.length)
        blit_text(display, text->cacherow.font, strpool_getstring(text->content) + text->cacherow.istart, text->cacherow.length, widget->position.x + text->cacherow.rx, widget->position.y + text->cacherow.ry, line, x0, x2, text->cachetext.markstart, text->cachetext.marklength, cmap_get(widget->state, widget->type, 0, 0));

}

static void rendertextbox(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_textbox *textbox = widget->data;

    blit_frame(display, widget->position.x, widget->position.y, widget->size.w, widget->size.h, line, x0, x2, cmap_get(widget->state, widget->type, (textbox->mode == ATTR_MODE_READONLY) ? 12 : 0, (textbox->mode == ATTR_MODE_READONLY) ? 0 : 4));

}

static void rendertextbutton(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_textbutton *textbutton = widget->data;

    blit_frame(display, widget->position.x, widget->position.y, widget->size.w, widget->size.h, line, x0, x2, cmap_get(widget->state, widget->type, 0, 4));

    if (util_intersects(line, widget->position.y + textbutton->cacherow.ry, widget->position.y + textbutton->cacherow.ry + textbutton->cacherow.font->lineheight))
        blit_text(display, textbutton->cacherow.font, strpool_getstring(textbutton->label) + textbutton->cacherow.istart, textbutton->cacherow.length, widget->position.x + textbutton->cacherow.rx, widget->position.y + textbutton->cacherow.ry, line, x0, x2, 0, 0, cmap_get(widget->state, widget->type, 12, 0));

}

static void rendertextedit(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_textedit *textedit = widget->data;
    struct text_font *font = pool_getfont(textedit->weight);
    unsigned int rownum = (line - widget->position.y) / font->lineheight;

    if (rownum >= textedit->cachetext.rows)
        return;

    if (!textedit->cachetext.exist || textedit->cachetext.rownum != rownum)
    {

        struct text_rowinfo rowinfo;

        cache_updatetext(&textedit->cachetext, font, rownum, strpool_getstring(textedit->content), strpool_getcstringlength(textedit->content), textedit->wrap, widget->size.w, widget->size.h, textedit->cachetext.offx);
        text_getrowinfo(&rowinfo, font, strpool_getstring(textedit->content), strpool_getcstringlength(textedit->content), textedit->wrap, widget->size.w, textedit->cachetext.icurrent);
        cache_initrow(&textedit->cacherow, &rowinfo, font, 0, 0, textedit->halign, textedit->valign, widget->size.w, widget->size.h, textedit->cachetext.offx, textedit->cachetext.rownum * rowinfo.lineheight);

    }

    if (textedit->cacherow.length)
    {

        if (textedit->cursor >= textedit->cacherow.istart && textedit->cursor < textedit->cacherow.istart + textedit->cacherow.length)
            blit_textedit(display, textedit->cacherow.font, textedit->cursor - textedit->cacherow.istart, strpool_getstring(textedit->content) + textedit->cacherow.istart, textedit->cacherow.length, widget->position.x + textedit->cacherow.rx, widget->position.y + textedit->cacherow.ry, line, x0, x2, cmap_get(widget->state, widget->type, 0, 0));
        else
            blit_text(display, textedit->cacherow.font, strpool_getstring(textedit->content) + textedit->cacherow.istart, textedit->cacherow.length, widget->position.x + textedit->cacherow.rx, widget->position.y + textedit->cacherow.ry, line, x0, x2, textedit->cachetext.markstart, textedit->cachetext.marklength, cmap_get(widget->state, widget->type, 0, 0));

    }

}

static void renderwindow(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_window *window = widget->data;
    unsigned int onhamburger = util_intersects(mx, widget->position.x, widget->position.x + CONFIG_WINDOW_BUTTON_WIDTH) && util_intersects(my, widget->position.y, widget->position.y + CONFIG_WINDOW_BUTTON_HEIGHT);
    unsigned int onminimize = util_intersects(mx, widget->position.x + CONFIG_WINDOW_BUTTON_WIDTH, widget->position.x + CONFIG_WINDOW_BUTTON_WIDTH * 2) && util_intersects(my, widget->position.y, widget->position.y + CONFIG_WINDOW_BUTTON_HEIGHT);
    unsigned int onx = util_intersects(mx, widget->position.x + widget->size.w - CONFIG_WINDOW_BUTTON_WIDTH, widget->position.x + widget->size.w) && util_intersects(my, widget->position.y, widget->position.y + CONFIG_WINDOW_BUTTON_HEIGHT);

    blit_frame(display, widget->position.x, widget->position.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, cmap_get(widget->state, widget->type, 0, 0));
    blit_frame(display, widget->position.x + CONFIG_WINDOW_BUTTON_WIDTH, widget->position.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, cmap_get(widget->state, widget->type, 0, 0));
    blit_frame(display, widget->position.x + CONFIG_WINDOW_BUTTON_WIDTH * 2, widget->position.y, widget->size.w - CONFIG_WINDOW_BUTTON_WIDTH * 3, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, cmap_get(widget->state, widget->type, 0, 0));
    blit_frame(display, widget->position.x + widget->size.w - CONFIG_WINDOW_BUTTON_WIDTH, widget->position.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, cmap_get(widget->state, widget->type, 0, 0));
    blit_frame(display, widget->position.x, widget->position.y + CONFIG_WINDOW_BUTTON_HEIGHT, widget->size.w, widget->size.h - CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, cmap_get(widget->state, widget->type, 4, 0));

    if (util_intersects(line, widget->position.y + window->cacherow.ry, widget->position.y + window->cacherow.ry + window->cacherow.font->lineheight))
        blit_text(display, window->cacherow.font, strpool_getstring(window->title) + window->cacherow.istart, window->cacherow.length, widget->position.x + window->cacherow.rx, widget->position.y + window->cacherow.ry, line, x0, x2, 0, 0, cmap_get(widget->state, widget->type, 11, 0));

    blit_iconhamburger(display, widget->position.x, widget->position.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, cmap_get(widget->state, widget->type, 8, (onhamburger) ? 1 : 0));
    blit_iconminimize(display, widget->position.x + CONFIG_WINDOW_BUTTON_WIDTH, widget->position.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, cmap_get(widget->state, widget->type, 8, (onminimize) ? 1 : 0));
    blit_iconx(display, widget->position.x + widget->size.w - CONFIG_WINDOW_BUTTON_WIDTH, widget->position.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, cmap_get(widget->state, widget->type, 8, (onx) ? 1 : 0));

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

    case WIDGET_TYPE_TEXTEDIT:
        rendertextedit(display, widget, line, x0, x2, mx, my);

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

void render(struct blit_display *display, int mx, int my)
{

    int line;

    for (line = area.position0.y; line < area.position2.y; line++)
    {

        struct list_item *current = 0;

        while ((current = pool_next(current)))
        {

            struct widget *widget = current->data;

            if (widget_intersectsy(widget, line))
            {

                int x0 = util_max(widget->position.x, area.position0.x);
                int x2 = util_min(widget->position.x + widget->size.w, area.position2.x);

                renderwidget(display, widget, line, x0, x2, mx, my);

            }

        }

        blit(display, line, area.position0.x, area.position2.x);

    }

}

