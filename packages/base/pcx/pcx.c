#include <fudge.h>
#include <gfx/gfx.h>
#include <gfx/pcx.h>
#include <gfx/vga.h>

static struct pcx_surface pcx;
static struct vga_surface vga;

static void prepare()
{

    unsigned char colormap[768];

    gfx_convert_colormap(colormap, pcx.colormap, 768);

    call_open(3, FUDGE_ROOT, 27, "system/video/vga/resolution");
    call_write(3, 0, 9, "320x200x8");
    call_close(3);
    call_open(3, FUDGE_ROOT, 25, "system/video/vga/colormap");
    call_write(3, 0, 768, colormap);
    call_close(3);

}

static void render(struct gfx_surface *surface)
{

    unsigned int offset = 0;
    unsigned int scanline;

    if (surface->width > FUDGE_BSIZE)
        return;

    call_open(3, FUDGE_ROOT, 21, "system/video/vga/data");

    for (scanline = 0; scanline < surface->height; scanline++)
    {

        char buffer[FUDGE_BSIZE];

        offset += surface->read(surface, offset, surface->width, buffer);

        call_write(3, scanline * surface->width, surface->width, buffer);

    }

    call_close(3);

}

void main()
{

    pcx_init_surface(&pcx, FUDGE_IN);
    pcx_load(&pcx);
    vga_init_surface(&vga);
    vga_load(&vga);
    prepare();
    render(&pcx.base);

}

