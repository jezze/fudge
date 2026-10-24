#include <fudge.h>
#include <abi.h>
#include "port.h"
#include "sdl.h"
#include "glue.h"

#define SCREENWIDTH                     320
#define SCREENHEIGHT                    200
#define EVENTS                          64
#define TICKRATE                        60
#define AUDIOFRAMES                     800
#define AUDIOCUSHION                    3
#define COUNTSPERTICK                   20

extern int g_argc;
extern char **g_argv;
void pop_main(void);

static struct keys keys;
static SDL_Event events[EVENTS];
static unsigned int eventhead;
static unsigned int eventtail;
static unsigned int ticks;
static unsigned int counterbase;
static unsigned int counting;
static unsigned int audioticks;
static unsigned int modifiers;
static unsigned int closed;
static Uint8 keystate[SDL_NUM_SCANCODES];
static SDL_TimerCallback timercallback;
static void *timerparam;
static Uint32 timerdue;
static unsigned int *framebuffer;
static unsigned int fbwidth;
static unsigned int fbheight;
static unsigned int video;
static unsigned int keyboard;
static unsigned int timer;
static unsigned int audio;
static unsigned int audiopaused = 1;
static SDL_AudioSpec *audiospec;
static short audiobuffer[AUDIOFRAMES * 2];
static unsigned int scaled[4096];
static char *arguments[] = {"pop", 0};

/* scancode set 1 to SDL scancodes, the first half plain keys and the second half 0xE0 keys */
static unsigned short scancodes[256] = {
    [0x01] = SDL_SCANCODE_ESCAPE,
    [0x02] = SDL_SCANCODE_1,
    [0x03] = SDL_SCANCODE_2,
    [0x04] = SDL_SCANCODE_3,
    [0x05] = SDL_SCANCODE_4,
    [0x06] = SDL_SCANCODE_5,
    [0x07] = SDL_SCANCODE_6,
    [0x08] = SDL_SCANCODE_7,
    [0x09] = SDL_SCANCODE_8,
    [0x0A] = SDL_SCANCODE_9,
    [0x0B] = SDL_SCANCODE_0,
    [0x0C] = SDL_SCANCODE_MINUS,
    [0x0D] = SDL_SCANCODE_EQUALS,
    [0x0E] = SDL_SCANCODE_BACKSPACE,
    [0x0F] = SDL_SCANCODE_TAB,
    [0x10] = SDL_SCANCODE_Q,
    [0x11] = SDL_SCANCODE_W,
    [0x12] = SDL_SCANCODE_E,
    [0x13] = SDL_SCANCODE_R,
    [0x14] = SDL_SCANCODE_T,
    [0x15] = SDL_SCANCODE_Y,
    [0x16] = SDL_SCANCODE_U,
    [0x17] = SDL_SCANCODE_I,
    [0x18] = SDL_SCANCODE_O,
    [0x19] = SDL_SCANCODE_P,
    [0x1A] = SDL_SCANCODE_LEFTBRACKET,
    [0x1B] = SDL_SCANCODE_RIGHTBRACKET,
    [0x1C] = SDL_SCANCODE_RETURN,
    [0x1D] = SDL_SCANCODE_LCTRL,
    [0x1E] = SDL_SCANCODE_A,
    [0x1F] = SDL_SCANCODE_S,
    [0x20] = SDL_SCANCODE_D,
    [0x21] = SDL_SCANCODE_F,
    [0x22] = SDL_SCANCODE_G,
    [0x23] = SDL_SCANCODE_H,
    [0x24] = SDL_SCANCODE_J,
    [0x25] = SDL_SCANCODE_K,
    [0x26] = SDL_SCANCODE_L,
    [0x27] = SDL_SCANCODE_SEMICOLON,
    [0x28] = SDL_SCANCODE_APOSTROPHE,
    [0x29] = SDL_SCANCODE_GRAVE,
    [0x2A] = SDL_SCANCODE_LSHIFT,
    [0x2B] = SDL_SCANCODE_BACKSLASH,
    [0x2C] = SDL_SCANCODE_Z,
    [0x2D] = SDL_SCANCODE_X,
    [0x2E] = SDL_SCANCODE_C,
    [0x2F] = SDL_SCANCODE_V,
    [0x30] = SDL_SCANCODE_B,
    [0x31] = SDL_SCANCODE_N,
    [0x32] = SDL_SCANCODE_M,
    [0x33] = SDL_SCANCODE_COMMA,
    [0x34] = SDL_SCANCODE_PERIOD,
    [0x35] = SDL_SCANCODE_SLASH,
    [0x36] = SDL_SCANCODE_RSHIFT,
    [0x37] = SDL_SCANCODE_KP_MULTIPLY,
    [0x38] = SDL_SCANCODE_LALT,
    [0x39] = SDL_SCANCODE_SPACE,
    [0x3A] = SDL_SCANCODE_CAPSLOCK,
    [0x3B] = SDL_SCANCODE_F1,
    [0x3C] = SDL_SCANCODE_F2,
    [0x3D] = SDL_SCANCODE_F3,
    [0x3E] = SDL_SCANCODE_F4,
    [0x3F] = SDL_SCANCODE_F5,
    [0x40] = SDL_SCANCODE_F6,
    [0x41] = SDL_SCANCODE_F7,
    [0x42] = SDL_SCANCODE_F8,
    [0x43] = SDL_SCANCODE_F9,
    [0x44] = SDL_SCANCODE_F10,
    [0x45] = SDL_SCANCODE_NUMLOCKCLEAR,
    [0x46] = SDL_SCANCODE_SCROLLLOCK,
    [0x47] = SDL_SCANCODE_KP_7,
    [0x48] = SDL_SCANCODE_KP_8,
    [0x49] = SDL_SCANCODE_KP_9,
    [0x4A] = SDL_SCANCODE_KP_MINUS,
    [0x4B] = SDL_SCANCODE_KP_4,
    [0x4C] = SDL_SCANCODE_KP_5,
    [0x4D] = SDL_SCANCODE_KP_6,
    [0x4E] = SDL_SCANCODE_KP_PLUS,
    [0x4F] = SDL_SCANCODE_KP_1,
    [0x50] = SDL_SCANCODE_KP_2,
    [0x51] = SDL_SCANCODE_KP_3,
    [0x52] = SDL_SCANCODE_KP_0,
    [0x53] = SDL_SCANCODE_KP_PERIOD,
    [0x57] = SDL_SCANCODE_F11,
    [0x58] = SDL_SCANCODE_F12,
    [0x80 + 0x1C] = SDL_SCANCODE_KP_ENTER,
    [0x80 + 0x1D] = SDL_SCANCODE_RCTRL,
    [0x80 + 0x35] = SDL_SCANCODE_KP_DIVIDE,
    [0x80 + 0x38] = SDL_SCANCODE_RALT,
    [0x80 + 0x47] = SDL_SCANCODE_HOME,
    [0x80 + 0x48] = SDL_SCANCODE_UP,
    [0x80 + 0x49] = SDL_SCANCODE_PAGEUP,
    [0x80 + 0x4B] = SDL_SCANCODE_LEFT,
    [0x80 + 0x4D] = SDL_SCANCODE_RIGHT,
    [0x80 + 0x4F] = SDL_SCANCODE_END,
    [0x80 + 0x50] = SDL_SCANCODE_DOWN,
    [0x80 + 0x51] = SDL_SCANCODE_PAGEDOWN,
    [0x80 + 0x52] = SDL_SCANCODE_INSERT,
    [0x80 + 0x53] = SDL_SCANCODE_DELETE,
    [0x80 + 0x5B] = SDL_SCANCODE_LGUI,
    [0x80 + 0x5C] = SDL_SCANCODE_RGUI,
    [0x80 + 0x5D] = SDL_SCANCODE_APPLICATION
};

