struct vga_driver
{

    struct base_driver base;
    struct video_interface ivideo;
    struct terminal_interface iterminal;
    struct {unsigned char color; unsigned short offset;} cursor;

};

void vga_init_driver(struct vga_driver *driver);
