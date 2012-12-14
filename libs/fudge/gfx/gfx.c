#include "../memory.h"
#include "gfx.h"

static void gfx_fill_rectangle(struct gfx_surface *self)
{

    char buffer[GFX_BUFFER_SIZE];
    unsigned int line = self->width * self->bpp;
    unsigned int recline = self->context.width * self->bpp;
    unsigned int size = line * self->context.height;
    unsigned int offset = (self->context.y * self->width + self->context.x) * self->bpp;
    unsigned int i;

    for (i = 0; i < recline; i += self->bpp)
        memory_copy(buffer + i, &self->context.color, self->bpp);

    for (i = offset; i < size; i += line)
        self->backend->write(self->backend, i, recline, buffer);

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

void gfx_init_backend(struct gfx_backend *backend, unsigned int id, void (*read)(struct gfx_backend *self, unsigned int offset, unsigned int count, void *buffer), void (*write)(struct gfx_backend *self, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(backend, sizeof (struct gfx_backend));

    backend->id = id;
    backend->read = read;
    backend->write = write;

}

void gfx_init_context(struct gfx_context *context)
{

    memory_clear(context, sizeof (struct gfx_context));

    context->color = 0xFFFFFF;
    context->primitive = GFX_NONE;

}

void gfx_init_surface(struct gfx_surface *surface, unsigned int width, unsigned int height, enum gfx_surface_type type, struct gfx_backend *backend)
{

    memory_clear(surface, sizeof (struct gfx_surface));

    gfx_init_context(&surface->context);

    surface->width = width;
    surface->height = height;
    surface->bpp = 4;
    surface->type = type;
    surface->backend = backend;

}

