#include <fudge.h>
#include <compression.h>
#include <image.h>
#include "port.h"
#include "sdl.h"

#define PNGWIDTH                        2048

struct pixel
{

    unsigned int r;
    unsigned int g;
    unsigned int b;
    unsigned int a;
    unsigned int index;
    unsigned int skip;

};

static struct png png;
static const unsigned char *pngdata;
static unsigned int pngcount;
static unsigned int pngrow[PNGWIDTH];

static unsigned int shift(Uint32 mask)
{

    unsigned int s = 0;

    if (!mask)
        return 0;

    while (!(mask & 1))
    {

        mask >>= 1;
        s++;

    }

    return s;

}

static void setformat(SDL_PixelFormat *format, Uint32 id)
{

    format->format = id;

    switch (id)
    {

    case SDL_PIXELFORMAT_INDEX8:
        format->BitsPerPixel = 8;
        format->BytesPerPixel = 1;
        format->Rmask = format->Gmask = format->Bmask = format->Amask = 0;

        break;

    case SDL_PIXELFORMAT_RGB24:
        format->BitsPerPixel = 24;
        format->BytesPerPixel = 3;
        format->Rmask = 0x000000FF;
        format->Gmask = 0x0000FF00;
        format->Bmask = 0x00FF0000;
        format->Amask = 0;

        break;

    case SDL_PIXELFORMAT_ARGB8888:
        format->BitsPerPixel = 32;
        format->BytesPerPixel = 4;
        format->Rmask = 0x00FF0000;
        format->Gmask = 0x0000FF00;
        format->Bmask = 0x000000FF;
        format->Amask = 0xFF000000;

        break;

    case SDL_PIXELFORMAT_ABGR8888:
        format->BitsPerPixel = 32;
        format->BytesPerPixel = 4;
        format->Rmask = 0x000000FF;
        format->Gmask = 0x0000FF00;
        format->Bmask = 0x00FF0000;
        format->Amask = 0xFF000000;

        break;

    }

    format->Rshift = shift(format->Rmask);
    format->Gshift = shift(format->Gmask);
    format->Bshift = shift(format->Bmask);
    format->Ashift = shift(format->Amask);

}

static Uint32 formatfrommasks(int depth, Uint32 rmask, Uint32 amask)
{

    switch (depth)
    {

    case 8:
        return SDL_PIXELFORMAT_INDEX8;

    case 24:
        return SDL_PIXELFORMAT_RGB24;

    case 32:
        return (rmask == 0x000000FF) ? SDL_PIXELFORMAT_ABGR8888 : SDL_PIXELFORMAT_ARGB8888;

    }

    return SDL_PIXELFORMAT_UNKNOWN;

}

static SDL_Surface *create(int width, int height, Uint32 id)
{

    SDL_Surface *surface = calloc(1, sizeof (SDL_Surface));
    unsigned int i;

    if (!surface)
        return 0;

    surface->format = &surface->ownformat;
    surface->ownpalette.ncolors = 256;
    surface->ownpalette.colors = surface->colors;

    setformat(surface->format, id);

    if (id == SDL_PIXELFORMAT_INDEX8)
        surface->format->palette = &surface->ownpalette;

    for (i = 0; i < 256; i++)
        surface->colors[i].a = SDL_ALPHA_OPAQUE;

    surface->w = width;
    surface->h = height;
    surface->pitch = (width * surface->format->BytesPerPixel + 3) & ~3;
    surface->pixels = calloc(1, surface->pitch * height + 1);
    surface->alphamod = 255;
    surface->blendmode = (surface->format->Amask) ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE;

    if (!surface->pixels)
    {

        free(surface);

        return 0;

    }

    SDL_SetClipRect(surface, 0);

    return surface;

}

static Uint32 readraw(SDL_Surface *surface, int x, int y)
{

    unsigned char *p = (unsigned char *)surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel;

    switch (surface->format->BytesPerPixel)
    {

    case 1:
        return p[0];

    case 3:
        return p[0] | (p[1] << 8) | (p[2] << 16);

    }

    return ((Uint32 *)p)[0];

}

static void writeraw(SDL_Surface *surface, int x, int y, Uint32 value)
{

    unsigned char *p = (unsigned char *)surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel;

    switch (surface->format->BytesPerPixel)
    {

    case 1:
        p[0] = value;

        break;

    case 3:
        p[0] = value;
        p[1] = value >> 8;
        p[2] = value >> 16;

        break;

    default:
        ((Uint32 *)p)[0] = value;

        break;

    }

}

