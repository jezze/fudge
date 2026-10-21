#include <fudge.h>
#include <abi.h>
#include "config.h"
#include "doomtype.h"
#include "doomdef.h"
#include "doomstat.h"
#include "d_main.h"
#include "d_event.h"
#include "v_video.h"
#include "r_draw.h"
#include "r_plane.h"
#include "r_things.h"
#include "r_main.h"
#include "f_wipe.h"
#include "st_stuff.h"
#include "am_map.h"
#include "w_wad.h"
#include "i_video.h"
#include "i_joy.h"
#include "lprintf.h"

#define EVENTS                          64
#define RESOLUTIONS                     8
#define LINESIZE                        4096
#define PALETTESIZE                     256
#define TRANSLATIONS                    256

int use_fullscreen;
int desired_fullscreen;
int render_vsync;
int render_screen_multiply = 1;
int process_priority;
dboolean window_focused;
const char *sdl_video_window_pos;
const char *screen_resolution;
const char *screen_resolutions_list[RESOLUTIONS] = {"320x200", "640x400", NULL};
int usejoystick;
int joyleft;
int joyright;
int joyup;
int joydown;
static struct keys keys;
static event_t events[EVENTS];
static unsigned int eventhead;
static unsigned int eventtail;
static unsigned int videotarget;
static unsigned int keyboard;
static unsigned int *framebuffer;
static unsigned int fbwidth;
static unsigned int fbheight;
static unsigned int *colours;
static unsigned int npalettes;
static int cachedgamma = -1;
static int newpal;
static unsigned int line[LINESIZE];

static unsigned short translations[TRANSLATIONS] = {
    [0x01] = KEYD_ESCAPE,
    [0x02] = '1',
    [0x03] = '2',
    [0x04] = '3',
    [0x05] = '4',
    [0x06] = '5',
    [0x07] = '6',
    [0x08] = '7',
    [0x09] = '8',
    [0x0A] = '9',
    [0x0B] = '0',
    [0x0C] = KEYD_MINUS,
    [0x0D] = KEYD_EQUALS,
    [0x0E] = KEYD_BACKSPACE,
    [0x0F] = KEYD_TAB,
    [0x10] = 'q',
    [0x11] = 'w',
    [0x12] = 'e',
    [0x13] = 'r',
    [0x14] = 't',
    [0x15] = 'y',
    [0x16] = 'u',
    [0x17] = 'i',
    [0x18] = 'o',
    [0x19] = 'p',
    [0x1A] = '[',
    [0x1B] = ']',
    [0x1C] = KEYD_ENTER,
    [0x1D] = KEYD_RCTRL,
    [0x1E] = 'a',
    [0x1F] = 's',
    [0x20] = 'd',
    [0x21] = 'f',
    [0x22] = 'g',
    [0x23] = 'h',
    [0x24] = 'j',
    [0x25] = 'k',
    [0x26] = 'l',
    [0x27] = ';',
    [0x28] = '\'',
    [0x29] = '`',
    [0x2A] = KEYD_RSHIFT,
    [0x2B] = '\\',
    [0x2C] = 'z',
    [0x2D] = 'x',
    [0x2E] = 'c',
    [0x2F] = 'v',
    [0x30] = 'b',
    [0x31] = 'n',
    [0x32] = 'm',
    [0x33] = ',',
    [0x34] = '.',
    [0x35] = '/',
    [0x36] = KEYD_RSHIFT,
    [0x37] = KEYD_KEYPADMULTIPLY,
    [0x38] = KEYD_RALT,
    [0x39] = KEYD_SPACEBAR,
    [0x3A] = KEYD_CAPSLOCK,
    [0x3B] = KEYD_F1,
    [0x3C] = KEYD_F2,
    [0x3D] = KEYD_F3,
    [0x3E] = KEYD_F4,
    [0x3F] = KEYD_F5,
    [0x40] = KEYD_F6,
    [0x41] = KEYD_F7,
    [0x42] = KEYD_F8,
    [0x43] = KEYD_F9,
    [0x44] = KEYD_F10,
    [0x45] = KEYD_NUMLOCK,
    [0x46] = KEYD_SCROLLLOCK,
    [0x47] = KEYD_KEYPAD7,
    [0x48] = KEYD_KEYPAD8,
    [0x49] = KEYD_KEYPAD9,
    [0x4A] = KEYD_KEYPADMINUS,
    [0x4B] = KEYD_KEYPAD4,
    [0x4C] = KEYD_KEYPAD5,
    [0x4D] = KEYD_KEYPAD6,
    [0x4E] = KEYD_KEYPADPLUS,
    [0x4F] = KEYD_KEYPAD1,
    [0x50] = KEYD_KEYPAD2,
    [0x51] = KEYD_KEYPAD3,
    [0x52] = KEYD_KEYPAD0,
    [0x53] = KEYD_KEYPADPERIOD,
    [0x57] = KEYD_F11,
    [0x58] = KEYD_F12,
    [0x80 + 0x1C] = KEYD_KEYPADENTER,
    [0x80 + 0x1D] = KEYD_RCTRL,
    [0x80 + 0x35] = KEYD_KEYPADDIVIDE,
    [0x80 + 0x38] = KEYD_RALT,
    [0x80 + 0x47] = KEYD_HOME,
    [0x80 + 0x48] = KEYD_UPARROW,
    [0x80 + 0x49] = KEYD_PAGEUP,
    [0x80 + 0x4B] = KEYD_LEFTARROW,
    [0x80 + 0x4D] = KEYD_RIGHTARROW,
    [0x80 + 0x4F] = KEYD_END,
    [0x80 + 0x50] = KEYD_DOWNARROW,
    [0x80 + 0x51] = KEYD_PAGEDOWN,
    [0x80 + 0x52] = KEYD_INSERT,
    [0x80 + 0x53] = KEYD_DEL
};

