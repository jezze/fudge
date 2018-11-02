#include <abi.h>
#include <fudge.h>

#define fpshift                         10
#define tofp(_a)                        ((_a) << fpshift)
#define fpabs(_a)                       ((_a < 0) ? -_a : _a)
#define mulfp(_a)                       (((_a) * (_a)) >> fpshift)

static struct ctrl_videosettings settings;

static void setup(struct ctrl_videosettings *settings)
{

    unsigned char colormap[768];
    unsigned int i;

    for (i = 0; i < 768; i += 3)
    {

        colormap[i + 0] = i;
        colormap[i + 1] = i;
        colormap[i + 2] = i;

    }

    file_walk(FILE_L1, FILE_L0, "colormap");
    file_open(FILE_L1);
    file_seekwriteall(FILE_L1, colormap, 768, 0);
    file_close(FILE_L1);

}

static void draw(struct ctrl_videosettings *settings, int x1, int y1, int x2, int y2, unsigned int iterations)
{

    char buffer[4096];
    int xs = (x2 - x1) / settings->w;
    int ys = (y2 - y1) / settings->h;
    unsigned int x;
    unsigned int y;

    file_walk(FILE_L1, FILE_L0, "data");
    file_open(FILE_L1);

    for (y = 0; y < settings->h; y++)
    {

        int yy = y1 + y * ys;

        for (x = 0; x < settings->w; x++)
        {

            int xx = x1 + x * xs;
            unsigned char c;
            int r = 0;
            int i = 0;

            for (c = 0; (c < iterations) && (fpabs(r) < tofp(2)) && (fpabs(i) < tofp(2)); c++)
            {

                int t = mulfp(r) - mulfp(i) + xx;

                i = ((r * i) >> (fpshift - 1)) + yy;
                r = t;

            }

            buffer[x] = c % 16;

        }

        file_seekwriteall(FILE_L1, buffer, settings->w, settings->w * y);

    }

    file_close(FILE_L1);

}

static unsigned int onstop(struct event_header *iheader, struct event_header *oheader)
{

    struct event_stop *stop = event_getdata(iheader);

    draw(&settings, tofp(-2), tofp(-1), tofp(1), tofp(1), 64);
    event_replystop(oheader, iheader, stop->session);
    event_send(oheader);

    return 1;

}

static unsigned int oninit(struct event_header *iheader, struct event_header *oheader)
{

    ctrl_setvideosettings(&settings, 320, 200, 1);
    file_walk2(FILE_L0, "/system/video/if:0");
    file_walk(FILE_L1, FILE_L0, "ctrl");
    file_open(FILE_L1);
    file_writeall(FILE_L1, &settings, sizeof (struct ctrl_videosettings));
    file_close(FILE_L1);
    file_open(FILE_L1);
    file_readall(FILE_L1, &settings, sizeof (struct ctrl_videosettings));
    file_close(FILE_L1);
    setup(&settings);

    return 0;

}

static unsigned int onkill(struct event_header *iheader, struct event_header *oheader)
{

    return 1;

}

void main(void)
{

    unsigned int status = 0;

    event_open();

    while (!status)
    {

        char ibuffer[FUDGE_BSIZE];
        char obuffer[FUDGE_BSIZE];
        struct event_header *iheader = event_read(ibuffer);
        struct event_header *oheader = (struct event_header *)obuffer;

        switch (iheader->type)
        {

        case EVENT_STOP:
            status = onstop(iheader, oheader);

            break;

        case EVENT_INIT:
            status = oninit(iheader, oheader);

            break;

        case EVENT_KILL:
            status = onkill(iheader, oheader);

            break;

        }

    }

    event_close();

}

