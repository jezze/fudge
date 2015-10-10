struct window_header
{

    unsigned int active;

};

struct window
{

    struct renderable base;
    struct window_header header;

};

void window_init(struct window *window);