static void push(evtype_t type, int key)
{

    unsigned int next = (eventhead + 1) % EVENTS;

    if (!key || next == eventtail)
        return;

    events[eventhead].type = type;
    events[eventhead].data1 = key;
    events[eventhead].data2 = 0;
    events[eventhead].data3 = 0;
    eventhead = next;

}

static int translate(unsigned int scancode)
{

    unsigned int id = keys_getcode(&keys, scancode);

    if (!id)
        return 0;

    return translations[((id & 0xF000) == 0x2000 ? 128 : 0) + (id & 0x7F)];

}

static void onkeypress(struct message *message)
{

    struct event_keypress *keypress = message->data;

    push(ev_keydown, translate(keypress->scancode));

}

static void onkeyrelease(struct message *message)
{

    struct event_keyrelease *keyrelease = message->data;

    push(ev_keyup, translate(keyrelease->scancode));

}

void I_StartTic(void)
{

    while (eventtail != eventhead)
    {

        D_PostEvent(&events[eventtail]);

        eventtail = (eventtail + 1) % EVENTS;

    }

}

void I_StartFrame(void)
{

}

void I_InitJoystick(void)
{

}

void I_PollJoystick(void)
{

}

/* every palette of PLAYPAL through the gamma table, as 0x00RRGGBB */
static void loadpalettes(void)
{

    int pplump = W_GetNumForName("PLAYPAL");
    int gtlump = (W_CheckNumForName)("GAMMATBL", ns_prboom);
    const byte *palette = W_CacheLumpNum(pplump);
    const byte *gtable = (const byte *)W_CacheLumpNum(gtlump) + 256 * usegamma;
    unsigned int i;

    npalettes = W_LumpLength(pplump) / (3 * PALETTESIZE);

    if (!colours)
        colours = malloc(sizeof (unsigned int) * PALETTESIZE * npalettes);

    for (i = 0; i < PALETTESIZE * npalettes; i++)
        colours[i] = (gtable[palette[i * 3]] << 16) | (gtable[palette[i * 3 + 1]] << 8) | gtable[palette[i * 3 + 2]];

    W_UnlockLumpNum(pplump);
    W_UnlockLumpNum(gtlump);

    cachedgamma = usegamma;

}

void I_SetPalette(int pal)
{

    newpal = pal;

}

void I_UpdateNoBlit(void)
{

}

