struct panel
{

    unsigned int active;
    struct box size;

};

void panel_draw(struct panel *panel, struct ctrl_videosettings *settings, unsigned int line);
void panel_activate(struct panel *panel);
void panel_deactivate(struct panel *panel);
void panel_init(struct panel *panel);
