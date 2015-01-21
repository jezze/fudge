#include <fudge.h>

#define SCREEN_WIDTH                    320
#define SCREEN_HEIGHT                   200
#define VIEW_WIDTH                      160

struct box
{

    unsigned int x;
    unsigned int y;
    unsigned int w;
    unsigned int h;

};

struct glyph
{

    struct box size;
    const char c;
    const unsigned char *buffer;

};

struct text
{

    struct box size;
    const char *content;

};

struct panel
{

    unsigned int active;
    struct box size;
    struct box border;
    struct box frame;
    struct box frametitle;
    struct text title;

};

struct window
{

    struct list_item item;
    unsigned int active;
    struct box size;
    struct box border;
    struct box frame;
    struct box frametitle;
    struct box bodyborder;
    struct box body;
    struct text title;

};

struct view
{

    unsigned int active;
    struct panel panel;
    struct list windows;
    struct window *windowactive;

};

struct event_header
{

    unsigned int destination;
    unsigned int source;
    unsigned int type;
    unsigned int count;

};

static struct box back;
static struct box empty;
static struct panel field;
static struct panel clock;
struct window window[32];
static unsigned int windows;
static struct view view[4];
static unsigned int views;
static struct view *viewactive;

static unsigned char colormap[] = {
    0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF,
    0x04, 0x02, 0x02,
    0x06, 0x04, 0x04,
    0x08, 0x06, 0x06,
    0x18, 0x10, 0x20,
    0x1C, 0x14, 0x24
};

static const unsigned char letter_[] = {
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00
};

static const unsigned char letter0[] = {
    0x00, 0x01, 0x01, 0x01, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x01,
    0x01, 0x00, 0x00, 0x01, 0x01,
    0x01, 0x00, 0x01, 0x00, 0x01,
    0x01, 0x01, 0x00, 0x00, 0x01,
    0x01, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x01, 0x01, 0x01, 0x00
};

static const unsigned char letter1[] = {
    0x00, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x01, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x01, 0x01, 0x01, 0x00
};

static const unsigned char letter2[] = {
    0x00, 0x01, 0x01, 0x01, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x01, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x01
};

static const unsigned char letter3[] = {
    0x00, 0x01, 0x01, 0x01, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01,
    0x01, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x01, 0x01, 0x01, 0x00
};

struct glyph font[] = {
    {{0, 0, 5, 7}, '_', letter_},
    {{0, 0, 5, 7}, '0', letter0},
    {{7, 7, 5, 7}, '1', letter1},
    {{24, 7, 5, 7}, '2', letter2},
    {{0, 0, 5, 7}, '3', letter3}
};

static unsigned char backbuffer[4096];

struct glyph *glyph_find(char c)
{

    unsigned int i;

    for (i = 0; i < 5; i++)
    {

        if (font[i].c == c)
            return &font[i];

    }

    return 0;

}

void setmode()
{

    call_walk(CALL_L0, CALL_DR, 23, "system/video/vga:0/ctrl");
    call_open(CALL_L0);
    call_write(CALL_L0, 0, 0, 0);
    call_close(CALL_L0);

}

void setcolormap()
{

    call_walk(CALL_L0, CALL_DR, 27, "system/video/vga:0/colormap");
    call_open(CALL_L0);
    call_write(CALL_L0, 0, 21, colormap);
    call_close(CALL_L0);

}

void draw_begin()
{

    call_walk(CALL_L0, CALL_DR, 23, "system/video/vga:0/data");
    call_open(CALL_L0);

}

void draw_backbuffer(unsigned int offset, unsigned int count, unsigned int o)
{

    call_write(CALL_L0, offset, count, backbuffer + o);

}

void draw_normal(struct box *box)
{

    unsigned int i;

    for (i = 0; i < box->h; i++)
        draw_backbuffer(box->y * SCREEN_WIDTH + box->x + SCREEN_WIDTH * i, box->w, box->w * i);

}

void draw_repeated(struct box *box)
{

    unsigned int i;

    for (i = 0; i < box->h; i++)
        draw_backbuffer(box->y * SCREEN_WIDTH + box->x + SCREEN_WIDTH * i, box->w, 0);

}

void draw_end()
{

    call_close(CALL_L0);

}

void backbuffer_fill(unsigned char color, unsigned int count)
{

    unsigned int i;

    for (i = 0; i < count; i++)
        backbuffer[i] = color;

}

void backbuffer_blitbuffer(const unsigned char *buffer, unsigned int count)
{

    memory_copy(backbuffer, buffer, count);

}

void backbuffer_blitglyph(struct glyph *glyph, unsigned char bgcolor, unsigned char fgcolor)
{

    unsigned int i;

    for (i = 0; i < glyph->size.w * glyph->size.h; i++)
    {

        if (glyph->buffer[i])
            backbuffer[i] = fgcolor;
         else
            backbuffer[i] = bgcolor;

    }

}

void box_draw(struct box *box, unsigned char color)
{

    backbuffer_fill(color, box->w);
    draw_repeated(box);

}

void glyph_draw(struct glyph *glyph)
{

    backbuffer_blitglyph(glyph, 0, 1);
    draw_normal(&glyph->size);

}

