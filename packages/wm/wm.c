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

    unsigned int active;
    struct box size;
    struct box border;
    struct box frame;
    struct box frametitle;
    struct box bodyborder;
    struct box body;
    struct text title;

};

struct event
{

    unsigned int type;
    unsigned int count;
    unsigned char buffer[FUDGE_BSIZE];

};

static struct box back;
static struct box empty;
static struct panel view[4];
static struct panel field;
static struct panel clock;
static struct window win[32];
static unsigned int wins;
static unsigned int winactive;
static unsigned int views;
static unsigned int viewactive;

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

static void view_activate(unsigned int index)
{

    view[viewactive].active = 0;

    viewactive = index;

    view[viewactive].active = 1;

}

static void window_activate(unsigned int index)
{

    win[winactive].active = 0;

    winactive = index;

    win[winactive].active = 1;

}

void draw()
{

    unsigned int i;

    draw_begin();
    box_draw(&back, 0);
    box_draw(&empty, 2);

    for (i = 0; i < views; i++)
        panel_draw(&view[i]);

    panel_draw(&field);
    panel_draw(&clock);

    for (i = 0; i < wins; i++)
        window_draw(&win[i]);

    draw_end();

}

void poll()
{

    struct event event;
    unsigned int count, roff;

    call_walk(CALL_L1, CALL_DR, 14, "system/event/0");
    call_open(CALL_L1);

    for (roff = 0; (count = call_read(CALL_L1, roff, FUDGE_BSIZE, &event)); roff += count)
    {

        switch (event.type)
        {

        case 1:
            if (event.buffer[0] == 0x02)
                view_activate(0);

            if (event.buffer[0] == 0x03)
                view_activate(1);

            if (event.buffer[0] == 0x04)
                view_activate(2);

            if (event.buffer[0] == 0x05)
                view_activate(3);

            break;

        case 4:
            window_init(&win[wins], 1, 18, 318, 181, "1212", 0);

            window_activate(wins);

            wins++;

            break;

        }

        draw();

    }

    call_close(CALL_L1);

}

void main()
{

    wins = 0;
    winactive = 0;
    views = 4;
    viewactive = 0;

    box_setsize(&back, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    box_setsize(&empty, 2, 19, 316, 179);
    panel_init(&view[0], 1, 1, 17, 17, "1", 1);
    panel_init(&view[1], 18, 1, 17, 17, "2", 0);
    panel_init(&view[2], 35, 1, 17, 17, "3", 0);
    panel_init(&view[3], 52, 1, 17, 17, "1", 0);
    panel_init(&field, 69, 1, 212, 17, "1", 0);
    panel_init(&clock, 281, 1, 38, 17, "1", 0);

    setmode();
    setcolormap();
    draw();
    poll();

}

