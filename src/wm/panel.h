struct panel
{

    struct renderable base;
    unsigned int backgroundcolor;
    unsigned int framecolor;

};

void panel_render(struct renderable *self, struct ctrl_videosettings *settings, void *drawdata, void *fontdata, unsigned int line);
void panel_init(struct panel *panel);
