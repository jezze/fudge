struct view
{

    struct list_item item;
    struct list windows;
    unsigned int center;
    unsigned int active;
    struct panel panel;
    struct window *windowactive;

};

void view_draw(struct view *view, struct ctrl_videosettings *settings, unsigned int line);
void view_init(struct view *view, unsigned int center);