void box_setsize(struct box *box, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{

    box->x = x;
    box->y = y;
    box->w = w;
    box->h = h;

}

void glyph_setsize(struct glyph *glyph, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{

    box_setsize(&glyph->size, x, y, w, h);

}

void panel_init(struct panel *panel, unsigned int x, unsigned int y, unsigned int w, unsigned int h, char *text, unsigned int active)
{

    box_setsize(&panel->size, x, y, w, h);
    box_setsize(&panel->border, panel->size.x, panel->size.y, panel->size.w, panel->size.h);
    box_setsize(&panel->frame, panel->border.x + 1, panel->border.y + 1, panel->border.w - 2, panel->border.h - 2);
    box_setsize(&panel->frametitle, panel->frame.x + 1, panel->frame.y + 1, panel->frame.w - 2, panel->frame.h - 2);

    panel->active = active;
    panel->title.content = text;

}

void panel_draw(struct panel *panel)
{

    box_draw(&panel->border, 0);

    if (panel->active)
    {

        box_draw(&panel->frame, 6);
        box_draw(&panel->frametitle, 5);

    }

    else
    {

        box_draw(&panel->frame, 4);
        box_draw(&panel->frametitle, 3);

    }

}

void window_init(struct window *window, unsigned int x, unsigned int y, unsigned int w, unsigned int h, char *text, unsigned int active)
{

    list_inititem(&window->item, window);
    box_setsize(&window->size, x, y, w, h);
    box_setsize(&window->border, window->size.x, window->size.y, window->size.w, window->size.h);
    box_setsize(&window->frame, window->border.x + 1, window->border.y + 1, window->border.w - 2, window->border.h - 2);
    box_setsize(&window->frametitle, window->frame.x + 1, window->frame.y + 1, window->frame.w - 2, 16);
    box_setsize(&window->bodyborder, window->frame.x + 1, window->frame.y + 18, window->frame.w - 2, window->frame.h - 19);
    box_setsize(&window->body, window->bodyborder.x + 1, window->bodyborder.y + 1, window->bodyborder.w - 2, window->bodyborder.h - 2);

    window->active = active;
    window->title.content = text;

}

void window_draw(struct window *window)
{

    box_draw(&window->border, 0);

    if (window->active)
    {

        box_draw(&window->frame, 6);
        box_draw(&window->frametitle, 5);

    }

    else
    {

        box_draw(&window->frame, 4);
        box_draw(&window->frametitle, 3);

    }

    box_draw(&window->bodyborder, 0);
    box_draw(&window->body, 2);

}

void view_init(struct view *view, unsigned int x, unsigned int y, unsigned int w, unsigned int h, char *text, unsigned int active)
{

    panel_init(&view->panel, x, y, w, h, text, active);
    list_init(&view->windows);

    view->active = active;
    view->windowactive = 0;

}

void draw(struct box *bb)
{

    unsigned int i;

    draw_begin();
    panel_draw(&field);
    panel_draw(&clock);

    for (i = 0; i < views; i++)
    {

        struct view *v = &view[i];

        panel_draw(&v->panel);

        if (!v->active)
            continue;

        if (v->windows.head)
        {

            struct window *w = v->windows.head->data;

            window_draw(w);

        }

        else
        {

            box_draw(&empty, 2);

        }

    }

    draw_end();

}

static struct window *event_newwindow()
{

    window_init(&window[windows], 1, 18, 318, 181, "1212", 0);

    windows++;

    return &window[windows - 1];

}

static void event_activateview(struct view *v)
{

    if (viewactive)
    {

        if (viewactive == v)
            return;

        viewactive->active = 0;
        viewactive->panel.active = 0;

        draw(&viewactive->panel.size);

    }

    viewactive = v;

    viewactive->active = 1;
    viewactive->panel.active = 1;

    draw(&viewactive->panel.size);

}

static void event_addwindow(struct view *v, struct window *w)
{

    list_add(&v->windows, &w->item);

}

static void event_activatewindow(struct view *v, struct window *w)
{
    if (v->windowactive == w)
        return;

    if (v->windowactive)
    {

        v->windowactive->active = 0;

        draw(&v->windowactive->size);

    }

    v->windowactive = w;

    v->windowactive->active = 1;
    draw(&w->size);

}

unsigned int event_next(void *buffer)
{

    struct event_header *header = buffer;

    return header->count + sizeof (struct event_header);

}

void poll()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count, roff;

    call_walk(CALL_L1, CALL_DR, 15, "system/event/wm");
    call_open(CALL_L1);

    for (roff = 0; (count = call_read(CALL_L1, roff, FUDGE_BSIZE, buffer)); roff += count)
    {

        unsigned int i;

        for (i = 0; i < count; i += event_next(buffer + i))
        {


            unsigned char *temp = buffer + i;
            struct event_header *header = (struct event_header *)temp;
            unsigned char *data = temp + sizeof (struct event_header);

            switch (header->type)
            {

            case 1:
                if (data[0] == 0x02)
                    event_activateview(&view[0]);

                if (data[0] == 0x03)
                    event_activateview(&view[1]);

                if (data[0] == 0x04)
                    event_activateview(&view[2]);

                if (data[0] == 0x05)
                    event_activateview(&view[3]);

                break;

            case 4:
                {

                    struct window *w = event_newwindow();

                    event_addwindow(viewactive, w);
                    event_activatewindow(viewactive, w);

                }

                break;

            }

        }

    }

    call_close(CALL_L1);

}

void main()
{

    windows = 0;
    views = 4;
    viewactive = &view[0];

    box_setsize(&back, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    box_setsize(&empty, 2, 19, 316, 179);
    view_init(&view[0], 1, 1, 17, 17, "1", 1);
    view_init(&view[1], 18, 1, 17, 17, "2", 0);
    view_init(&view[2], 35, 1, 17, 17, "3", 0);
    view_init(&view[3], 52, 1, 17, 17, "4", 0);
    panel_init(&field, 69, 1, 212, 17, "1", 0);
    panel_init(&clock, 281, 1, 38, 17, "1", 0);

    setmode();
    setcolormap();
    draw_begin();
    box_draw(&back, 0);
    draw_end();
    draw(&back);
    poll();

}

