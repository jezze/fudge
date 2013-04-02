#include <fudge.h>
#include <gfx/gfx.h>
#include <gfx/pcx.h>
#include <gfx/vga.h>

static struct pcx_surface pcx;
static struct vga_surface vga;

void main()
{

    unsigned char colormap[768];

    pcx_init_surface(&pcx, FUDGE_IN);
    vga_init_surface(&vga, FUDGE_OUT);
    pcx_load(&pcx);
    vga_load(&vga);
    gfx_convert_colormap(colormap, pcx.colormap, 768);
    call_open(FUDGE_OUT, FUDGE_ROOT, 27, "system/video/vga/resolution");
    call_write(FUDGE_OUT, 0, 9, "320x200x8");
    call_close(FUDGE_OUT);
    call_open(FUDGE_OUT, FUDGE_ROOT, 25, "system/video/vga/colormap");
    call_write(FUDGE_OUT, 0, 768, colormap);
    call_close(FUDGE_OUT);
    call_open(FUDGE_OUT, FUDGE_ROOT, 21, "system/video/vga/data");
    gfx_blit_surface(&vga.base, &pcx.base);
    call_close(FUDGE_OUT);

}

