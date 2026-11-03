#include <fudge.h>
#include <abi.h>
#include "cpu.h"
#include "audio.h"
#include "video.h"

#define SCREENS                         16
#define SCREENSIZE                      (LCD_WIDTH * LCD_HEIGHT * 4)

static unsigned char rom[0x80000];
static unsigned char cart_ram[0x20000];
static unsigned int *framebuffer;
static unsigned int w, h, scalew, scaleh, totw, toth, offx, offy;
static struct keys keys;
static char path[128];
static struct gb_s gb;
static unsigned int audio;
static unsigned int video;
static unsigned int keyboard;
static unsigned int timer;
static unsigned int wm;
static unsigned int fullscreen;
static unsigned int wanted;
static unsigned int flushing;
static unsigned int dirty;
static unsigned int screentarget;
static unsigned int screenid;
static unsigned int *screen;
static char screenname[32];
static short sound[AUDIO_SAMPLES * 2];

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

/* one flush at a time is on its way to the wm; a frame shown meanwhile goes out when it answers */
static void flush(void)
{

    char *id = "screen";

    if (flushing)
    {

        dirty = 1;

        return;

    }

    channel_send(0, wm, MESSAGE_TYPE_WMFLUSH, cstring_length_zero(id), id);

    flushing = 1;
    dirty = 0;

}

static void setbutton(unsigned int id, unsigned int pressed)
{

    unsigned int bit = !pressed;

    switch (id)
    {

    case KEYS_KEY_CURSORUP:
        gb.direct.joypad_bits.up = bit;

        break;

    case KEYS_KEY_CURSORLEFT:
        gb.direct.joypad_bits.left = bit;

        break;

    case KEYS_KEY_CURSORRIGHT:
        gb.direct.joypad_bits.right = bit;

        break;

    case KEYS_KEY_CURSORDOWN:
        gb.direct.joypad_bits.down = bit;

        break;

    case KEYS_KEY_ENTER:
        gb.direct.joypad_bits.start = bit;

        break;

    case KEYS_KEY_BACKSPACE:
        gb.direct.joypad_bits.select = bit;

        break;

    case KEYS_KEY_Z:
        gb.direct.joypad_bits.a = bit;

        break;

    case KEYS_KEY_X:
        gb.direct.joypad_bits.b = bit;

        break;

    }

}

static void ontimertick(struct message *message)
{

    gb_run_frame(&gb);

    if (fullscreen && framebuffer)
    {

        struct message_videoflush videoflush;

        video_render(framebuffer, w, scalew, scaleh, totw, toth, offx, offy);

        videoflush.x = offx;
        videoflush.y = offy;
        videoflush.width = totw;
        videoflush.height = toth;

        channel_send(0, video, MESSAGE_TYPE_VIDEOFLUSH, sizeof (struct message_videoflush), &videoflush);

    }

    if (!fullscreen && screen)
    {

        video_render(screen, LCD_WIDTH, 1, 1, LCD_WIDTH, LCD_HEIGHT, 0, 0);
        flush();

    }

    if (audio)
    {

        audio_render(sound);
        channel_send(1, audio, MESSAGE_TYPE_DATA, sizeof (sound), sound);

    }

}

static void setupvideo(void)
{

    struct message_videoconf videoconf;
    struct message_videoinfo videoinfo;

    videoconf.width = option_getdecimal("width");
    videoconf.height = option_getdecimal("height");
    videoconf.bpp = option_getdecimal("bpp");

    channel_send(0, video, MESSAGE_TYPE_VIDEOCONF, sizeof (struct message_videoconf), &videoconf);
    channel_send(1, video, MESSAGE_TYPE_INFO, 0, 0);

    if (channel_wait(1, video, MESSAGE_TYPE_VIDEOINFO, sizeof (struct message_videoinfo), &videoinfo) && videoinfo.bpp == 4)
    {

        framebuffer = (unsigned int *)(unsigned long)videoinfo.framebuffer;
        w = videoinfo.width;
        h = videoinfo.height;
        scalew = videoinfo.width / LCD_WIDTH;
        scaleh = videoinfo.height / LCD_HEIGHT;
        totw = LCD_WIDTH * scalew;
        toth = LCD_HEIGHT * scaleh;
        offx = (w - totw) / 2;
        offy = (h - toth) / 2;

    }

}

