struct image
{

    struct box size;
    unsigned char *data;

};

void image_draw(struct image *image, unsigned int bpp, unsigned int line);
void image_init(struct image *image, void *data);
