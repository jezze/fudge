struct renderable
{

    struct list_item item;
    struct box size;
    void *data;
    unsigned int visible;
    void (*render)(struct renderable *self, struct ctrl_videosettings *settings, unsigned int line);

};

void renderable_render(struct renderable *renderable, struct ctrl_videosettings *settings, unsigned int line);
void renderable_init(struct renderable *renderable, void *data, void (*render)(struct renderable *self, struct ctrl_videosettings *settings, unsigned int line));
