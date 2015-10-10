struct panel_header
{

    unsigned int active;

};

struct panel
{

    struct renderable base;
    struct panel_header header;

};

void panel_init(struct panel *panel);
