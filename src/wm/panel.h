struct panel
{

    struct renderable base;
    unsigned int backgroundcolor;
    unsigned int framecolor;

};

void panel_activate(struct panel *panel);
void panel_deactivate(struct panel *panel);
void panel_init(struct panel *panel);
