#include <fudge.h>
#include <abi.h>
#include "cpu.h"
#include "video.h"

static unsigned char rom[0x80000];
static unsigned char cart_ram[0x20000];
static unsigned char *framebuffer;
static unsigned int w, h, scalew, scaleh, totw, toth, offx, offy;
static unsigned int escaped;

static unsigned char gb_rom_read(struct gb_s *gb, const unsigned int addr)
{

    return rom[addr];

}

static unsigned char gb_cart_ram_read(struct gb_s *gb, const unsigned int addr)
{

    return cart_ram[addr];

}

static void gb_cart_ram_write(struct gb_s *gb, const unsigned int addr, const unsigned char val)
{

    cart_ram[addr] = val;

}

static unsigned int getsavesize(struct gb_s *gb)
{

    unsigned int ram_sizes[] = { 0x00, 0x800, 0x2000, 0x8000, 0x20000 };
    unsigned char ram_size = gb->gb_rom_read(gb, 0x0149);

    return ram_sizes[ram_size];

}

static char *getromname(struct gb_s* gb, char title_str[16])
{

    unsigned int title_loc = 0x134;
    unsigned int title_end = 0x143;
    char *title_start = title_str;

    for (; title_loc <= title_end; title_loc++)
    {

        char title_char = gb->gb_rom_read(gb, title_loc);

        if (title_char >= ' ' && title_char <= '_')
        {

            *title_str = title_char;
            title_str++;

        }

        else
            break;

    }

    *title_str = '\0';

    return title_start;

}

static unsigned char *read_rom_to_ram(char *filename)
{

    if (file_walk2(FILE_L0, filename))
        file_read(FILE_L0, rom, 0x80000);

    return rom;

}

static void read_cart_ram_file(char *filename, unsigned int len)
{

    if (file_walk2(FILE_L0, filename))
        file_read(FILE_L0, cart_ram, len);

}

static void gb_error(struct gb_s *gb, const enum gb_error_e gb_err, const unsigned short val)
{

    /* EXIT */

}

static void render(void)
{

    if (framebuffer)
    {

        void *panel = video_getfb();
        unsigned int y;
        unsigned short *fp = (unsigned short *)framebuffer;
        unsigned short *pp = (unsigned short *)panel;

        for (y = 0; y < toth; y++)
        {

            unsigned int x;

            for (x = 0; x < totw; x++)
            {

                unsigned short *p = fp + ((y + offy) * w) + (x + offx);
                unsigned short *t = pp + ((y / scaleh) * 160) + x / scalew;

                *p = *t;

            }

        }

    }

}

static void keypress(struct gb_s *gb, void *data)
{

    struct event_keypress *keypress = data;

    if (escaped)
    {

        switch (keypress->scancode)
        {

        case 0x48:
            gb->direct.joypad_bits.up = 0;

            break;

        case 0x4B:
            gb->direct.joypad_bits.left = 0;

            break;

        case 0x4D:
            gb->direct.joypad_bits.right = 0;

            break;

        case 0x50:
            gb->direct.joypad_bits.down = 0;

            break;

        }

        escaped = 0;

    }

    else
    {

        switch (keypress->scancode)
        {

        case 0x01:
            channel_send(EVENT_WMUNGRAB);
            channel_send(EVENT_WMUNMAP);
            channel_close();

            break;

        case 0x1C:
            gb->direct.joypad_bits.start = 0;

            break;

        case 0x0E:
            gb->direct.joypad_bits.select = 0;

            break;

        case 0x2C:
            gb->direct.joypad_bits.a = 0;

            break;

        case 0x2D:
            gb->direct.joypad_bits.b = 0;

            break;

        }

    }

}

static void keyrelease(struct gb_s *gb, void *data)
{

    struct event_keyrelease *keyrelease = data;

    if (escaped)
    {

        switch (keyrelease->scancode)
        {

        case 0x48 | 0x80:
            gb->direct.joypad_bits.up = 1;

            break;

        case 0x4B | 0x80:
            gb->direct.joypad_bits.left = 1;

            break;

        case 0x4D | 0x80:
            gb->direct.joypad_bits.right = 1;

            break;

        case 0x50 | 0x80:
            gb->direct.joypad_bits.down = 1;

            break;

        }

        escaped = 0;

    }

    else
    {

        switch (keyrelease->scancode)
        {

        case 0x1C | 0x80:
            gb->direct.joypad_bits.start = 1;

            break;

        case 0x0E | 0x80:
            gb->direct.joypad_bits.select = 1;

            break;

        case 0x2C | 0x80:
            gb->direct.joypad_bits.a = 1;

            break;

        case 0x2D | 0x80:
            gb->direct.joypad_bits.b = 1;

            break;

        case 0xE0:
            escaped = 1;

            break;

        }

    }

}

