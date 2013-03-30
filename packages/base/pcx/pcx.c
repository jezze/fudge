#include <fudge.h>
#include <gfx/gfx.h>

static struct gfx_backend backend;
static struct gfx_surface rootSurface;

void main()
{

    char back[] = {0x11, 0x11, 0x11, 0x11};

    call_open(3, FUDGE_ROOT, 27, "system/video/vga/resolution");
    call_write(3, 0, 9, "320x200x8");
    call_close(3);
    call_open(3, FUDGE_ROOT, 21, "system/video/vga/data");
    gfx_init_backend(&backend, 3);
    gfx_init_surface(&rootSurface, 320, 200, GFX_RGB8, &backend);
    backend.write(&backend, 0, 4, back);
    call_close(3);

}

