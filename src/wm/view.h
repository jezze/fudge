struct view
{

    struct list_item item;
    struct list clients;
    unsigned int center;
    struct panel panel;
    struct text number;
    struct client *clientfocus;

};

void view_activate(struct view *view);
void view_deactivate(struct view *view);
void view_init(struct view *view, unsigned int num);
