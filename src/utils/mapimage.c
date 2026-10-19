#include <fudge.h>
#include <abi.h>
#include <compression.h>
#include <image.h>

#define ROWSIZE                         4096

static char source[1024];
static unsigned int paths;
static unsigned int filetarget;
static unsigned int fileid;
static unsigned int width;
static unsigned int height;
static unsigned int *pixels;
static struct png png;
static unsigned char colormap[768];
static unsigned char rawline[ROWSIZE * 2];
static unsigned char line[ROWSIZE];

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

static unsigned int openpcx(struct pcx_header *header)
{

    struct record record;

    if (fs_read_all(1, filetarget, fileid, header, sizeof (struct pcx_header), 0) != sizeof (struct pcx_header))
        return 0;

    pcx_convertheader(header);

    if (header->identifier != PCX_IDENTIFIER || header->version != PCX_VERSION || header->encoding != PCX_ENCODING || header->bpp != 8 || header->nplanes != 1 || (unsigned short)header->bpl > ROWSIZE || (unsigned short)header->xend - (unsigned short)header->xstart >= (unsigned short)header->bpl)
        return 0;

    buffer_clear(colormap, 768);
    buffer_copy(colormap, header->palette, 48);

    if (fs_stat(1, filetarget, fileid, &record) && record.size > sizeof (struct pcx_header) + 768 + 1)
    {

        unsigned char magic;

        fs_read_all(1, filetarget, fileid, &magic, 1, record.size - 768 - 1);

        if (magic == PCX_COLORMAP_MAGIC)
            fs_read_all(1, filetarget, fileid, colormap, 768, record.size - 768);

    }

    width = (unsigned short)header->xend - (unsigned short)header->xstart + 1;
    height = (unsigned short)header->yend - (unsigned short)header->ystart + 1;

    return 1;

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

static unsigned int findtarget(unsigned int target, char *path)
{

    unsigned int id = fs_walk(1, target, 0, path);
    char *name = path + fs_dirlength(path);
    unsigned int parent;

    if (id)
        return id;

    parent = fs_walkparent(1, target, path);

    return (parent && cstring_length(name)) ? fs_create(1, target, parent, name, cstring_length(name)) : 0;

}

static unsigned int fit(unsigned int target, unsigned int id)
{

    unsigned char zero = 0;

    if (!width || !height || width > 0x4000 || height > 0x4000)
        return 0;

    return fs_write(1, target, id, &zero, 1, width * height * 4 - 1) == 1;

}

static void convert(unsigned int reply, char *path, char *to)
{

    unsigned int length = cstring_length(path);
    unsigned int ispng = length > 4 && cstring_match(path + length - 4, ".png");
    unsigned int target = fs_auth(to);
    struct pcx_header header;
    unsigned int id;

    filetarget = fs_auth(path);
    fileid = (filetarget) ? fs_walk(1, filetarget, 0, path) : 0;

    if (!fileid)
    {

        channel_send_fmt(0, reply, EVENT_ERROR, "Path not found: %s\n", path);

        return;

    }

    if (!(ispng ? openpng() : openpcx(&header)))
    {

        channel_send_fmt(0, reply, EVENT_ERROR, "Not an image: %s\n", path);

        return;

    }

    id = (target) ? findtarget(target, to) : 0;

    if (!id || !fit(target, id))
    {

        channel_send_fmt(0, reply, EVENT_ERROR, "Image does not fit: %s\n", to);

        return;

    }

    pixels = fs_mmap(1, target, id, 0);

    if (!pixels)
    {

        channel_send_fmt(0, reply, EVENT_ERROR, "File could not be mapped: %s\n", to);

        return;

    }

    if (ispng ? decodepng() : decodepcx(&header))
        channel_send_fmt(0, reply, EVENT_DATA, "width \"%u\" height \"%u\"\n", &width, &height);
    else
        channel_send_fmt(0, reply, EVENT_ERROR, "Image could not be decoded: %s\n", path);

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
        channel_send_fmt(0, message->source, EVENT_ERROR, "Usage: mapimage <image> <file>\n");

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_TERM, onterm);

}

