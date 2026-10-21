#include "config.h"
#include "doomtype.h"
#include "doomdef.h"
#include "doomstat.h"
#include "m_fixed.h"
#include "r_fps.h"
#include "i_system.h"
#include "i_capture.h"
#include "lprintf.h"
#include "e6y.h"

int ms_to_next_tick;
int interpolation_method;
extern int renderer_fps;
dboolean realframe = false;
const char *cap_soundcommand;
const char *cap_videocommand;
const char *cap_muxcommand;
const char *cap_tempfile1;
const char *cap_tempfile2;
int cap_remove_tempfiles;
int cap_fps;
int cap_frac;
int capturing_video = 0;
static unsigned int basetime;
static unsigned int start_displaytime;
static unsigned int displaytime;
static dboolean indisplay = false;
static int saved_gametic = -1;
static int subframe = 0;
static int prevsubframe = 0;
static char datadir[PATH_MAX];

int I_GetTime_RealTime(void)
{

    unsigned int t = SDL_GetTicks();
    int i;

    if (!basetime)
        basetime = t;

    t -= basetime;
    i = t * (TICRATE / 5) / 200;
    ms_to_next_tick = (i + 1) * 200 / (TICRATE / 5) - t;

    if (ms_to_next_tick > 1000 / TICRATE || ms_to_next_tick < 1)
        ms_to_next_tick = 1;

    return i;

}

dboolean I_StartDisplay(void)
{

    if (indisplay)
        return false;

    realframe = (!movement_smooth) || (gametic > saved_gametic);

    if (realframe)
        saved_gametic = gametic;

    start_displaytime = SDL_GetTicks();
    indisplay = true;

    return true;

}

void I_EndDisplay(void)
{

    displaytime = SDL_GetTicks() - start_displaytime;
    indisplay = false;

}

fixed_t I_GetTimeFrac(void)
{

    unsigned int now = SDL_GetTicks();
    fixed_t frac;

    subframe++;

    if (tic_vars.step == 0)
        return FRACUNIT;

    if (interpolation_method == 0 || prevsubframe <= 0 || renderer_fps <= 0)
        frac = (fixed_t)((now - tic_vars.start + displaytime) * FRACUNIT / tic_vars.step);
    else
        frac = (fixed_t)((unsigned int)FRACUNIT * TICRATE * subframe / renderer_fps);

    return BETWEEN(0, FRACUNIT, frac);

}

void I_GetTime_SaveMS(void)
{

    if (!movement_smooth)
        return;

    tic_vars.start = SDL_GetTicks();
    tic_vars.next = (unsigned int)((tic_vars.start * tic_vars.msec + 1.0f) / tic_vars.msec);
    tic_vars.step = tic_vars.next - tic_vars.start;
    prevsubframe = subframe;
    subframe = 0;

}

unsigned long I_GetRandomTimeSeed(void)
{

    return SDL_GetTicks();

}

const char *I_GetVersionString(char *buf, size_t sz)
{

    snprintf(buf, sz, "%s v%s", PACKAGE_NAME, PACKAGE_VERSION);

    return buf;

}

const char *I_SigString(char *buf, size_t sz, int signum)
{

    snprintf(buf, sz, "signal %d", signum);

    return buf;

}

void I_Read(int fd, void *vbuf, size_t sz)
{

    unsigned char *buf = vbuf;

    while (sz)
    {

        int rc = read(fd, buf, sz);

        if (rc <= 0)
            I_Error("I_Read: read failed");

        sz -= rc;
        buf += rc;

    }

}

int I_Filelength(int handle)
{

    int offset = lseek(handle, 0, SEEK_CUR);
    int length = lseek(handle, 0, SEEK_END);

    lseek(handle, offset, SEEK_SET);

    return length;

}

/* the data directory stands in for the executable's directory, the home directory and the current one */
void I_SetDataDir(const char *path)
{

    snprintf(datadir, PATH_MAX, "%s", path);

}

const char *I_DoomExeDir(void)
{

    return datadir;

}

const char *I_GetTempDir(void)
{

    return datadir;

}

dboolean HasTrailingSlash(const char *dn)
{

    size_t length = strlen(dn);

    return length && (dn[length - 1] == '/' || dn[length - 1] == ':');

}

/* a name with a service is taken as it is, anything else is looked for in the data directory */
static char *findfile(char *out, const char *wfname, const char *ext)
{

    if (strchr(wfname, ':'))
        snprintf(out, PATH_MAX, "%s", wfname);
    else
        snprintf(out, PATH_MAX, "%s%s%s", datadir, HasTrailingSlash(datadir) ? "" : "/", wfname);

    if (ext && access(out, F_OK))
        strncat(out, ext, PATH_MAX - strlen(out) - 1);

    return (access(out, F_OK)) ? NULL : out;

}

char *I_FindFile(const char *wfname, const char *ext)
{

    char *p;

    if (!wfname)
        return NULL;

    p = malloc(PATH_MAX);

    if (findfile(p, wfname, ext))
    {

        lprintf(LO_INFO, " found %s\n", p);

        return p;

    }

    free(p);

    return NULL;

}

const char *I_FindFile2(const char *wfname, const char *ext)
{

    static char p[PATH_MAX];

    if (!wfname)
        return NULL;

    return findfile(p, wfname, ext);

}

void I_CapturePrep(const char *fn)
{

}

void I_CaptureFrame(void)
{

}

void I_CaptureFinish(void)
{

}

