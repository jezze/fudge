struct pcx_surface
{

    struct gfx_surface base;
    unsigned int id;
    struct pcx_header header;
    unsigned char colormap[768];

};

void pcx_load(struct pcx_surface *surface);
void pcx_initsurface(struct pcx_surface *surface, unsigned int id);