static void enterfullscreen(void)
{

    if (wm)
    {

        channel_send(1, wm, MESSAGE_TYPE_WMGRAB, 0, 0);
        channel_wait(1, wm, MESSAGE_TYPE_WMACK, 0, 0);

    }

    setupvideo();

    gb.direct.joypad = 0xFF;
    keys.mod = 0;
    fullscreen = 1;
    wanted = 1;

    channel_send(0, keyboard, MESSAGE_TYPE_LINK, 0, 0);

}

static void leavefullscreen(void)
{

    channel_send(0, keyboard, MESSAGE_TYPE_UNLINK, 0, 0);

    gb.direct.joypad = 0xFF;
    framebuffer = 0;
    fullscreen = 0;
    wanted = 0;

    if (wm)
    {

        channel_send(1, wm, MESSAGE_TYPE_WMUNGRAB, 0, 0);
        channel_wait(1, wm, MESSAGE_TYPE_WMACK, 0, 0);

    }

}

/* the grab is answered on channel 1, so channel 0 must have room for what the wm sends before answering: run switches between drains, when its slots are free, and ticks stay away meanwhile */
static void switchscreen(unsigned int on)
{

    channel_send(0, timer, MESSAGE_TYPE_UNLINK, 0, 0);

    if (on)
        enterfullscreen();
    else
        leavefullscreen();

    channel_send(0, timer, MESSAGE_TYPE_LINK, 0, 0);

}

/* a frame runs on each tick */
static void run(void)
{

    while (channel_drain(0))
    {

        if (wanted != fullscreen)
            switchscreen(wanted);

        if (!channel_process(0))
            break;

    }

}

/* Alt+F and Alt+W ask for the screen or the window, when there is a window */
static void press(unsigned int id, unsigned int mod)
{

    if (screen && (mod & KEYS_MOD_ALT) && (id == KEYS_KEY_F || id == KEYS_KEY_W))
        wanted = (id == KEYS_KEY_F);
    else
        setbutton(id, 1);

}

static void onkeypress(struct message *message)
{

    struct message_keypress *keypress = message_validate(MESSAGE_TYPE_KEYPRESS, message->data, message->length);

    if (keypress)
    {

        unsigned int id = keys_getcode(&keys, keypress->scancode);

        press(id, keys.mod);

    }

}

static void onkeyrelease(struct message *message)
{

    struct message_keyrelease *keyrelease = message_validate(MESSAGE_TYPE_KEYRELEASE, message->data, message->length);

    if (keyrelease)
    {

        setbutton(keys_getcode(&keys, keyrelease->scancode), 0);

    }

}

static void onwmkeypress(struct message *message)
{

    struct message_wmkeypress *wmkeypress = message_validate(MESSAGE_TYPE_WMKEYPRESS, message->data, message->length);

    if (wmkeypress)
    {

        if (!fullscreen)
            press(wmkeypress->id, wmkeypress->keymod);

    }

}

static void onwmkeyrelease(struct message *message)
{

    struct message_wmkeyrelease *wmkeyrelease = message_validate(MESSAGE_TYPE_WMKEYRELEASE, message->data, message->length);

    if (wmkeyrelease)
    {

        if (!fullscreen)
            setbutton(wmkeyrelease->id, 0);

    }

}

static void onwmack(struct message *message)
{

    flushing = 0;

    if (dirty && !fullscreen)
        flush();

}

static void load(unsigned int source, unsigned int target, unsigned int id)
{

    char romname[16];

    fs_read_full(1, target, id, rom, 0x80000, 0);
    gb_init(&gb, &gb_rom_read, &gb_cart_ram_read, &gb_cart_ram_write, &gb_error);
    audio_init();

    if (getsavesize(&gb))
        fs_read_full(1, target, id, cart_ram, getsavesize(&gb), 0x80000);

    channel_send_fmt(0, source, MESSAGE_TYPE_DATA, "ROM: %s\n", getromname(&gb, romname));

}