/* the game draws 8-bit pixels, shown at the largest whole scale that fits, centered */
void I_FinishUpdate(void)
{

    unsigned int scalew = fbwidth / SCREENWIDTH;
    unsigned int scaleh = fbheight / SCREENHEIGHT;
    unsigned int scale = (scalew < scaleh) ? scalew : scaleh;
    unsigned int totw = SCREENWIDTH * scale;
    unsigned int toth = SCREENHEIGHT * scale;
    unsigned int offx = (fbwidth - totw) / 2;
    unsigned int offy = (fbheight - toth) / 2;
    struct event_videoflush videoflush;
    unsigned int *palette;
    int y;

    if (!framebuffer || !scale || totw > LINESIZE)
        return;

    if (cachedgamma != usegamma)
        loadpalettes();

    palette = colours + PALETTESIZE * (((unsigned int)newpal < npalettes) ? newpal : 0);

    for (y = 0; y < SCREENHEIGHT; y++)
    {

        byte *row = screens[0].data + y * screens[0].byte_pitch;
        unsigned int x;
        unsigned int i;

        for (x = 0; x < totw; x++)
            line[x] = palette[row[x / scale]];

        for (i = 0; i < scale; i++)
            buffer_copy(framebuffer + (offy + y * scale + i) * fbwidth + offx, line, totw * 4);

    }

    videoflush.x = offx;
    videoflush.y = offy;
    videoflush.width = totw;
    videoflush.height = toth;

    channel_send(0, videotarget, EVENT_VIDEOFLUSH, sizeof (struct event_videoflush), &videoflush);

}

int I_ScreenShot(const char *fname)
{

    return 0;

}

static void setmode(void)
{

    struct event_videoconf videoconf;
    struct event_videoinfo videoinfo;

    videoconf.width = option_getdecimal("width");
    videoconf.height = option_getdecimal("height");
    videoconf.bpp = 4;

    channel_send(0, videotarget, EVENT_VIDEOCONF, sizeof (struct event_videoconf), &videoconf);
    channel_send(1, videotarget, EVENT_INFO, 0, 0);

    if (channel_wait(1, videotarget, EVENT_VIDEOINFO, sizeof (struct event_videoinfo), &videoinfo) && videoinfo.bpp == 4)
    {

        framebuffer = (unsigned int *)videoinfo.framebuffer;
        fbwidth = videoinfo.width;
        fbheight = videoinfo.height;

    }

}

/* only 8-bit rendering, at the size of the options renderwidth and renderheight */
void I_InitScreenResolution(void)
{

    int i;

    desired_screenwidth = option_getdecimal("renderwidth");
    desired_screenheight = option_getdecimal("renderheight");

    V_InitMode(VID_MODE8);

    SCREENWIDTH = desired_screenwidth;
    SCREENHEIGHT = desired_screenheight;
    SCREENPITCH = SCREENWIDTH;
    REAL_SCREENWIDTH = SCREENWIDTH;
    REAL_SCREENHEIGHT = SCREENHEIGHT;
    REAL_SCREENPITCH = SCREENPITCH;

    V_DestroyUnusedTrueColorPalettes();
    V_FreeScreens();

    for (i = 0; i < 5; i++)
    {

        screens[i].width = REAL_SCREENWIDTH;
        screens[i].height = REAL_SCREENHEIGHT;
        screens[i].byte_pitch = REAL_SCREENPITCH;
        screens[i].short_pitch = REAL_SCREENPITCH / V_GetModePixelDepth(VID_MODE16);
        screens[i].int_pitch = REAL_SCREENPITCH / V_GetModePixelDepth(VID_MODE32);

    }

    R_InitMeltRes();
    R_InitSpritesRes();
    R_InitBuffersRes();
    R_InitPlanesRes();
    R_InitVisplanesRes();

    lprintf(LO_INFO, "I_InitScreenResolution: Using resolution %dx%d\n", REAL_SCREENWIDTH, REAL_SCREENHEIGHT);

}

void I_UpdateVideoMode(void)
{

    if (!framebuffer)
        setmode();

    screens[0].not_on_heap = false;

    V_AllocScreens();
    R_InitBuffer(SCREENWIDTH, SCREENHEIGHT);
    R_ExecuteSetViewSize();
    V_SetPalette(0);
    loadpalettes();
    ST_SetResolution();
    AM_SetResolution();

}

static void shutdowngraphics(void)
{

    if (keyboard)
        channel_send(0, keyboard, EVENT_UNLINK, 0, 0);

}

void I_InitGraphics(void)
{

    static int firsttime = 1;

    if (!firsttime)
        return;

    firsttime = 0;
    videotarget = channel_lookup(option_getstring("video-service"));
    keyboard = channel_lookup(option_getstring("keyboard-service"));

    if (!videotarget || !keyboard)
        I_Error("I_InitGraphics: could not find the video or keyboard service");

    atexit(shutdowngraphics);
    I_UpdateVideoMode();
    keys_init(&keys, KEYS_LAYOUT_QWERTY_US, (void *)0);
    channel_bind(EVENT_KEYPRESS, onkeypress);
    channel_bind(EVENT_KEYRELEASE, onkeyrelease);
    channel_send(0, keyboard, EVENT_LINK, 0, 0);

}

