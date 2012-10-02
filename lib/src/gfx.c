#include <call.h>
#include <gfx.h>
#include <memory.h>

void gfx_set_color(struct gfx_surface *self, unsigned int color)
{

    self->context.color = color;

}

void gfx_fill(struct gfx_surface *self)
{

    char buffer[0x1000];
    unsigned int size = self->width * self->height * self->bpp;
    unsigned int i;

    for (i = 0; i < 0x1000; i += 4)
        memory_copy(buffer + i, &self->context.color, 4);

    for (i = 0; i < size; i += 0x1000)
        self->backend->write(self->backend, i, 0x1000, buffer);

}

void backend_read(struct gfx_backend *self, unsigned int offset, unsigned int count, void *buffer)
{

    call_read(self->id, offset, count, buffer);

}

void backend_write(struct gfx_backend *self, unsigned int offset, unsigned int count, void *buffer)
{

    call_write(self->id, offset, count, buffer);

}

void gfx_init_backend(struct gfx_backend *backend, unsigned int id)
{

    memory_clear(backend, sizeof (struct gfx_backend));

    backend->id = id;
    backend->read = backend_read;
    backend->write = backend_write;

}

void gfx_init_surface(struct gfx_surface *surface, unsigned int width, unsigned int height, enum gfx_surface_type type, struct gfx_backend *backend)
{

    memory_clear(surface, sizeof (struct gfx_surface));

    surface->width = width;
    surface->height = height;
    surface->bpp = 4;
    surface->type = type;
    surface->backend = backend;

}

void gfx_init_window(struct gfx_window *window, unsigned int x, unsigned int y, struct gfx_surface *surface, void (*draw)(struct gfx_window *self))
{

    memory_clear(window, sizeof (struct gfx_window));

    window->x = x;
    window->y = y;
    window->surface = surface;
    window->draw = draw;

}

