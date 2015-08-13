struct view
{

    struct list_item item;
    struct list clients;
    unsigned int center;
    struct panel panel;
    struct box body;
    struct client *clientfocus;

};

void view_activate(struct view *view);
void view_deactivate(struct view *view);
void view_init(struct view *view, struct box *screen, struct box *menu, unsigned int num, unsigned int total);
