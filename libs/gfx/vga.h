struct vga_surface
{

    struct gfx_surface base;
    unsigned int id;

};

void vga_load(struct vga_surface *surface);
void vga_initsurface(struct vga_surface *surface, unsigned int id);
