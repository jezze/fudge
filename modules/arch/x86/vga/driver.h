struct vga_driver
{

    struct base_driver base;
    struct base_terminal_interface iterminal;
    struct base_video_interface ivideo;
    struct {unsigned char color; unsigned short offset;} cursor;

};

void vga_init_driver(struct vga_driver *driver);
