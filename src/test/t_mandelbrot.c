#include <fudge.h>
#include <abi.h>

#define fpshift                         10
#define tofp(_a)                        ((_a) << fpshift)
#define fpabs(_a)                       ((_a < 0) ? -_a : _a)
#define mulfp(_a)                       (((_a) * (_a)) >> fpshift)

struct rgb
{

    unsigned char r;
    unsigned char g;
    unsigned char b;

};

struct hsv
{

    unsigned char h;
    unsigned char s;
    unsigned char v;

};

struct rgb hsv2rgb(struct hsv hsv)
{

    struct rgb rgb;
    unsigned char region;
    unsigned char remainder;
    unsigned char p, q, t;

    if (hsv.s == 0)
    {

        rgb.r = hsv.v;
        rgb.g = hsv.v;
        rgb.b = hsv.v;

        return rgb;

    }

    region = hsv.h / 43;
    remainder = (hsv.h - (region * 43)) * 6;
    p = (hsv.v * (255 - hsv.s)) >> 8;
    q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
    t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {

        case 0:
            rgb.r = hsv.v;
            rgb.g = t;
            rgb.b = p;

            break;

        case 1:
            rgb.r = q;
            rgb.g = hsv.v;
            rgb.b = p;

            break;

        case 2:
            rgb.r = p;
            rgb.g = hsv.v;
            rgb.b = t;

            break;

        case 3:
            rgb.r = p;
            rgb.g = q;
            rgb.b = hsv.v;

            break;

        case 4:
            rgb.r = t;
            rgb.g = p;
            rgb.b = hsv.v;

            break;

        default:
            rgb.r = hsv.v;
            rgb.g = p;
            rgb.b = q;

            break;

    }

    return rgb;

}

struct hsv rgb2hsv(struct rgb rgb)
{

    unsigned char rgbmin = rgb.r < rgb.g ? (rgb.r < rgb.b ? rgb.r : rgb.b) : (rgb.g < rgb.b ? rgb.g : rgb.b);
    unsigned char rgbmax = rgb.r > rgb.g ? (rgb.r > rgb.b ? rgb.r : rgb.b) : (rgb.g > rgb.b ? rgb.g : rgb.b);
    struct hsv hsv;

    hsv.v = rgbmax;

    if (hsv.v == 0)
    {

        hsv.h = 0;
        hsv.s = 0;

        return hsv;

    }

    hsv.s = 255 * (rgbmax - rgbmin) / hsv.v;

    if (hsv.s == 0)
    {

        hsv.h = 0;

        return hsv;

    }

    if (rgbmax == rgb.r)
        hsv.h = 0 + 43 * (rgb.g - rgb.b) / (rgbmax - rgbmin);
    else if (rgbmax == rgb.g)
        hsv.h = 85 + 43 * (rgb.b - rgb.r) / (rgbmax - rgbmin);
    else
        hsv.h = 171 + 43 * (rgb.r - rgb.g) / (rgbmax - rgbmin);

    return hsv;

}

static void draw(struct event_videoinfo *videoinfo, int x1, int y1, int x2, int y2, unsigned int iterations)
{

    unsigned char buffer[7680];
    int xs = (x2 - x1) / videoinfo->width;
    int ys = (y2 - y1) / videoinfo->height;
    unsigned int x;
    unsigned int y;

    for (y = 0; y < videoinfo->height; y++)
    {

        int yy = y1 + y * ys;

        for (x = 0; x < videoinfo->width; x++)
        {

            unsigned int c;
            int xx = x1 + x * xs;
            int r = 0;
            int i = 0;

            for (c = 0; c < iterations; c++)
            {

                int t;

                if (fpabs(r) > tofp(2))
                    break;

                if (fpabs(i) > tofp(2))
                    break;

                t = mulfp(r) - mulfp(i) + xx;
                i = ((r * i) >> (fpshift - 1)) + yy;
                r = t;

            }

            if (videoinfo->bpp == 4)
            {

                unsigned char *p = buffer + x * 4;
                struct hsv hsv;
                struct rgb rgb;

                hsv.h = (255 * c) / 64;
                hsv.s = 255;
                hsv.v = (c < 64) ? 255 : 0;
                rgb = hsv2rgb(hsv);
                p[0] = rgb.b;
                p[1] = rgb.g;
                p[2] = rgb.r;
                p[3] = 255;

            }

            else
            {

                buffer[x] = (c < 64) ? (255 * c) / 64 : 0;

            }

        }

        /*
        fs_write_all(1, target, data, buffer, videoinfo->width * videoinfo->bpp, videoinfo->width * y * videoinfo->bpp);
        */

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    lookup("wm-service");
    lookup2("mouse-service", "mouse", 0, 0);
    channel_send(0, option_getdecimal("wm-service"), EVENT_WMMAP);

    while (channel_process(0));

    channel_send(0, option_getdecimal("wm-service"), EVENT_WMUNMAP);

}

static void onmousepress(unsigned int source, void *mdata, unsigned int msize)
{

    channel_close();

}

static void onwminit(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_videoconf videoconf;
    struct event_videoinfo videoinfo;

    videoconf.width = option_getdecimal("width");
    videoconf.height = option_getdecimal("height");
    videoconf.bpp = option_getdecimal("bpp");

    channel_send(0, option_getdecimal("wm-service"), EVENT_WMGRAB);
    channel_wait(0, option_getdecimal("wm-service"), EVENT_WMACK);
    channel_send_buffer(0, option_getdecimal("video-service"), EVENT_VIDEOCONF, sizeof (struct event_videoconf), &videoconf);
    channel_wait_buffer(0, option_getdecimal("video-service"), EVENT_VIDEOINFO, sizeof (struct event_videoinfo), &videoinfo);

    if (videoinfo.bpp == 1)
    {

        unsigned char colormap[768];
        unsigned int i;

        for (i = 0; i < 768; i += 3)
        {

            struct hsv hsv;
            struct rgb rgb;

            hsv.h = (64 * (i / 3)) / 256;
            hsv.s = 255;
            hsv.v = (i) ? 255 : 0;
            rgb = hsv2rgb(hsv);
            colormap[i + 0] = rgb.r;
            colormap[i + 1] = rgb.g;
            colormap[i + 2] = rgb.b;

        }

        channel_send_buffer(0, option_getdecimal("video-service"), EVENT_VIDEOCMAP, 768, &colormap);

    }

    draw(&videoinfo, tofp(-2), tofp(-1), tofp(1), tofp(1), 64);
    channel_send(0, option_getdecimal("mouse-service"), EVENT_LINK);

    while (channel_process(0));

    channel_send(0, option_getdecimal("mouse-service"), EVENT_UNLINK);
    channel_send(0, option_getdecimal("wm-service"), EVENT_WMUNGRAB);
    channel_wait(0, option_getdecimal("wm-service"), EVENT_WMACK);

}

void init(void)
{

    option_add("width", "640");
    option_add("height", "480");
    option_add("bpp", "4");
    option_add("mouse-service", "");
    option_add("wm-service", "");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_MOUSEPRESS, onmousepress);
    channel_bind(EVENT_WMINIT, onwminit);

}

