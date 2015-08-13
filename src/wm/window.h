struct window
{

    struct renderable base;
    unsigned int active;

};

void window_activate(struct window *window);
void window_deactivate(struct window *window);
void window_init(struct window *window);
