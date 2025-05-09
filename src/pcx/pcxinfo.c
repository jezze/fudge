#include <fudge.h>
#include <abi.h>
#include <image.h>

static char *encodings[2] = {
    "None",
    "RLE"
};

static char *palettemodes[3] = {
    "None",
    "Monochrome/Color",
    "Greyscale"
};

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int target = fs_auth(mdata);
    unsigned int id = fs_walk(1, target, 0, mdata);

    if (id)
    {

        struct pcx_header header;
        struct record record;
        unsigned char magic;
        unsigned int width;
        unsigned int height;

        fs_stat(1, target, id, &record);
        fs_read_all(1, target, id, &header, sizeof (struct pcx_header), 0);

        width = header.xend - header.xstart + 1;
        height = header.yend - header.ystart + 1;

        channel_send_fmt1(0, source, EVENT_DATA, "Identifier: %c\n", &header.identifier);
        channel_send_fmt1(0, source, EVENT_DATA, "Version: %c\n", &header.version);
        channel_send_fmt1(0, source, EVENT_DATA, "Encoding: %s\n", (header.encoding < 2) ? encodings[(unsigned int)header.encoding] : "Unknown");
        channel_send_fmt3(0, source, EVENT_DATA, "Size: %ix%ix%c\n", &width, &height, &header.bpp);
        channel_send_fmt1(0, source, EVENT_DATA, "Planes: %c\n", &header.nplanes);
        channel_send_fmt1(0, source, EVENT_DATA, "Palette mode: %s\n", (header.palettemode < 3) ? palettemodes[header.palettemode] : "Unknown");

        if (header.palettemode)
        {

            fs_read_all(1, target, id, &magic, 1, record.size - 768 - 1);
            channel_send_fmt1(0, source, EVENT_DATA, "Palette exist: %s\n", (magic == PCX_COLORMAP_MAGIC) ? "Yes" : "No");

        }

    }

    else
    {

        channel_send_fmt1(0, source, EVENT_ERROR, "Path not found: %s\n", mdata);

    }

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);

}

