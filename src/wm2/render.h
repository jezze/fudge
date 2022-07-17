struct render_damage
{

    unsigned int state;
    struct position position0;
    struct position position1;

};

struct render_display
{

    void *framebuffer;
    struct size size;
    unsigned int bpp;
    struct render_damage damage;

};

void render_damage(struct render_display *display, int x0, int y0, int x1, int y1);
void render_paint(struct render_display *display, struct mouse *mouse);
