struct image
{

    struct renderable base;
    unsigned char *data;

};

void image_init(struct image *image, void *data);
