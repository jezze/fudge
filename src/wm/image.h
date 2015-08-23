struct image
{

    struct renderable base;
    unsigned char *data;

};

void image_render(struct renderable *self, struct ctrl_videosettings *settings, void *drawdata, void *fontdata, unsigned int line);
void image_init(struct image *image, void *data);