static void setupaudio(void)
{

    unsigned int i;

    audio = channel_lookup(option_getstring("audio-service"));

    if (!audio)
        return;

    /* a few ticks of silence first, so the device has something to play while the next frame is made */
    buffer_clear(sound, sizeof (sound));

    for (i = 0; i < 3; i++)
        channel_send(1, audio, MESSAGE_TYPE_DATA, sizeof (sound), sound);

}

/* a mem: file the wm shows in a window, made as large as the screen by writing its last pixel */
static unsigned int createscreen(void)
{

    unsigned int parent;
    unsigned int zero = 0;
    unsigned int i;

    screentarget = fs_auth("mem:");
    parent = (screentarget) ? fs_walk(1, screentarget, 0, "mem:") : 0;

    for (i = 0; i < SCREENS && parent && !screenid; i++)
    {

        cstring_write_fmt(screenname, 32, 0, "gameboyscreen%u\\0", &i);

        screenid = fs_create(1, screentarget, parent, screenname, cstring_length(screenname));

    }

    if (screenid && fs_write(1, screentarget, screenid, &zero, 4, SCREENSIZE - 4) == 4)
        screen = fs_mmap(1, screentarget, screenid, 0);

    return screen != 0;

}

static void destroyscreen(void)
{

    if (screen)
        fs_munmap(1, screentarget, screenid);

    if (screenid)
        fs_remove(1, screentarget, fs_walk(1, screentarget, 0, "mem:"), screenid);

}

static void showwindow(void)
{

    unsigned int width = LCD_WIDTH;
    unsigned int height = LCD_HEIGHT;

    channel_send_fmt(0, wm, MESSAGE_TYPE_WMRENDERDATA, "+ window id \"window\" label \"Gameboy (%s)\" flow \"stretch\"\n+ media id \"screen\" in \"window\" mimetype \"image/raw\" source \"mem:%s\" width \"%u\" height \"%u\"\n", path + fs_dirlength(path), screenname, &width, &height);

}

static void onpath(struct message *message)
{

    unsigned int target;
    unsigned int id;

    if (path[0])
        return;

    keyboard = channel_lookup(option_getstring("keyboard-service"));
    timer = channel_lookup(option_getstring("timer-service"));
    video = channel_lookup(option_getstring("video-service"));
    wm = channel_lookup(option_getstring("wm-service"));

    buffer_write(path, 128, message->data, message->length, 0);

    if (!keyboard || !timer || !video)
        return;

    target = fs_auth(path);
    id = (target) ? fs_walk(1, target, 0, path) : 0;

    if (!id)
        return;

    setupaudio();
    load(message->source, target, id);
    channel_bind(MESSAGE_TYPE_TIMERTICK, ontimertick);
    channel_bind(MESSAGE_TYPE_KEYPRESS, onkeypress);
    channel_bind(MESSAGE_TYPE_KEYRELEASE, onkeyrelease);
    channel_bind(MESSAGE_TYPE_WMKEYPRESS, onwmkeypress);
    channel_bind(MESSAGE_TYPE_WMKEYRELEASE, onwmkeyrelease);
    channel_bind(MESSAGE_TYPE_WMACK, onwmack);

    if (wm && createscreen())
        showwindow();
    else
        enterfullscreen();

    channel_send(0, timer, MESSAGE_TYPE_LINK, 0, 0);
    run();
    channel_send(0, timer, MESSAGE_TYPE_UNLINK, 0, 0);

    if (fullscreen)
        leavefullscreen();

    if (screen)
        channel_send(0, wm, MESSAGE_TYPE_WMUNMAP, 0, 0);

    destroyscreen();

}

void init(void)
{

    keys_init(&keys, KEYS_LAYOUT_QWERTY_US, (void *)0);
    option_add("width", "1024");
    option_add("height", "768");
    option_add("bpp", "4");
    option_add("keyboard-service", "keyboard:0");
    option_add("timer-service", "timer:0");
    option_add("video-service", "video:0");
    option_add("wm-service", "wm");
    option_add("audio-service", "mixer");
    channel_bind(MESSAGE_TYPE_PATH, onpath);

}

