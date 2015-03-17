struct window
{

    struct list_item item;
    unsigned int active;
    struct box size;
    struct text title;
    unsigned int border;

};

void window_draw(struct window *window);
void window_init(struct window *window, char *text, unsigned int active, unsigned int border);
