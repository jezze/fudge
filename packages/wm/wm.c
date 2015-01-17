#include <fudge.h>

struct box
{

    unsigned int x;
    unsigned int y;
    unsigned int w;
    unsigned int h;

};

struct panel
{

    unsigned int active;
    struct box size;
    struct box border;
    struct box frame;
    struct box frametitle;

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

};

static unsigned char colormap[] = {
    0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF,
    0x04, 0x02, 0x02,
    0x06, 0x04, 0x04,
    0x08, 0x06, 0x06,
    0x18, 0x10, 0x20,
    0x1C, 0x14, 0x24
};

static unsigned char buffer[4096];

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

void draw_line(unsigned int offset, unsigned int count)
{

    call_write(CALL_L0, offset, count, buffer);

}

void draw_end()
{

    call_close(CALL_L0);

}

void fillbuffer(unsigned int color, unsigned int count)
{

    unsigned int i;

    for (i = 0; i < count; i++)
        buffer[i] = color;

}

void fill(struct box *box, char color)
{

    unsigned int i;

    fillbuffer(color, box->w);

    for (i = 0; i < box->h; i++)
        draw_line(box->y * 320 + box->x + 320 * i, box->w);

}

static void box_setsize(struct box *box, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{

    box->x = x;
    box->y = y;
    box->w = w;
    box->h = h;

}

static void panel_setsize(struct panel *panel, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{

    panel->active = 0;

    box_setsize(&panel->size, x, y, w, h);
    box_setsize(&panel->border, panel->size.x, panel->size.y, panel->size.w, panel->size.h);
    box_setsize(&panel->frame, panel->border.x + 1, panel->border.y + 1, panel->border.w - 2, panel->border.h - 2);
    box_setsize(&panel->frametitle, panel->frame.x + 1, panel->frame.y + 1, panel->frame.w - 2, panel->frame.h - 2);

}

static void panel_draw(struct panel *panel)
{

    fill(&panel->border, 0);

    if (panel->active)
    {

        fill(&panel->frame, 6);
        fill(&panel->frametitle, 5);

    }

    else
    {

        fill(&panel->frame, 4);
        fill(&panel->frametitle, 3);

    }

}

static void window_setsize(struct window *window, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{

    window->active = 0;

    box_setsize(&window->size, x, y, w, h);
    box_setsize(&window->border, window->size.x, window->size.y, window->size.w, window->size.h);
    box_setsize(&window->frame, window->border.x + 1, window->border.y + 1, window->border.w - 2, window->border.h - 2);
    box_setsize(&window->frametitle, window->frame.x + 1, window->frame.y + 1, window->frame.w - 2, 16);
    box_setsize(&window->bodyborder, window->frame.x + 1, window->frame.y + 18, window->frame.w - 2, window->frame.h - 19);
    box_setsize(&window->body, window->bodyborder.x + 1, window->bodyborder.y + 1, window->bodyborder.w - 2, window->bodyborder.h - 2);

}

static void window_draw(struct window *window)
{

    fill(&window->border, 0);

    if (window->active)
    {

        fill(&window->frame, 6);
        fill(&window->frametitle, 5);

    }

    else
    {

        fill(&window->frame, 4);
        fill(&window->frametitle, 3);

    }

    fill(&window->bodyborder, 0);
    fill(&window->body, 2);

}

void main()
{

    struct box back;
    struct panel panel1;
    struct panel panel2;
    struct panel panel3;
    struct panel panel4;
    struct panel panel5;
    struct panel panel6;
    struct window win1;
    struct window win2;
    struct window win3;

    box_setsize(&back, 0, 0, 320, 200);
    panel_setsize(&panel1, 1, 1, 17, 17);
    panel_setsize(&panel2, 18, 1, 17, 17);
    panel_setsize(&panel3, 35, 1, 17, 17);
    panel_setsize(&panel4, 52, 1, 17, 17);
    panel_setsize(&panel5, 69, 1, 212, 17);
    panel_setsize(&panel6, 281, 1, 38, 17);
    window_setsize(&win1, 1, 18, 160, 181);
    window_setsize(&win2, 161, 18, 158, 100);
    window_setsize(&win3, 161, 118, 158, 81);

    panel2.active = 1;
    win1.active = 1;

    setmode();
    setcolormap();
    draw_begin();
    fill(&back, 0);
    panel_draw(&panel1);
    panel_draw(&panel2);
    panel_draw(&panel3);
    panel_draw(&panel4);
    panel_draw(&panel5);
    panel_draw(&panel6);
    window_draw(&win1);
    window_draw(&win2);
    window_draw(&win3);
    draw_end();

}

