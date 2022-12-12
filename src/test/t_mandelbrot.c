#include <fudge.h>
#include <abi.h>

#define fpshift                         10
#define tofp(_a)                        ((_a) << fpshift)
#define fpabs(_a)                       ((_a < 0) ? -_a : _a)
#define mulfp(_a)                       (((_a) * (_a)) >> fpshift)

static struct ctrl_videosettings oldsettings;
static struct ctrl_videosettings newsettings;

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

static void draw(struct ctrl_videosettings *settings, int x1, int y1, int x2, int y2, unsigned int iterations)
{

    char buffer[7680];
    int xs = (x2 - x1) / settings->width;
    int ys = (y2 - y1) / settings->height;
    unsigned int x;
    unsigned int y;

    for (y = 0; y < settings->height; y++)
    {

        int yy = y1 + y * ys;

        for (x = 0; x < settings->width; x++)
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

            if (settings->bpp == 4)
            {

                unsigned char *p = (unsigned char *)(buffer + x * 4);
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

                buffer[x] = c % 16;

            }

        }

        file_seekwriteall(FILE_G2, buffer, settings->width * settings->bpp, settings->width * y * settings->bpp);

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (!file_walk2(FILE_L0, option_getstring("video")))
        channel_error("Could not find video device");

    if (!file_walk(FILE_G1, FILE_L0, "ctrl"))
        channel_error("Could not find video device ctrl");

    if (!file_walk(FILE_G2, FILE_L0, "data"))
        channel_error("Could not find video device data");

    file_seekreadall(FILE_G1, &oldsettings, sizeof (struct ctrl_videosettings), 0);
    file_seekwriteall(FILE_G1, &newsettings, sizeof (struct ctrl_videosettings), 0);
    file_seekreadall(FILE_G1, &newsettings, sizeof (struct ctrl_videosettings), 0);

    if (newsettings.bpp == 1)
    {

        unsigned char colormap[768];
        unsigned int i;

        for (i = 0; i < 768; i += 3)
        {

            colormap[i + 0] = i;
            colormap[i + 1] = i;
            colormap[i + 2] = i;

        }

        if (!file_walk(FILE_L1, FILE_L0, "colormap"))
            channel_error("Could not find video device colormap");

        file_seekwriteall(FILE_L1, colormap, 768, 0);

    }

    draw(&newsettings, tofp(-2), tofp(-1), tofp(1), tofp(1), 64);

}

static void onterm(unsigned int source, void *mdata, unsigned int msize)
{

    file_seekwriteall(FILE_G1, &oldsettings, sizeof (struct ctrl_videosettings), 0);
    channel_close();

}

void init(void)
{

    newsettings.width = 640;
    newsettings.height = 480;
    newsettings.bpp = 4;

    option_add("video", "system:video/if:0");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_TERM, onterm);

}

