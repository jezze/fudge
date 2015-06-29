struct mouse
{

    struct box size;

};

void mouse_draw(struct mouse *mouse, unsigned int bpp, unsigned int line);
void mouse_init(struct mouse *mouse, struct box *screen);
