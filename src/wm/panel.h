struct panel
{

    unsigned int active;
    struct box size;

};

void panel_draw(struct panel *panel, unsigned int line);
void panel_init(struct panel *panel);
