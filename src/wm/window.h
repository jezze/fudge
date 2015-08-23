struct window
{

    struct renderable base;
    unsigned int framecolor;

};

void window_render(struct renderable *self, struct ctrl_videosettings *settings, void *drawdata, void *fontdata, unsigned int line);
void window_init(struct window *window, unsigned int color);
