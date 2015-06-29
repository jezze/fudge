struct view
{

    struct list_item item;
    struct list windows;
    unsigned int center;
    unsigned int active;
    struct panel panel;
    struct window *windowfocus;

};

void view_draw(struct view *view, unsigned int bpp, unsigned int line);
void view_activate(struct view *view);
void view_deactivate(struct view *view);
void view_init(struct view *view, unsigned int center);
