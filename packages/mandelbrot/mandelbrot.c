#include <abi.h>
#include <fudge.h>
#include "video.h"

#define fpshift                         10
#define tofp(_a)                        ((_a) << fpshift)
#define fromfp(_a)                      ((_a) >> fpshift)
#define fpabs(_a)                       ((_a < 0) ? -_a : _a)
#define mulfp(_a,_b)                    (((_a) * (_b)) >> fpshift)
#define divfp(_a,_b)                    (((_a) << fpshift) / (_b))

void draw(struct ctrl_videosettings *settings, int x1, int y1, int x2, int y2, unsigned int iterations)
{

    int xs = (x2 - x1) / settings->w;
    int ys = (y2 - y1) / settings->h;
    char buffer[4096];
    int x, y;

    for (y = 0; y < settings->h; y++)
    {

        int yy = y1 + y * ys;

        for (x = 0; x < settings->w; x++)
        {

            int xx = x1 + x * xs;
            unsigned char c;
            int t, r = 0, i = 0;

            for (c = 0; (c < iterations) && (fpabs(r) < tofp(2)) && (fpabs(i) < tofp(2)); c++)
            {

                t = (mulfp(r, r) - mulfp(i, i)) + xx;
                i = ((r * i) >> (fpshift - 1)) + yy;
                r = t;

            }

            if (c == iterations)
                buffer[x] = 0;
            else
                buffer[x] = c % 16;

        }

        video_draw(y * settings->w, settings->w, buffer);

    }

}

void main()
{

    struct ctrl_videosettings settings;

    settings.w = 320;
    settings.h = 200;
    settings.bpp = 8;

    video_setmode(&settings);
    video_open();
    draw(&settings, tofp(-2), tofp(-2), tofp(2), tofp(2), 64);
    video_close();

    for (;;);

}

