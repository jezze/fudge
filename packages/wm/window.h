struct window
{

    struct list_item item;
    unsigned int active;
    struct box size;
    struct text title;

};

void window_draw(struct window *window, unsigned int line);
void window_init(struct window *window, char *text, unsigned int active);
