struct panel
{

    unsigned int active;
    struct box size;
    struct text title;
    unsigned int border;

};

void panel_draw(struct panel *panel);
void panel_init(struct panel *panel, char *text, unsigned int active, unsigned int border);
