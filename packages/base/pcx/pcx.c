#include <fudge.h>
#include <gfx/gfx.h>
#include <gfx/pcx.h>

static struct pcx_surface surface;

static void render(struct pcx_surface *surface)
{

    unsigned char colormap[768];
    unsigned int width = surface->header.xend - surface->header.xstart + 1;
    unsigned int height = surface->header.yend - surface->header.ystart + 1;
    unsigned int offset = 128;
    unsigned int row;

    gfx_convert_colormap(colormap, surface->colormap, 768);
    call_open(3, FUDGE_ROOT, 27, "system/video/vga/resolution");
    call_write(3, 0, 9, "320x200x8");
    call_close(3);
    call_open(3, FUDGE_ROOT, 25, "system/video/vga/colormap");
    call_write(3, 0, 768, colormap);
    call_close(3);
    call_open(3, FUDGE_ROOT, 21, "system/video/vga/data");

    for (row = 0; row < height; row++)
    {

        char buffer[FUDGE_BSIZE];

        offset += pcx_read(surface, offset, FUDGE_BSIZE, buffer);

        call_write(3, row * width, width, buffer);

    }

    call_close(3);

}

void main()
{

    pcx_init_surface(&surface, FUDGE_IN);
    pcx_load(&surface);
    render(&surface);

}

