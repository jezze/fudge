#include <fudge.h>
#include <abi.h>
#include <compression.h>
#include <image.h>

#define ROWSIZE                         4096
#define TEXTSIZE                        0x4000
#define TYPE_PCX                        0
#define TYPE_PNG                        1
#define TYPE_TOFFEE                     2
#define TYPE_JPEG                       3

static char source[1024];
static unsigned int paths;
static unsigned int filetarget;
static unsigned int fileid;
static unsigned int width;
static unsigned int height;
static unsigned int *pixels;
static struct png png;
static struct jpeg jpeg;
static unsigned char colormap[768];
static unsigned char rawline[ROWSIZE * 2];
static unsigned char line[ROWSIZE];
static struct toffee toffee;
static unsigned int palette[TOFFEE_SLOTS];
static char text[TEXTSIZE];

static unsigned int readpng(struct png *png, void *buffer, unsigned int count, unsigned int offset)
{

    return fs_read_full(1, filetarget, fileid, buffer, count, offset);

}

static unsigned int openpng(void)
{

    if (!png_open(&png, readpng))
        return 0;

    width = png.width;
    height = png.height;

    return 1;

}

static unsigned int decodepng(void)
{

    unsigned int y;

    for (y = 0; y < height; y++)
    {

        if (!png_readrow(&png, pixels + y * width))
            return 0;

    }

    return 1;

}

static unsigned int readjpeg(struct jpeg *jpeg, void *buffer, unsigned int count, unsigned int offset)
{

    return fs_read_full(1, filetarget, fileid, buffer, count, offset);

}

static unsigned int openjpeg(void)
{

    /* a progressive jpeg needs memory for its coefficients that this program does not have */
    if (!jpeg_open(&jpeg, readjpeg) || jpeg.needed)
        return 0;

    width = jpeg.width;
    height = jpeg.height;

    return 1;

}

static unsigned int decodejpeg(void)
{

    unsigned int y;

    for (y = 0; y < height; y++)
    {

        if (!jpeg_readrow(&jpeg, pixels + y * width))
            return 0;

    }

    return 1;

}

static unsigned int openpcx(struct pcx_header *header)
{

    if (fs_read_full(1, filetarget, fileid, header, sizeof (struct pcx_header), 0) != sizeof (struct pcx_header))
        return 0;

    pcx_convertheader(header);

    if (header->identifier == PCX_IDENTIFIER && header->version == PCX_VERSION && header->encoding == PCX_ENCODING && header->bpp == 8 && header->nplanes == 1 && (unsigned short)header->bpl <= ROWSIZE && (unsigned short)header->xend - (unsigned short)header->xstart < (unsigned short)header->bpl)
    {

        struct record record;

        buffer_clear(colormap, 768);
        buffer_copy(colormap, header->palette, 48);

        if (fs_stat(1, filetarget, fileid, &record) && record.size > sizeof (struct pcx_header) + 768 + 1)
        {

            unsigned char magic;

            fs_read_full(1, filetarget, fileid, &magic, 1, record.size - 768 - 1);

            if (magic == PCX_COLORMAP_MAGIC)
                fs_read_full(1, filetarget, fileid, colormap, 768, record.size - 768);

        }

        width = (unsigned short)header->xend - (unsigned short)header->xstart + 1;
        height = (unsigned short)header->yend - (unsigned short)header->ystart + 1;

        return 1;

    }

    return 0;

}

static unsigned int decodepcx(struct pcx_header *header)
{

    unsigned int bpl = (unsigned short)header->bpl;
    unsigned int offset = sizeof (struct pcx_header);
    unsigned int x;
    unsigned int y;

    for (y = 0; y < height; y++)
    {

        unsigned int *row = pixels + y * width;

        fs_read_full(1, filetarget, fileid, rawline, bpl * 2, offset);

        offset += pcx_readline(rawline, bpl, line);

        for (x = 0; x < width; x++)
        {

            unsigned char *color = colormap + line[x] * 3;

            row[x] = 0xFF000000 | (color[0] << 16) | (color[1] << 8) | color[2];

        }

    }

    return 1;

}

static unsigned int opentoffee(void)
{

    if (!toffee_parse(&toffee, text, fs_read_full(1, filetarget, fileid, text, TEXTSIZE, 0)))
        return 0;

    width = option_getdecimal("width");
    height = option_getdecimal("height");

    if (width > TOFFEE_SIZE || height > TOFFEE_SIZE)
        return 0;

    if (!width && !height)
    {

        width = toffee.width * option_getdecimal("scale") / 100;
        height = toffee.height * option_getdecimal("scale") / 100;

    }

    else if (!width)
    {

        width = height * toffee.width / toffee.height;

    }

    else if (!height)
    {

        height = width * toffee.height / toffee.width;

    }

    return width && height && width <= TOFFEE_SIZE && height <= TOFFEE_SIZE;

}

static unsigned int blend(unsigned int bg, unsigned int color)
{

    unsigned int a = color >> 24;
    unsigned int ba = (bg >> 24) * (255 - a) / 255;
    unsigned int oa = a + ba;
    unsigned int result = oa << 24;
    unsigned int shift;

    if (!oa)
        return 0;

    for (shift = 0; shift < 24; shift += 8)
        result |= ((((color >> shift) & 0xFF) * a + ((bg >> shift) & 0xFF) * ba) / oa) << shift;

    return result;

}

