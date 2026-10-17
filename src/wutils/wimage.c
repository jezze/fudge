#include <fudge.h>
#include <abi.h>
#include <compression.h>
#include <image.h>
#include "kv.h"

#define NUM_MODES                       2
#define NUM_NAMES                       16
#define ROWSIZE                         4096

static char path[256];
static unsigned int wm;
static char *labels[NUM_MODES] = {"Normal", "Stretched"};
static char *flows[NUM_MODES] = {"vertical", "stretch"};
static unsigned int mode;
static unsigned int memtarget;
static unsigned int memparent;
static unsigned int memid;
static char memname[32];
static unsigned int *pixels;
static unsigned int filetarget;
static unsigned int fileid;
static unsigned int width;
static unsigned int height;
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

        if (!png_readrow(&png, y, pixels + y * width))
            return 0;

    }

    return 1;

}

static unsigned int openpcx(struct pcx_header *header)
{

    struct record record;

    if (fs_read_all(1, filetarget, fileid, header, sizeof (struct pcx_header), 0) != sizeof (struct pcx_header))
        return 0;

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

/* the mem: file grows to hold width times height pixels; a write past its end is refused */
static unsigned int fit(void)
{

    unsigned int size = width * height * 4;
    unsigned char zero = 0;

    if (!width || !height || width > 0x4000 || height > 0x4000)
        return 0;

    return fs_write(1, memtarget, memid, &zero, 1, size - 1) == 1;

}

/* decodes the file at path into the mem: file */
static unsigned int decode(void)
{

    unsigned int length = cstring_length(path);
    struct pcx_header header;

    filetarget = fs_auth(path);
    fileid = (filetarget) ? fs_walk(1, filetarget, 0, path) : 0;

    if (!fileid || !pixels)
        return 0;

    if (length > 4 && cstring_match(path + length - 4, ".png"))
        return openpng() && fit() && decodepng();
    else
        return openpcx(&header) && fit() && decodepcx(&header);

}

/* a file of our own in mem:, named wimage and the first free number */
static void createmem(void)
{

    unsigned int i;

    memtarget = fs_auth("mem:");
    memparent = (memtarget) ? fs_walk(1, memtarget, 0, "mem:") : 0;

    for (i = 0; i < NUM_NAMES && memparent && !memid; i++)
    {

        cstring_write_fmt(memname, 32, 0, "wimage%u\\0", &i);

        memid = fs_create(1, memtarget, memparent, memname, cstring_length(memname));

    }

    if (memid)
        pixels = fs_mmap(1, memtarget, memid);

}

static void showimage(void)
{

    if (!wm || !path[0])
        return;

    channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "= window label \"Image Viewer (%s)\"\n= frame flow \"%s\"\n", path + fs_dirlength(path), flows[mode]);

    if (decode())
        channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "+ image id \"image\" in \"frame\" mimetype \"image/raw\" width \"%u\" height \"%u\" source \"mem:%s\"\n", &width, &height, memname);

}

static void choose(void)
{

    unsigned int target = fs_spawn(1, 0, "initrd:bin/wfile");

    if (target)
    {

        channel_send_fmt(0, target, EVENT_OPTION, "mode\\0select\\0wm-service\\0%s\\0", option_getstring("wm-service"));
        channel_send(0, target, EVENT_MAIN, 0, 0);
        channel_send(0, target, EVENT_TERM, 0, 0);

    }

}

static void onmain(struct message *message)
{

    wm = channel_lookup(option_getstring("wm-service"));

    if (wm)
    {

        char *alfi = "initrd:data/alfi/wimage.alfi";

        createmem();
        channel_send(0, wm, EVENT_WMRENDERFILE, cstring_length_zero(alfi), alfi);
        showimage();
        channel_hold(0);
        channel_send(0, wm, EVENT_WMUNMAP, 0, 0);

        if (memid)
            fs_remove(1, memtarget, memparent, memid);

    }

}

static void onpath(struct message *message)
{

    if (path[0])
        channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "- image\n");

    cstring_write_fmt(path, 256, 0, "%s\\0", message->data);
    showimage();

}

static void onwmevent(struct message *message)
{

    struct event_wmevent *event = message->data;

    if (kv_match(event, "q=mode"))
    {

        char *label = kv_getstring(event, "mode=");
        unsigned int i;

        for (i = 0; i < NUM_MODES; i++)
        {

            if (label && cstring_match(label, labels[i]))
                mode = i;

        }

        channel_send_fmt(0, message->source, EVENT_WMRENDERDATA, "= mode label \"%s\"\n= frame flow \"%s\"\n", labels[mode], flows[mode]);

    }

    if (kv_match(event, "q=open"))
        choose();

}

void init(void)
{

    option_add("wm-service", "wm");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_WMEVENT, onwmevent);

}