static void push(Uint32 type, SDL_Scancode scancode)
{

    unsigned int next = (eventhead + 1) % EVENTS;

    if (next == eventtail)
        return;

    events[eventhead].key.type = type;
    events[eventhead].key.keysym.scancode = scancode;
    events[eventhead].key.keysym.mod = modifiers;
    eventhead = next;

}

static unsigned int modifier(SDL_Scancode scancode)
{

    switch (scancode)
    {

    case SDL_SCANCODE_LSHIFT:
        return KMOD_LSHIFT;

    case SDL_SCANCODE_RSHIFT:
        return KMOD_RSHIFT;

    case SDL_SCANCODE_LCTRL:
        return KMOD_LCTRL;

    case SDL_SCANCODE_RCTRL:
        return KMOD_RCTRL;

    case SDL_SCANCODE_LALT:
        return KMOD_LALT;

    case SDL_SCANCODE_RALT:
        return KMOD_RALT;

    }

    return 0;

}

static SDL_Scancode translate(unsigned int scancode)
{

    unsigned int id = keys_getcode(&keys, scancode);

    if (!id)
        return SDL_SCANCODE_UNKNOWN;

    return scancodes[((id & 0xF000) == 0x2000 ? 128 : 0) + (id & 0x7F)];

}

static void onkeypress(struct message *message)
{

    struct event_keypress *keypress = message->data;
    SDL_Scancode scancode = translate(keypress->scancode);

    if (!scancode)
        return;

    modifiers |= modifier(scancode);
    keystate[scancode] = 1;

    push(SDL_KEYDOWN, scancode);

}

