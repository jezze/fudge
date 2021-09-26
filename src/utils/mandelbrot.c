#include <fudge.h>
#include <abi.h>

#define fpshift                         10
#define tofp(_a)                        ((_a) << fpshift)
#define fpabs(_a)                       ((_a < 0) ? -_a : _a)
#define mulfp(_a)                       (((_a) * (_a)) >> fpshift)

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
    file_seekwriteall(FILE_L1, colormap, 768, 0);

}

static void draw(struct ctrl_videosettings *settings, int x1, int y1, int x2, int y2, unsigned int iterations)
{

    char buffer[4096];
    int xs = (x2 - x1) / settings->w;
    int ys = (y2 - y1) / settings->h;
    unsigned int x;
    unsigned int y;

    file_walk(FILE_L1, FILE_L0, "data");

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

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct ctrl_videosettings settings;

    ctrl_setvideosettings(&settings, 320, 200, 1);
    file_walk2(FILE_L0, "system:video/if:0");
    file_walk(FILE_L1, FILE_L0, "ctrl");
    file_seekwriteall(FILE_L1, &settings, sizeof (struct ctrl_videosettings), 0);
    file_seekreadall(FILE_L1, &settings, sizeof (struct ctrl_videosettings), 0);
    setup(&settings);
    draw(&settings, tofp(-2), tofp(-1), tofp(1), tofp(1), 64);
    channel_close();

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);

}

