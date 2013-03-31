struct vga_surface
{

    struct gfx_surface base;

};

void vga_load(struct vga_surface *surface);
void vga_init_surface(struct vga_surface *surface);
