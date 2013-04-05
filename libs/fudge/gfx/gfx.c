#include "../call.h"
#include "../define.h"
#include "../memory.h"
#include "gfx.h"

static void gfx_fill_rectangle(struct gfx_surface *self)
{

    char buffer[FUDGE_BSIZE];
    unsigned int line = self->width * self->bpp;
    unsigned int recline = self->context.width * self->bpp;
    unsigned int size = line * self->context.height;
    unsigned int offset = (self->context.y * self->width + self->context.x) * self->bpp;
    unsigned int i;

    for (i = 0; i < recline; i += self->bpp)
        memory_copy(buffer + i, &self->context.color, self->bpp);

    for (i = offset; i < size; i += line)
        self->write(self, i, recline, buffer);

}

void gfx_fill(struct gfx_surface *self)
{

    switch (self->context.primitive)
    {

        case GFX_RECTANGLE:

            gfx_fill_rectangle(self);

            break;

        default:

            break;

    }

}

void gfx_set_color(struct gfx_surface *self, unsigned int color)
{

    self->context.color = color;

}

void gfx_set_color_rgb(struct gfx_surface *self, unsigned char red, unsigned char green, unsigned char blue)
{

    self->context.color = (red << 24) | (green << 16) | (blue << 8) | 0xFF;

}

void gfx_set_color_rgba(struct gfx_surface *self, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
{

    self->context.color = (red << 24) | (green << 16) | (blue << 8) | alpha;

}

void gfx_set_rectangle(struct gfx_surface *surface, unsigned int x, unsigned int y, unsigned int width, unsigned int height)
{

    surface->context.primitive = GFX_RECTANGLE;
    surface->context.x = x;
    surface->context.y = y;
    surface->context.width = width;
    surface->context.height = height;

}

void gfx_write_surface(unsigned int id, struct gfx_surface *in)
{

    unsigned int offset = 0;
    unsigned int scanline;

    if (in->width > FUDGE_BSIZE)
        return;

    for (scanline = 0; scanline < in->height; scanline++)
    {

        char buffer[FUDGE_BSIZE];

        offset += in->read(in, offset, in->width, buffer);

        call_write(id, scanline * in->width, in->width, buffer);

    }

}

void gfx_init_context(struct gfx_context *context)
{

    memory_clear(context, sizeof (struct gfx_context));

    context->color = 0xFFFFFF;
    context->primitive = GFX_NONE;

}

void gfx_init_surface(struct gfx_surface *surface, unsigned int (*read)(struct gfx_surface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct gfx_surface *self, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(surface, sizeof (struct gfx_surface));
    gfx_init_context(&surface->context);

    surface->read = read;
    surface->write = write;

}