static void onkeyrelease(struct message *message)
{

    struct event_keyrelease *keyrelease = message->data;
    SDL_Scancode scancode = translate(keyrelease->scancode);

    if (!scancode)
        return;

    modifiers &= ~modifier(scancode);
    keystate[scancode] = 0;

    push(SDL_KEYUP, scancode);

}

/* one tick of sound per timer tick, the game's mixer run as SDL's audio thread would, or silence while paused */
static void sendaudio(void)
{

    unsigned int count = audiospec->freq / TICKRATE * 4;

    buffer_clear(audiobuffer, count);

    if (!audiopaused)
        audiospec->callback(audiospec->userdata, (Uint8 *)audiobuffer, count);

    channel_send(1, audio, EVENT_DATA, count, audiobuffer);

}

/* the clock is the timer's counter, so ticks dropped while the game was busy still count; sound is kept AUDIOCUSHION ticks ahead of it, and what a stall left unsent is skipped */
static void ontimertick(struct message *message)
{

    struct event_timertick *timertick = message->data;

    if (!counting)
    {

        counterbase = timertick->counter - ticks;
        counting = 1;

    }

    ticks = timertick->counter - counterbase;

    if (audio)
    {

        if (audioticks < ticks)
            audioticks = ticks;

        while (audioticks < ticks + AUDIOCUSHION)
        {

            sendaudio();

            audioticks++;

        }

    }

}

static unsigned int setaudioformat(unsigned int rate)
{

    struct event_audioconf audioconf;

    audioconf.rate = rate;
    audioconf.channels = 2;
    audioconf.bits = 16;

    return channel_send(1, audio, EVENT_AUDIOCONF, sizeof (struct event_audioconf), &audioconf);

}

/* the device plays the rate asked for, or 48000 Hz, which every driver takes; the game is told the rate in desired */
int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained)
{

    unsigned int i;

    audio = channel_lookup(option_getstring("audio-service"));

    if (!audio || desired->channels != 2 || desired->format != AUDIO_S16SYS)
    {

        audio = 0;

        return -1;

    }

    if (!setaudioformat(desired->freq))
    {

        if (!setaudioformat(48000))
        {

            audio = 0;

            return -1;

        }

        desired->freq = 48000;

    }

    desired->silence = 0;
    desired->samples = desired->freq / TICKRATE;
    audiospec = desired;
    audiopaused = 1;

    for (i = 0; i < AUDIOCUSHION; i++)
        sendaudio();

    audioticks = ticks + AUDIOCUSHION;

    return 0;

}

void SDL_CloseAudio(void)
{

    audiopaused = 1;

}

void SDL_PauseAudio(int pause)
{

    audiopaused = pause;

}

/* the mixer only runs from the timer tick, between the game's own steps, so there is nothing to lock */
void SDL_LockAudio(void)
{

}

void SDL_UnlockAudio(void)
{

}

/* the game only sends itself sound-ended events, which it ignores */
int SDL_PushEvent(SDL_Event *event)
{

    return 1;

}

/* the messages already waiting are taken first, without blocking, so input and ticks never wait in the mailbox for the next delay */
int SDL_PollEvent(SDL_Event *event)
{

    if (!closed && !channel_drain(0))
        closed = 1;

    if (closed)
    {

        event->type = SDL_QUIT;

        return 1;

    }

    if (eventtail == eventhead)
        return 0;

    event[0] = events[eventtail];
    eventtail = (eventtail + 1) % EVENTS;

    return 1;

}

/* time is counted in timer ticks, so a delay takes messages until enough ticks have passed, at least one */
void SDL_Delay(Uint32 ms)
{

    unsigned int target = ticks + (ms * TICKRATE + 999) / 1000;

    while (!closed && ticks < target)
    {

        if (!channel_process(0))
            closed = 1;

    }

    if (timercallback && SDL_GetTicks() >= timerdue)
    {

        Uint32 interval = timercallback(0, timerparam);

        if (interval)
            timerdue = SDL_GetTicks() + interval;
        else
            timercallback = 0;

    }

}

/* one timer at a time is all the game asks for, run from SDL_Delay */
SDL_TimerID SDL_AddTimer(Uint32 interval, SDL_TimerCallback callback, void *param)
{

    timercallback = callback;
    timerparam = param;
    timerdue = SDL_GetTicks() + interval;

    return 1;

}

const Uint8 *SDL_GetKeyboardState(int *numkeys)
{

    if (numkeys)
        numkeys[0] = SDL_NUM_SCANCODES;

    return keystate;

}

Uint32 SDL_GetTicks(void)
{

    return ticks * 1000 / TICKRATE;

}

/* the counter only moves with timer ticks, but it counts finer so the game's rates above 60 Hz divide to a non-zero count */
Uint32 SDL_GetPerformanceCounter(void)
{

    return ticks * COUNTSPERTICK;

}

Uint32 SDL_GetPerformanceFrequency(void)
{

    return TICKRATE * COUNTSPERTICK;

}

