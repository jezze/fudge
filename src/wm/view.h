struct view
{

    struct list_item item;
    struct list clients;
    unsigned int center;
    struct panel panel;
    struct glyph number;
    struct client *clientfocus;

};

void view_activate(struct view *view);
void view_deactivate(struct view *view);
void view_init(struct view *view, struct box *menu, struct box *body, unsigned int num, unsigned int total, unsigned char *fontdata);
