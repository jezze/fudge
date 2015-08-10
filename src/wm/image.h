struct image
{

    struct box size;
    unsigned char *data;

};

void image_draw(struct image *image, struct ctrl_videosettings *settings, unsigned int line);
void image_init(struct image *image, void *data);
