#include <fudge.h>
#include <abi.h>
#include "cpu.h"
#include "video.h"

static unsigned char rom[0x80000];
static unsigned char cart_ram[0x20000];
static unsigned int *framebuffer;
static unsigned int w, h, scalew, scaleh, totw, toth, offx, offy;
static struct keys keys;

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

static void gb_error(struct gb_s *gb, const enum gb_error_e gb_err, const unsigned short val)
{

    /* EXIT */

}

static void keypress(struct gb_s *gb, void *data)
{

    struct event_keypress *keypress = data;
    unsigned int id = keys_getcode(&keys, keypress->scancode);

    switch (id)
    {

    case KEYS_KEY_CURSORUP:
        gb->direct.joypad_bits.up = 0;

        break;

    case KEYS_KEY_CURSORLEFT:
        gb->direct.joypad_bits.left = 0;

        break;

    case KEYS_KEY_CURSORRIGHT:
        gb->direct.joypad_bits.right = 0;

        break;

    case KEYS_KEY_CURSORDOWN:
        gb->direct.joypad_bits.down = 0;

        break;

    case KEYS_KEY_ENTER:
        gb->direct.joypad_bits.start = 0;

        break;

    case KEYS_KEY_BACKSPACE:
        gb->direct.joypad_bits.select = 0;

        break;

    case KEYS_KEY_Z:
        gb->direct.joypad_bits.a = 0;

        break;

    case KEYS_KEY_X:
        gb->direct.joypad_bits.b = 0;

        break;

    case KEYS_KEY_ESCAPE:
        channel_send(option_getdecimal("wm-service"), EVENT_WMUNMAP);
        channel_close();

        break;

    }

}

static void keyrelease(struct gb_s *gb, void *data)
{

    struct event_keyrelease *keyrelease = data;
    unsigned int id = keys_getcode(&keys, keyrelease->scancode);

    switch (id)
    {

    case KEYS_KEY_CURSORUP:
        gb->direct.joypad_bits.up = 1;

        break;

    case KEYS_KEY_CURSORLEFT:
        gb->direct.joypad_bits.left = 1;

        break;

    case KEYS_KEY_CURSORRIGHT:
        gb->direct.joypad_bits.right = 1;

        break;

    case KEYS_KEY_CURSORDOWN:
        gb->direct.joypad_bits.down = 1;

        break;

    case KEYS_KEY_ENTER:
        gb->direct.joypad_bits.start = 1;

        break;

    case KEYS_KEY_BACKSPACE:
        gb->direct.joypad_bits.select = 1;

        break;

    case KEYS_KEY_Z:
        gb->direct.joypad_bits.a = 1;

        break;

    case KEYS_KEY_X:
        gb->direct.joypad_bits.b = 1;

        break;

    }

}

static void run(void)
{

    char romname[16];
    enum gb_init_error_e gb_ret;
    struct gb_s gb;
    struct message message;
    char data[MESSAGE_SIZE];

    call_read(FILE_G5, rom, 0x80000, 0);

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

    if (getsavesize(&gb))
        call_read(FILE_G5, cart_ram, getsavesize(&gb), 0x80000);

    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "ROM: %s\n", getromname(&gb, romname));

    while (channel_pick(&message, data))
    {

        switch (message.event)
        {

        case EVENT_TIMERTICK:
            gb_run_frame(&gb);

            if (framebuffer)
                video_render(framebuffer, w, scalew, scaleh, totw, toth, offx, offy);

            break;

        case EVENT_KEYPRESS:
            keypress(&gb, data);

            break;

        case EVENT_KEYRELEASE:
            keyrelease(&gb, data);

            break;

        default:
            channel_dispatch(&message, data);

            break;

        }

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (!call_walk_absolute(FILE_L0, option_getstring("keyboard")))
        PANIC();

    if (!call_walk_relative(FILE_G0, FILE_L0, "event"))
        PANIC();

    if (!call_walk_absolute(FILE_L0, option_getstring("timer")))
        PANIC();

    if (!call_walk_relative(FILE_G1, FILE_L0, "event1"))
        PANIC();

    if (!call_walk_absolute(FILE_L0, option_getstring("video")))
        PANIC();

    if (!call_walk_relative(FILE_G2, FILE_L0, "event"))
        PANIC();

    if (!call_walk_relative(FILE_G3, FILE_L0, "ctrl"))
        PANIC();

    channel_send(option_getdecimal("wm-service"), EVENT_WMMAP);

    while (channel_process());

    channel_send(option_getdecimal("wm-service"), EVENT_WMUNMAP);

}

static void onvideomode(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_videomode *videomode = mdata;

    framebuffer = videomode->framebuffer;
    w = videomode->w;
    h = videomode->h;
    scalew = videomode->w / LCD_WIDTH;
    scaleh = videomode->h / LCD_HEIGHT;
    totw = LCD_WIDTH * scalew;
    toth = LCD_HEIGHT * scaleh;
    offx = (w - totw) / 2;
    offy = (h - toth) / 2;

}

static void onwminit(unsigned int source, void *mdata, unsigned int msize)
{

    struct ctrl_videosettings settings;

    settings.width = option_getdecimal("width");
    settings.height = option_getdecimal("height");
    settings.bpp = option_getdecimal("bpp");

    channel_send(option_getdecimal("wm-service"), EVENT_WMGRAB);
    channel_wait_any(EVENT_WMACK);
    call_link(FILE_G0, 8000);
    call_link(FILE_G1, 8001);
    call_link(FILE_G2, 8002);
    call_notify(FILE_G3, EVENT_CONFIG, sizeof (struct ctrl_videosettings), &settings);
    run();
    call_unlink(FILE_G2);
    call_unlink(FILE_G1);
    call_unlink(FILE_G0);
    channel_send(option_getdecimal("wm-service"), EVENT_WMUNGRAB);
    channel_wait_any(EVENT_WMACK);

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    if (call_walk_absolute(FILE_L0, mdata))
        call_walk_duplicate(FILE_G5, FILE_L0);
    else
        channel_send_fmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Path not found: %s\n", mdata);

}

void init(void)
{

    keys_init(&keys, KEYS_LAYOUT_QWERTY_US, KEYS_MAP_US);
    option_add("width", "1024");
    option_add("height", "768");
    option_add("bpp", "4");
    option_add("keyboard", "system:keyboard");
    option_add("timer", "system:timer/if.0");
    option_add("video", "system:video/if.0");
    option_add("wm-service", "12345");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_VIDEOMODE, onvideomode);
    channel_bind(EVENT_WMINIT, onwminit);

}

