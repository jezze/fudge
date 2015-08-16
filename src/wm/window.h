struct window
{

    struct renderable base;
    unsigned int framecolor;

};

void window_init(struct window *window, unsigned int color);
