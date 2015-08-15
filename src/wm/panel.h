struct panel
{

    struct renderable base;
    unsigned int backgroundcolor;
    unsigned int framecolor;

};

void panel_init(struct panel *panel);
