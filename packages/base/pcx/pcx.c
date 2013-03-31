#include <fudge.h>
#include <gfx/gfx.h>
#include <gfx/pcx.h>

static struct gfx_backend backend;
static struct gfx_surface rootSurface;
static struct pcx_header header;

static void print_data()
{

    unsigned int width = header.xend - header.xstart + 1;
    unsigned int height = header.yend - header.ystart + 1;
    unsigned int scanline = header.nplanes * header.bpl;
    unsigned int offset = 128;
    unsigned int row;

    for (row = 0; row < height; row++)
    {

        char raw[0x1000];
        char buffer[0x1000];
        unsigned int rindex = 0;
        unsigned int bindex = 0;
        unsigned int count;
        char current;

        call_read(FUDGE_IN, offset, scanline, &raw);

        do
        {

            count = 1;
            current = raw[rindex];
            rindex++;

            if ((current & 0xC0) == 0xC0)
            {

                count = current & 0x3F;
                current = raw[rindex];
                rindex++;

            }

            for (; count; count--)
            {

                buffer[bindex] = current;
                bindex++;

            }

        } while (bindex < width);

        offset += rindex;

        call_write(FUDGE_OUT, row * width, width, &buffer);

    }

}

void main()
{

    call_read(FUDGE_IN, 0, sizeof (struct pcx_header), &header);
    call_open(FUDGE_OUT, FUDGE_ROOT, 21, "system/video/vga/data");
    call_open(3, FUDGE_ROOT, 27, "system/video/vga/resolution");
    call_write(3, 0, 9, "320x200x8");
    call_close(3);
    gfx_init_backend(&backend, FUDGE_OUT);
    gfx_init_surface(&rootSurface, 320, 200, GFX_RGB8, &backend);
    print_data();

}