static void decode(SDL_PixelFormat *format, Uint32 raw, struct pixel *pixel)
{

    if (format->palette)
    {

        SDL_Color *color = &format->palette->colors[raw & 0xFF];

        pixel->r = color->r;
        pixel->g = color->g;
        pixel->b = color->b;
        pixel->a = color->a;
        pixel->index = raw;

    }

    else
    {

        pixel->r = (raw & format->Rmask) >> format->Rshift;
        pixel->g = (raw & format->Gmask) >> format->Gshift;
        pixel->b = (raw & format->Bmask) >> format->Bshift;
        pixel->a = (format->Amask) ? (raw & format->Amask) >> format->Ashift : 255;

    }

}

static Uint32 nearest(SDL_Palette *palette, unsigned int r, unsigned int g, unsigned int b)
{

    unsigned int best = 0;
    unsigned int bestdistance = 0xFFFFFFFF;
    int i;

    for (i = 0; i < palette->ncolors; i++)
    {

        int dr = (int)palette->colors[i].r - (int)r;
        int dg = (int)palette->colors[i].g - (int)g;
        int db = (int)palette->colors[i].b - (int)b;
        unsigned int distance = dr * dr + dg * dg + db * db;

        if (distance < bestdistance)
        {

            best = i;
            bestdistance = distance;

        }

    }

    return best;

}

static Uint32 encode(SDL_PixelFormat *format, unsigned int r, unsigned int g, unsigned int b, unsigned int a)
{

    if (format->palette)
        return nearest(format->palette, r, g, b);

    return (r << format->Rshift) | (g << format->Gshift) | (b << format->Bshift) | ((format->Amask) ? a << format->Ashift : 0);

}

/* the pixel a blit or a conversion takes from source, with a colorkeyed pixel skipped (blit) or made transparent (convert) */
static void readpixel(SDL_Surface *source, int x, int y, struct pixel *pixel)
{

    Uint32 raw = readraw(source, x, y);

    decode(source->format, raw, pixel);

    pixel->skip = source->hascolorkey && raw == source->colorkey;

    if (source->format->palette)
        pixel->a = 255;

}

static void writepixel(SDL_Surface *target, int x, int y, struct pixel *pixel, unsigned int blend, unsigned int sameindex)
{

    unsigned int a = pixel->a;

    if (sameindex)
    {

        writeraw(target, x, y, pixel->index);

        return;

    }

    if (blend && a < 255)
    {

        struct pixel old;

        if (!a)
            return;

        decode(target->format, readraw(target, x, y), &old);

        writeraw(target, x, y, encode(target->format, (pixel->r * a + old.r * (255 - a)) / 255, (pixel->g * a + old.g * (255 - a)) / 255, (pixel->b * a + old.b * (255 - a)) / 255, a + old.a * (255 - a) / 255));

        return;

    }

    writeraw(target, x, y, encode(target->format, pixel->r, pixel->g, pixel->b, a));

}

SDL_Surface *SDL_CreateRGBSurface(Uint32 flags, int width, int height, int depth, Uint32 rmask, Uint32 gmask, Uint32 bmask, Uint32 amask)
{

    return create(width, height, formatfrommasks(depth, rmask, amask));

}

SDL_Surface *SDL_ConvertSurfaceFormat(SDL_Surface *surface, Uint32 format, Uint32 flags)
{

    SDL_Surface *out = create(surface->w, surface->h, format);
    int x;
    int y;

    if (!out)
        return 0;

    if (surface->format->palette && out->format->palette)
        SDL_SetPaletteColors(out->format->palette, surface->format->palette->colors, 0, 256);

    for (y = 0; y < surface->h; y++)
    {

        for (x = 0; x < surface->w; x++)
        {

            struct pixel pixel;

            readpixel(surface, x, y, &pixel);

            if (surface->format->palette)
                pixel.a = surface->format->palette->colors[pixel.index].a;

            if (pixel.skip && out->format->Amask)
                pixel.a = 0;

            writepixel(out, x, y, &pixel, 0, surface->format->palette && out->format->palette);

        }

    }

    if (surface->hascolorkey && !out->format->Amask)
    {

        struct pixel key;

        decode(surface->format, surface->colorkey, &key);

        out->hascolorkey = 1;
        out->colorkey = (out->format->palette) ? surface->colorkey : encode(out->format, key.r, key.g, key.b, 255);

    }

    return out;

}