static void readpalette(void)
{

    char *value = option_getstring("palette");
    unsigned int length = cstring_length(value);
    unsigned int offset = 0;
    unsigned int i;

    for (i = 0; i < TOFFEE_SLOTS; i++)
    {

        unsigned int count = buffer_findbyte(value + offset, length - offset, ' ');

        palette[i] = (count) ? cstring_read_value(value + offset, count, 16) : 0;
        offset += (offset + count < length) ? count + 1 : count;

    }

}

static unsigned int decodetoffee(void)
{

    unsigned int y;

    readpalette();

    for (y = 0; y < height; y++)
    {

        unsigned int *row = pixels + y * width;
        unsigned int i;

        buffer_clear(row, width * 4);

        for (i = 0; i < toffee.nshapes; i++)
        {

            struct toffee_shape *shape = &toffee.shapes[i];
            unsigned int color = (shape->slot < TOFFEE_SLOTS) ? palette[shape->slot] : shape->color;
            struct toffee_line scan;
            unsigned int j;

            toffee_scanline(&toffee, &scan, i, width, height, y);

            for (j = 0; j < scan.count; j++)
            {

                int x;

                for (x = scan.spans[j].x0; x < scan.spans[j].x2; x++)
                    row[x] = blend(row[x], color);

            }

        }

    }

    return 1;

}

static unsigned int gettype(char *path)
{

    unsigned int length = cstring_length(path);

    if (length > 4 && cstring_match(path + length - 4, ".png"))
        return TYPE_PNG;

    if (length > 4 && cstring_match(path + length - 4, ".tof"))
        return TYPE_TOFFEE;

    if ((length > 4 && cstring_match(path + length - 4, ".jpg")) || (length > 5 && cstring_match(path + length - 5, ".jpeg")))
        return TYPE_JPEG;

    return TYPE_PCX;

}

static unsigned int open(unsigned int type, struct pcx_header *header)
{

    switch (type)
    {

    case TYPE_PNG:
        return openpng();

    case TYPE_TOFFEE:
        return opentoffee();

    case TYPE_JPEG:
        return openjpeg();

    }

    return openpcx(header);

}

static unsigned int decode(unsigned int type, struct pcx_header *header)
{

    switch (type)
    {

    case TYPE_PNG:
        return decodepng();

    case TYPE_TOFFEE:
        return decodetoffee();

    case TYPE_JPEG:
        return decodejpeg();

    }

    return decodepcx(header);

}

static unsigned int findtarget(unsigned int target, char *path)
{

    unsigned int id = fs_walk(1, target, 0, path);

    if (!id)
    {

        char *name = path + fs_dirlength(path);
        unsigned int parent;

        parent = fs_walkparent(1, target, path);

        return (parent && cstring_length(name)) ? fs_create(1, target, parent, name, cstring_length(name)) : 0;

    }

    return id;

}

static unsigned int fit(unsigned int target, unsigned int id)
{

    if (width && height && width <= 0x4000 && height <= 0x4000)
    {

        unsigned char zero = 0;

        return fs_write(1, target, id, &zero, 1, width * height * 4 - 1) == 1;

    }

    return 0;

}

static void convert(unsigned int reply, char *path, char *to)
{

    unsigned int type = gettype(path);
    unsigned int target = fs_auth(to);
    struct pcx_header header;
    unsigned int id;

    filetarget = fs_auth(path);
    fileid = (filetarget) ? fs_walk(1, filetarget, 0, path) : 0;

    if (!fileid)
    {

        channel_send_fmt(0, reply, MESSAGE_TYPE_ERROR, "Path not found: %s\n", path);

        return;

    }

    if (!open(type, &header))
    {

        channel_send_fmt(0, reply, MESSAGE_TYPE_ERROR, "Not an image: %s\n", path);

        return;

    }

    id = (target) ? findtarget(target, to) : 0;

    if (!id || !fit(target, id))
    {

        channel_send_fmt(0, reply, MESSAGE_TYPE_ERROR, "Image does not fit: %s\n", to);

        return;

    }

    pixels = fs_mmap(1, target, id, 0);

    if (!pixels)
    {

        channel_send_fmt(0, reply, MESSAGE_TYPE_ERROR, "File could not be mapped: %s\n", to);

        return;

    }

    if (decode(type, &header))
        channel_send_fmt(0, reply, MESSAGE_TYPE_DATA, "width \"%u\" height \"%u\"\n", &width, &height);
    else
        channel_send_fmt(0, reply, MESSAGE_TYPE_ERROR, "Image could not be decoded: %s\n", path);

    fs_munmap(1, target, id);

}

static void onpath(struct message *message)
{

    if (!paths)
        buffer_write(source, 1024, message->data, cstring_length_zero(message->data), 0);
    else if (paths == 1)
        convert(message->source, source, message->data);

    paths++;

}

static void onterm(struct message *message)
{

    if (paths != 2)
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "Usage: mapimage [-width <n>] [-height <n>] [-scale <percent>] [-palette <colors>] <image> <file>\n");

}

void init(void)
{

    option_add("width", "0");
    option_add("height", "0");
    option_add("scale", "100");
    option_add("palette", "FF101010 FF202020 FF000000 FFF0F0F0 FFE0E0E0 FFFFFFFF");
    channel_bind(MESSAGE_TYPE_PATH, onpath);
    channel_bind(MESSAGE_TYPE_TERM, onterm);

}