static void run(void)
{

    char *romfile = "/data/pocket.gb";
    char romname[16];
    enum gb_init_error_e gb_ret;
    struct gb_s gb;
    struct message message;

    read_rom_to_ram(romfile);

    gb_ret = gb_init(&gb, &gb_rom_read, &gb_cart_ram_read, &gb_cart_ram_write, &gb_error);

    switch (gb_ret)
    {

    case GB_INIT_NO_ERROR:
        break;

    case GB_INIT_CARTRIDGE_UNSUPPORTED:
        break;

    case GB_INIT_INVALID_CHECKSUM:
        break;

    default:
        break;

    }

    read_cart_ram_file(romfile, getsavesize(&gb));
    channel_sendstring(EVENT_DATA, "ROM: ");
    channel_sendstring(EVENT_DATA, getromname(&gb, romname));
    channel_sendstring(EVENT_DATA, "\n");
    video_setpalette();

    while (channel_pick(&message))
    {

        switch (message.header.event)
        {

        case EVENT_TIMERTICK:
            gb_run_frame(&gb);
            render();

            break;

        case EVENT_KEYPRESS:
            keypress(&gb, message.data.buffer);

            break;

        case EVENT_KEYRELEASE:
            keyrelease(&gb, message.data.buffer);

            break;

        default:
            channel_dispatch(&message);

            break;

        }

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, "system:service/wm"))
        file_notify(FILE_L0, EVENT_WMMAP, 0, 0);
    else
        channel_warning("Could not open window manager service");

}

static void onterm(unsigned int source, void *mdata, unsigned int msize)
{

    channel_send(EVENT_WMUNGRAB);
    channel_send(EVENT_WMUNMAP);
    channel_close();

}

static void onvideomode(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_videomode *videomode = mdata;

    framebuffer = videomode->framebuffer;
    w = videomode->w;
    h = videomode->h;
    scalew = videomode->w / 160;
    scaleh = videomode->h / 144;
    totw = 160 * scalew;
    toth = 144 * scaleh;
    offx = (w - totw) / 2;
    offy = (h - toth) / 2;

}

static void onwminit(unsigned int source, void *mdata, unsigned int msize)
{

    struct ctrl_videosettings settings;

    if (!file_walk2(FILE_L0, option_getstring("keyboard")))
        channel_warning("Could not open keyboard");

    if (!file_walk(FILE_G0, FILE_L0, "event"))
        channel_warning("Could not open keyboard event");

    if (!file_walk2(FILE_L0, option_getstring("video")))
        channel_error("Could not find video device");

    if (!file_walk(FILE_L1, FILE_L0, "ctrl"))
        channel_error("Could not find video device ctrl");

    if (!file_walk(FILE_G1, FILE_L0, "event"))
        channel_warning("Could not open video event");

    if (!file_walk2(FILE_L0, option_getstring("timer")))
        channel_error("Could not find timer device");

    if (!file_walk(FILE_G2, FILE_L0, "event1"))
        channel_warning("Could not open timer event");

    channel_send(EVENT_WMGRAB);

    settings.width = option_getdecimal("width");
    settings.height = option_getdecimal("height");
    settings.bpp = option_getdecimal("bpp");

    file_link(FILE_G0);
    file_link(FILE_G1);
    file_link(FILE_G2);
    file_seekwriteall(FILE_L1, &settings, sizeof (struct ctrl_videosettings), 0);
    run();
    file_unlink(FILE_G2);
    file_unlink(FILE_G1);
    file_unlink(FILE_G0);

}

void init(void)
{

    option_add("width", "1024");
    option_add("height", "768");
    option_add("bpp", "2");
    option_add("keyboard", "system:keyboard");
    option_add("timer", "system:timer/if:0");
    option_add("video", "system:video/if:0");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_TERM, onterm);
    channel_bind(EVENT_VIDEOMODE, onvideomode);
    channel_bind(EVENT_WMINIT, onwminit);

}

