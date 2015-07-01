struct window
{

    struct list_item item;
    unsigned int active;
    struct box size;
    struct box screen;
    unsigned int source;

};

void window_draw(struct window *window, unsigned int bpp, unsigned int line);
void window_activate(struct window *window);
void window_deactivate(struct window *window);
void window_init(struct window *window);