SDL_Surface *SDL_ConvertSurface(SDL_Surface *surface, const SDL_PixelFormat *format, Uint32 flags)
{

    return SDL_ConvertSurfaceFormat(surface, format->format, flags);

}

void SDL_FreeSurface(SDL_Surface *surface)
{

    if (!surface)
        return;

    free(surface->pixels);
    free(surface);

}

int SDL_LockSurface(SDL_Surface *surface)
{

    return 0;

}

void SDL_UnlockSurface(SDL_Surface *surface)
{

}

int SDL_SetColorKey(SDL_Surface *surface, int flag, Uint32 key)
{

    surface->hascolorkey = flag;
    surface->colorkey = key;

    return 0;

}

int SDL_SetSurfaceBlendMode(SDL_Surface *surface, int mode)
{

    surface->blendmode = mode;

    return 0;

}

int SDL_SetSurfaceAlphaMod(SDL_Surface *surface, Uint8 alpha)
{

    surface->alphamod = alpha;

    return 0;

}

int SDL_SetSurfacePalette(SDL_Surface *surface, SDL_Palette *palette)
{

    if (!surface->format->palette || !palette)
        return -1;

    return SDL_SetPaletteColors(surface->format->palette, palette->colors, 0, palette->ncolors);

}

int SDL_SetPaletteColors(SDL_Palette *palette, const SDL_Color *colors, int first, int count)
{

    int i;

    for (i = 0; i < count && first + i < palette->ncolors; i++)
        palette->colors[first + i] = colors[i];

    return 0;

}

unsigned int SDL_SetClipRect(SDL_Surface *surface, const SDL_Rect *rect)
{

    SDL_Rect *clip = &surface->clip_rect;
    int right;
    int bottom;

    if (!rect)
    {

        clip->x = 0;
        clip->y = 0;
        clip->w = surface->w;
        clip->h = surface->h;

        return 1;

    }

    clip->x = (rect->x > 0) ? rect->x : 0;
    clip->y = (rect->y > 0) ? rect->y : 0;
    right = (rect->x + rect->w < surface->w) ? rect->x + rect->w : surface->w;
    bottom = (rect->y + rect->h < surface->h) ? rect->y + rect->h : surface->h;
    clip->w = (right > clip->x) ? right - clip->x : 0;
    clip->h = (bottom > clip->y) ? bottom - clip->y : 0;

    return clip->w && clip->h;

}

Uint32 SDL_MapRGB(const SDL_PixelFormat *format, Uint8 r, Uint8 g, Uint8 b)
{

    return encode((SDL_PixelFormat *)format, r, g, b, 255);

}

