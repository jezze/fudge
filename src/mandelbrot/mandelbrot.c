#include <abi.h>
#include <fudge.h>

#define fpshift                         10
#define tofp(_a)                        ((_a) << fpshift)
#define fpabs(_a)                       ((_a < 0) ? -_a : _a)
#define mulfp(_a)                       (((_a) * (_a)) >> fpshift)

void setup(struct ctrl_videosettings *settings)
{

    unsigned char colormap[768];
    unsigned int i;

    for (i = 0; i < 768; i += 3)
    {

        colormap[i + 0] = i;
        colormap[i + 1] = i;
        colormap[i + 2] = i;

    }

    call_walk(CALL_L1, CALL_L0, 8, "colormap");
    call_open(CALL_L1);
    file_seekwriteall(CALL_L1, colormap, 768, 0);
    call_close(CALL_L1);

}

void draw(struct ctrl_videosettings *settings, int x1, int y1, int x2, int y2, unsigned int iterations)
{

    char buffer[4096];
    int xs = (x2 - x1) / settings->w;
    int ys = (y2 - y1) / settings->h;
    unsigned int x;
    unsigned int y;

    call_walk(CALL_L1, CALL_L0, 4, "data");
    call_open(CALL_L1);

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

        file_seekwriteall(CALL_L1, buffer, settings->w, settings->w * y);

    }

    call_close(CALL_L1);

}

void main(void)
{

    struct ctrl_videosettings settings;

    ctrl_setvideosettings(&settings, 320, 200, 8);
    call_walk(CALL_L0, CALL_PR, 18, "system/video/if:0/");
    call_walk(CALL_L1, CALL_L0, 4, "ctrl");
    call_open(CALL_L1);
    file_writeall(CALL_L1, &settings, sizeof (struct ctrl_videosettings));
    call_close(CALL_L1);
    call_open(CALL_L1);
    file_readall(CALL_L1, &settings, sizeof (struct ctrl_videosettings));
    call_close(CALL_L1);
    setup(&settings);
    draw(&settings, tofp(-2), tofp(-1), tofp(1), tofp(1), 64);

}

