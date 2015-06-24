struct mouse
{

    struct box size;

};

void mouse_draw(struct mouse *mouse, struct ctrl_videosettings *settings, unsigned int line);
void mouse_init(struct mouse *mouse);