Uint32 SDL_MapRGBA(const SDL_PixelFormat *format, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{

    return encode((SDL_PixelFormat *)format, r, g, b, a);

}

int SDL_FillRect(SDL_Surface *surface, const SDL_Rect *rect, Uint32 color)
{

    SDL_Rect *clip = &surface->clip_rect;
    int x0 = clip->x;
    int y0 = clip->y;
    int x1 = clip->x + clip->w;
    int y1 = clip->y + clip->h;
    int x;
    int y;

    if (rect)
    {

        if (rect->x > x0)
            x0 = rect->x;

        if (rect->y > y0)
            y0 = rect->y;

        if (rect->x + rect->w < x1)
            x1 = rect->x + rect->w;

        if (rect->y + rect->h < y1)
            y1 = rect->y + rect->h;

    }

    for (y = y0; y < y1; y++)
    {

        for (x = x0; x < x1; x++)
            writeraw(surface, x, y, color);

    }

    return 0;

}

int SDL_BlitSurface(SDL_Surface *source, const SDL_Rect *sourcerect, SDL_Surface *target, SDL_Rect *targetrect)
{

    SDL_Rect *clip = &target->clip_rect;
    unsigned int sameindex = source->format->palette && target->format->palette;
    unsigned int blend = source->blendmode == SDL_BLENDMODE_BLEND;
    int sx = (sourcerect) ? sourcerect->x : 0;
    int sy = (sourcerect) ? sourcerect->y : 0;
    int w = (sourcerect) ? sourcerect->w : source->w;
    int h = (sourcerect) ? sourcerect->h : source->h;
    int tx = (targetrect) ? targetrect->x : 0;
    int ty = (targetrect) ? targetrect->y : 0;
    int x;
    int y;

    if (sx < 0)
    {

        w += sx;
        tx -= sx;
        sx = 0;

    }

    if (sy < 0)
    {

        h += sy;
        ty -= sy;
        sy = 0;

    }

    if (sx + w > source->w)
        w = source->w - sx;

    if (sy + h > source->h)
        h = source->h - sy;

    if (tx < clip->x)
    {

        w -= clip->x - tx;
        sx += clip->x - tx;
        tx = clip->x;

    }

    if (ty < clip->y)
    {

        h -= clip->y - ty;
        sy += clip->y - ty;
        ty = clip->y;

    }

    if (tx + w > clip->x + clip->w)
        w = clip->x + clip->w - tx;

    if (ty + h > clip->y + clip->h)
        h = clip->y + clip->h - ty;

    if (targetrect)
    {

        targetrect->x = tx;
        targetrect->y = ty;
        targetrect->w = (w > 0) ? w : 0;
        targetrect->h = (h > 0) ? h : 0;

    }

    if (w <= 0 || h <= 0)
        return 0;

    if (source->format->format == target->format->format && !source->hascolorkey && !(blend && source->format->Amask) && source->alphamod == 255)
    {

        unsigned int bpp = source->format->BytesPerPixel;

        for (y = 0; y < h; y++)
            memmove((unsigned char *)target->pixels + (ty + y) * target->pitch + tx * bpp, (unsigned char *)source->pixels + (sy + y) * source->pitch + sx * bpp, w * bpp);

        return 0;

    }

    for (y = 0; y < h; y++)
    {

        for (x = 0; x < w; x++)
        {

            struct pixel pixel;

            readpixel(source, sx + x, sy + y, &pixel);

            if (pixel.skip)
                continue;

            pixel.a = pixel.a * source->alphamod / 255;

            writepixel(target, tx + x, ty + y, &pixel, blend, sameindex);

        }

    }

    return 0;

}

static unsigned int readpng(struct png *p, void *buffer, unsigned int count, unsigned int offset)
{

    if (offset >= pngcount)
        return 0;

    if (count > pngcount - offset)
        count = pngcount - offset;

    memcpy(buffer, pngdata + offset, count);

    return count;

}

static unsigned int getindex(unsigned char *line, unsigned int x, unsigned int depth)
{

    unsigned int bit = x * depth;

    if (depth == 8)
        return line[x];

    return (line[bit / 8] >> (8 - depth - bit % 8)) & ((1 << depth) - 1);

}

/* colormap images keep their indices, the game picks palettes by index; everything else becomes ARGB8888 */
SDL_Surface *SDL_LoadPNG(const void *data, unsigned int count)
{

    SDL_Surface *surface;
    unsigned int indexed;
    unsigned int x;
    unsigned int y;

    pngdata = data;
    pngcount = count;

    if (!png_open(&png, readpng) || png.width > PNGWIDTH)
        return 0;

    indexed = png.colortype == 3;
    surface = create(png.width, png.height, (indexed) ? SDL_PIXELFORMAT_INDEX8 : SDL_PIXELFORMAT_ARGB8888);

    if (!surface)
        return 0;

    if (indexed)
    {

        for (x = 0; x < png.npalette && x < 256; x++)
        {

            unsigned int color = png.palette[x];

            surface->colors[x].r = color >> 16;
            surface->colors[x].g = color >> 8;
            surface->colors[x].b = color;
            surface->colors[x].a = color >> 24;

        }

    }

    for (y = 0; y < png.height; y++)
    {

        unsigned char *row = (unsigned char *)surface->pixels + y * surface->pitch;

        if (!png_readrow(&png, pngrow))
        {

            SDL_FreeSurface(surface);

            return 0;

        }

        for (x = 0; x < png.width; x++)
        {

            if (indexed)
                row[x] = getindex(png.line, x, png.depth);
            else
                ((Uint32 *)row)[x] = pngrow[x];

        }

    }

    return surface;

}

void SDL_SetTextInputRect(const SDL_Rect *rect)
{

}

void SDL_StartTextInput(void)
{

}

void SDL_StopTextInput(void)
{

}

const char *SDL_GetError(void)
{

    return "";

}

void SDL_Quit(void)
{

}

