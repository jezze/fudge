#include <fudge.h>
#include <gfx/gfx.h>
#include <gfx/pcx.h>

static struct pcx_header header;

static unsigned int size()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int offset;
    unsigned int count;

    for (offset = 0; (count = call_read(FUDGE_IN, offset, FUDGE_BSIZE, buffer)); offset += count);

    return offset;

}

static unsigned int read_scanline(unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned char raw[FUDGE_BSIZE];
    unsigned int width = header.xend - header.xstart + 1;
    unsigned int rindex = 0;
    unsigned int oindex = 0;
    unsigned int repeat;
    unsigned char current;

    call_read(id, offset, count, raw);

    do
    {

        repeat = 1;
        current = raw[rindex];
        rindex++;

        if ((current & 0xC0) == 0xC0)
        {

            repeat = current & 0x3F;
            current = raw[rindex];
            rindex++;

        }

        while (repeat--)
        {

            ((char *)buffer)[oindex] = current;
            oindex++;

        }

    } while (oindex < width);

    return rindex;

}

static unsigned int read_colormap(unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (count < 769)
        return 0;

    offset = size() - 769;
    count = call_read(FUDGE_IN, offset, 769, buffer);

    if (((char *)buffer)[0] != PCX_COLORMAP_MAGIC)
        return 0;

    return count;

}

static void set_resolution()
{

    call_open(3, FUDGE_ROOT, 27, "system/video/vga/resolution");
    call_write(3, 0, 9, "320x200x8");
    call_close(3);

}

static void set_colormap()
{

    char colormap[FUDGE_BSIZE];
    unsigned int i;

    read_colormap(FUDGE_IN, 0, FUDGE_BSIZE, colormap);
    call_open(3, FUDGE_ROOT, 25, "system/video/vga/colormap");

    for (i = 0; i < PCX_COLORMAP_SLOTS; i++)
        call_write(3, i * 4, 3, &colormap[i * 3 + 1]);

    call_close(3);

}

static void render()
{

    unsigned int width = header.xend - header.xstart + 1;
    unsigned int height = header.yend - header.ystart + 1;
    unsigned int offset = 128;
    unsigned int row;

    for (row = 0; row < height; row++)
    {

        char buffer[FUDGE_BSIZE];

        offset += read_scanline(FUDGE_IN, offset, FUDGE_BSIZE, buffer);

        call_write(FUDGE_OUT, row * width, width, buffer);

    }

}

void main()
{

    call_read(FUDGE_IN, 0, sizeof (struct pcx_header), &header);
    call_open(FUDGE_OUT, FUDGE_ROOT, 21, "system/video/vga/data");
    set_resolution();
    set_colormap();
    render();

}

