#include <abi.h>
#include <fudge.h>
#include <video/video.h>

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

    video_setcolormap(0, 3, 256, colormap);

}

void draw(struct ctrl_videosettings *settings, int x1, int y1, int x2, int y2, unsigned int iterations)
{

    char buffer[4096];
    int xs = (x2 - x1) / settings->w;
    int ys = (y2 - y1) / settings->h;
    int x;
    int y;

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

        video_draw(y * settings->w, settings->w, buffer);

    }

}

void main()
{

    struct ctrl_videosettings settings;

    ctrl_initvideosettings(&settings, 320, 200, 8);
    video_setmode(&settings);
    setup(&settings);
    video_open();
    draw(&settings, tofp(-2), tofp(-1), tofp(1), tofp(1), 64);
    video_close();

}

