#include <fudge.h>
#include "wm.h"

unsigned int read_line(void *buffer)
{

    char *in = (char *)buffer;

    unsigned int i;

    for (i = 0; i < 512; i++)
    {

        if (in[i] == '\n')
            return i + 1;

    }

    return 0;

}

void ppm_parse(struct ppm_header *header, void *buffer)
{

    unsigned int offset = 0;
    char *in = (char *)buffer;

    memory_copy(header->sign, in, 2);

    offset = read_line(in + offset);

    offset = read_line(in + offset);

    header->width = 20;
    header->height = 20;

    offset = read_line(in + offset);

    header->data = in + 0x34;

}

static unsigned int idlfb;
static unsigned int idmx;
static unsigned int idmy;

static char buffer[0x2000];

static int mx;
static int my;

static void draw_pixel(unsigned int x, unsigned int y, unsigned int color)
{

    unsigned int offset = (y * SCREEN_WIDTH + x) * 4;

    call_write(idlfb, offset, 4, &color);

}

static void draw_buffer(unsigned int x, unsigned int y, unsigned int count, void *buffer)
{

    unsigned int offset = (y * SCREEN_WIDTH + x) * 4;

    call_write(idlfb, offset, count, buffer);

}

static void draw_stroke(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int color)
{

    unsigned int width = x2 - x1 + 1;
    unsigned int i;

    for (i = 0; i <= width; i++)
        memory_copy(buffer + i * 4, &color, 4);

    draw_buffer(x1, y1, width * 4, &buffer);

    for (i = y1; i <= y2; i++)
    {

        draw_pixel(x1, i, color);
        draw_pixel(x2, i, color);

    }

    draw_buffer(x1, y2, width * 4, &buffer);

}

static void draw_fill(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int color)
{

    unsigned int width = x2 - x1 + 1;
    unsigned int i;

    for (i = 0; i <= width; i++)
        memory_copy(buffer + i * 4, &color, 4);

    for (i = y1; i <= y2; i++)
        draw_buffer(x1, i, width * 4, &buffer);

}

static void draw_ppm(char *name, unsigned int x, unsigned int y)
{

    unsigned int id = call_open(FILE_NEW, name);
    call_read(id, 0, 0x2000, buffer);

    struct ppm_header header;

    ppm_parse(&header, buffer);

    header.width = 20;
    header.height = 20;
    unsigned int offset = 0x34;

    unsigned int count = call_read(id, offset, 0x2000, buffer);

    int cx = x;
    int cy = y;

    unsigned int i;

    for (i = 1; i < count; i += 3)
    {

        draw_pixel(cx, cy, *(unsigned int *)(buffer + i));

        cx++;

        if (cx == x + header.width)
        {

            cx = x;
            cy++;

        }

    }

    call_close(id);

}

void set_xres(unsigned int xres)
{

    unsigned int id = call_open(FILE_NEW, "/module/bga/xres");
    call_write(id, 0, 4, &xres);
    call_close(id);

}

void set_yres(unsigned int yres)
{

    unsigned int id = call_open(FILE_NEW, "/module/bga/yres");
    call_write(id, 0, 4, &yres);
    call_close(id);

}

void set_bpp(unsigned int bpp)
{

    unsigned int id = call_open(FILE_NEW, "/module/bga/bpp");
    call_write(id, 0, 4, &bpp);
    call_close(id);

}

void enable()
{

    unsigned int enable = 1;

    unsigned int id = call_open(FILE_NEW, "/module/bga/enable");
    call_write(id, 0, 4, &enable);
    call_close(id);

}

void set_mouse_coords(unsigned int x, unsigned int y)
{

    mx = x;
    my = y;

    if (mx > SCREEN_WIDTH)
        mx = SCREEN_WIDTH;

    if (my > SCREEN_HEIGHT)
        my = SCREEN_HEIGHT;

}

void mouse_event()
{

    char dx;
    char dy;

    call_read(idmx, 0, 1, &dx);
    call_read(idmy, 0, 1, &dy);

    set_mouse_coords(mx + (dx * 0.2), my - (dy * 0.2));

    draw_ppm("/ramdisk/data/fu-raw.ppm", mx, my);

    call_wait();

}

void draw_frame(unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{

    draw_fill(x, y, w, h, FRAME_BACKGROUND);
    draw_stroke(x + 0, y + 0, w - 0, h - 0, FRAME_BORDER_DARK);
    draw_stroke(x + 1, y + 1, w - 1, h - 1, FRAME_BORDER_LIGHT);
    draw_stroke(x + 5, y + 35, w - 5, h - 5, FRAME_BORDER_DARK);
    draw_stroke(x + 4, y + 34, w - 4, h - 4, FRAME_BORDER_LIGHT);

}

void draw_panel(unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{

    draw_fill(x, y, w, h, PANEL_BACKGROUND);
    draw_stroke(x, y, w, h, PANEL_BORDER_DARK);
    draw_stroke(x + 1, y + 1, w - 1, h - 1, PANEL_BORDER_LIGHT);

}

void draw_window(unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{

    draw_frame(x, y, w, h);
    draw_fill(x + 6, y + 36, w - 6, h - 6, WINDOW_BACKGROUND);
    draw_panel(x + 24, y + 54, w - 24, y + 112);
    draw_panel(x + 24, y + 112, w - 24, h - 24);

}

void main(int argc, char *argv[])
{

    idlfb = call_open(FILE_NEW, "/module/bga/lfb");

    if (!idlfb)
        return;

    idmx = call_open(FILE_NEW, "/module/ps2/mx");
    idmy = call_open(FILE_NEW, "/module/ps2/my");

    set_xres(SCREEN_WIDTH);
    set_yres(SCREEN_HEIGHT);
    set_bpp(SCREEN_BPP);
    enable();

    set_mouse_coords(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

    draw_fill(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BACKGROUND);
    draw_window(200, 100, 600, 400);

    call_attach(0x2C, mouse_event);
    call_wait();

}

