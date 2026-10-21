#include <fudge.h>
#include <abi.h>
#include "config.h"
#include "doomdef.h"
#include "doomstat.h"
#include "d_main.h"
#include "m_argv.h"
#include "m_misc.h"
#include "g_game.h"
#include "r_fps.h"
#include "r_demo.h"
#include "z_zone.h"
#include "i_main.h"
#include "i_system.h"
#include "i_sound.h"
#include "lprintf.h"
#include "cybermind.h"
#include "e6y.h"
#include "glue.h"

#define EXITS                           32
#define ARGUMENTS                       16
#define ARGUMENTSIZE                    256
#define TIMERID                         1

int realtic_clock_rate = 100;
int endoom_mode;
int (*I_GetTime)(void);
static void (*exits[EXITS])(void);
static unsigned int nexits;
static int has_exited;
static long long scale = 1 << 24;
static unsigned int source;
static unsigned int timer;
static unsigned int timerhz = 60;
static unsigned int closed;
static char *arguments[ARGUMENTS];
static char values[ARGUMENTSIZE];
static unsigned int valuesoffset;

static int gettimescaled(void)
{

    return (int)((long long)I_GetTime_RealTime() * scale >> 24);

}

static int gettimefast(void)
{

    static int fasttic;

    return fasttic++;

}

static void settime(void)
{

    if (fastdemo)
    {

        I_GetTime = gettimefast;

    }

    else if (realtic_clock_rate != 100)
    {

        scale = ((long long)realtic_clock_rate << 24) / 100;
        I_GetTime = gettimescaled;

    }

    else
    {

        I_GetTime = I_GetTime_RealTime;

    }

}

void I_Init(void)
{

    settime();

    if (!(nomusicparm && nosfxparm))
        I_InitSound();

    R_InitInterpolation();

}

void I_Init2(void)
{

    settime();
    R_InitInterpolation();

    force_singletics_to = gametic + BACKUPTICS;

}

/* the timer's counter is the clock, asked for whenever the game wants the time, so no ticks pile up in the mailbox */
unsigned int SDL_GetTicks(void)
{

    struct event_timerinfo timerinfo;

    if (!timer || !channel_send(1, timer, EVENT_INFO, 0, 0) || !channel_wait(1, timer, EVENT_TIMERINFO, sizeof (struct event_timerinfo), &timerinfo))
        return 0;

    if (timerinfo.hz)
        timerhz = timerinfo.hz;

    return (unsigned long long)timerinfo.counter * 1000 / timerhz;

}

/* a sleep takes messages, keys among them, until the timer expires */
void I_uSleep(unsigned long usecs)
{

    struct event_timerrequest timerrequest;
    struct message message;

    timerrequest.id = TIMERID;
    timerrequest.ms = (usecs < 1000) ? 1 : usecs / 1000;
    timerrequest.repeat = 0;

    if (closed || !channel_send(0, timer, EVENT_TIMERREQUEST, sizeof (struct event_timerrequest), &timerrequest))
        return;

    if (!channel_poll(0, timer, EVENT_TIMEREXPIRE, &message))
    {

        closed = 1;

        I_SafeExit(0);

    }

}

int atexit(void (*function)(void))
{

    if (nexits == EXITS)
        return -1;

    exits[nexits++] = function;

    return 0;

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

void exit(int status)
{

    while (nexits)
        exits[--nexits]();

    ungrab();
    port_destroy();

    if (status && source)
        channel_send_fmt(0, source, EVENT_ERROR, "%s", port_lasterror());

    channel_exit(0);
    call_despawn((status) ? EXIT_STATUS_FAILED : EXIT_STATUS_NORMAL);

    for (;;);

}

void I_SafeExit(int rc)
{

    if (!has_exited)
    {

        has_exited = rc ? 2 : 1;

        exit(rc);

    }

}

static void quit(void)
{

    if (!has_exited)
        has_exited = 1;

    if (has_exited == 1)
    {

        I_ShutdownRecording();

        if (demorecording)
            G_CheckDemoStatus();

        M_SaveDefaults();
        I_DemoExShutdown();

        if (dump_things)
            cyb_DumpEnd(dumpFile);

    }

}

/* the options become the game's own command line, a value with spaces as several arguments */
static void addargument(char *name, char *option)
{

    char *value = option_getstring(option);
    char *token;

    if (!cstring_length(value) || myargc + 1 >= ARGUMENTS || valuesoffset + cstring_length(value) + 1 > ARGUMENTSIZE)
        return;

    arguments[myargc++] = name;
    token = strtok(strcpy(values + valuesoffset, value), " ");
    valuesoffset += cstring_length(value) + 1;

    for (; token && myargc < ARGUMENTS; token = strtok(NULL, " "))
        arguments[myargc++] = token;

}

static void onmain(struct message *message)
{

    unsigned int wm = channel_lookup(option_getstring("wm-service"));

    source = message->source;
    timer = channel_lookup(option_getstring("timer-service"));

    if (!timer)
    {

        channel_send_fmt(0, message->source, EVENT_ERROR, "Could not find the timer service\n");
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

    I_SetDataDir(option_getstring("data"));

    arguments[0] = "doom";
    myargc = 1;

    addargument("-iwad", "iwad");
    addargument("-file", "file");
    addargument("-warp", "warp");
    addargument("-skill", "skill");

    myargv = arguments;

    ParamsMatchingCheck();
    M_LoadDefaults();
    atexit(Z_Close);
    Z_Init();
    atexit(quit);
    D_DoomMain();
    I_SafeExit(0);

}

void init(void)
{

    option_add("data", "root:data/doom/");
    option_add("iwad", "");
    option_add("file", "");
    option_add("warp", "");
    option_add("skill", "");
    option_add("width", "1024");
    option_add("height", "768");
    option_add("renderwidth", "320");
    option_add("renderheight", "200");
    option_add("keyboard-service", "keyboard");
    option_add("timer-service", "timer");
    option_add("video-service", "video");
    option_add("wm-service", "wm");
    channel_bind(EVENT_MAIN, onmain);

}

