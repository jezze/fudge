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

    if (file_walk2(FILE_L0, mdata))
    {

        struct pcx_header header;
        struct record record;
        unsigned char magic;
        unsigned int width;
        unsigned int height;

        file_stat(FILE_L0, &record);
        file_readall(FILE_L0, &header, sizeof (struct pcx_header), 0);

        width = header.xend - header.xstart + 1;
        height = header.yend - header.ystart + 1;

        channel_sendfmt1(CHANNEL_DEFAULT, EVENT_DATA, "Identifier: %c\n", &header.identifier);
        channel_sendfmt1(CHANNEL_DEFAULT, EVENT_DATA, "Version: %c\n", &header.version);
        channel_sendfmt1(CHANNEL_DEFAULT, EVENT_DATA, "Encoding: %s\n", (header.encoding < 2) ? encodings[(unsigned int)header.encoding] : "Unknown");
        channel_sendfmt3(CHANNEL_DEFAULT, EVENT_DATA, "Size: %ix%ix%c\n", &width, &height, &header.bpp);
        channel_sendfmt1(CHANNEL_DEFAULT, EVENT_DATA, "Planes: %c\n", &header.nplanes);
        channel_sendfmt1(CHANNEL_DEFAULT, EVENT_DATA, "Palette mode: %s\n", (header.palettemode < 3) ? palettemodes[header.palettemode] : "Unknown");

        if (header.palettemode)
        {

            file_readall(FILE_L0, &magic, 1, record.size - 768 - 1);
            channel_sendfmt1(CHANNEL_DEFAULT, EVENT_DATA, "Palette exist: %s\n", (magic == PCX_COLORMAP_MAGIC) ? "Yes" : "No");

        }

    }

    else
    {

        channel_sendfmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Path not found: %s\n", mdata);

    }

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);

}