/* the game draws 320x200 RGB24, shown at the largest whole scale that fits, centered */
void glue_present(SDL_Surface *surface)
{

    unsigned int scalew = fbwidth / SCREENWIDTH;
    unsigned int scaleh = fbheight / SCREENHEIGHT;
    unsigned int scale = (scalew < scaleh) ? scalew : scaleh;
    unsigned int totw = SCREENWIDTH * scale;
    unsigned int toth = SCREENHEIGHT * scale;
    unsigned int offx = (fbwidth - totw) / 2;
    unsigned int offy = (fbheight - toth) / 2;
    struct event_videoflush videoflush;
    unsigned int y;

    if (!framebuffer || !scale || totw > 4096)
        return;

    for (y = 0; y < SCREENHEIGHT; y++)
    {

        unsigned char *row = (unsigned char *)surface->pixels + y * surface->pitch;
        unsigned int x;
        unsigned int i;

        for (x = 0; x < SCREENWIDTH; x++)
        {

            unsigned char *p = row + x * 3;
            unsigned int colour = (p[0] << 16) | (p[1] << 8) | p[2];
            unsigned int *out = scaled + x * scale;

            for (i = 0; i < scale; i++)
                out[i] = colour;

        }

        for (i = 0; i < scale; i++)
            buffer_copy(framebuffer + (offy + y * scale + i) * fbwidth + offx, scaled, totw * 4);

    }

    videoflush.x = offx;
    videoflush.y = offy;
    videoflush.width = totw;
    videoflush.height = toth;

    channel_send(0, video, EVENT_VIDEOFLUSH, sizeof (struct event_videoflush), &videoflush);

}

static void ungrab(void)
{

    unsigned int wm = channel_lookup(option_getstring("wm-service"));

    if (wm)
    {

        channel_send(1, wm, EVENT_WMUNGRAB, 0, 0);
        channel_wait(1, wm, EVENT_WMACK, 0, 0);

    }

}

void glue_exit(int status)
{

    if (keyboard)
        channel_send(0, keyboard, EVENT_UNLINK, 0, 0);

    if (timer)
        channel_send(0, timer, EVENT_UNLINK, 0, 0);

    ungrab();
    port_destroy();
    channel_exit(0);
    call_despawn();

    for (;;);

}

static void onmain(struct message *message)
{

    unsigned int wm = channel_lookup(option_getstring("wm-service"));
    struct event_videoconf videoconf;
    struct event_videoinfo videoinfo;

    keyboard = channel_lookup(option_getstring("keyboard-service"));
    timer = channel_lookup(option_getstring("timer-service"));
    video = channel_lookup(option_getstring("video-service"));

    if (!keyboard || !timer || !video)
    {

        channel_send_fmt(0, message->source, EVENT_ERROR, "Could not find the keyboard, timer or video service\n");
        channel_close();

        return;

    }

    if (!port_init(option_getstring("data")))
    {

        channel_send_fmt(0, message->source, EVENT_ERROR, "No mem: file could be mapped\n");
        channel_close();

        return;

    }

    if (wm)
    {

        channel_send(1, wm, EVENT_WMGRAB, 0, 0);
        channel_wait(1, wm, EVENT_WMACK, 0, 0);

    }

    videoconf.width = option_getdecimal("width");
    videoconf.height = option_getdecimal("height");
    videoconf.bpp = option_getdecimal("bpp");

    channel_send(0, video, EVENT_VIDEOCONF, sizeof (struct event_videoconf), &videoconf);
    channel_send(1, video, EVENT_INFO, 0, 0);

    if (channel_wait(1, video, EVENT_VIDEOINFO, sizeof (struct event_videoinfo), &videoinfo) && videoinfo.bpp == 4)
    {

        framebuffer = (unsigned int *)videoinfo.framebuffer;
        fbwidth = videoinfo.width;
        fbheight = videoinfo.height;

    }

    channel_send(0, keyboard, EVENT_LINK, 0, 0);
    channel_send(0, timer, EVENT_LINK, 0, 0);

    g_argc = 1;
    g_argv = arguments;

    pop_main();
    glue_exit(0);

}

void init(void)
{

    keys_init(&keys, KEYS_LAYOUT_QWERTY_US, (void *)0);
    option_add("width", "1024");
    option_add("height", "768");
    option_add("bpp", "4");
    option_add("data", "root:data/pop/");
    option_add("audio-service", "audio");
    option_add("keyboard-service", "keyboard");
    option_add("timer-service", "timer");
    option_add("video-service", "video");
    option_add("wm-service", "wm");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_TIMERTICK, ontimertick);
    channel_bind(EVENT_KEYPRESS, onkeypress);
    channel_bind(EVENT_KEYRELEASE, onkeyrelease);

}

