struct view
{

    struct list_item item;
    unsigned int active;
    struct panel panel;
    struct list windows;
    struct window *windowactive;

};

void view_addwindow(struct view *v, struct window *w);
void view_init(struct view *view, char *text, unsigned int active);
