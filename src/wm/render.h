#define RENDER_AREA_STATE_NONE          0
#define RENDER_AREA_STATE_MADE          1

struct render_area
{

    unsigned int state;
    struct util_position position0;
    struct util_position position2;

};

void render_damage(struct render_area *area, int x0, int y0, int x2, int y2);
void render_undamage(struct render_area *area);
void render(struct blit_display *display, struct render_area *area, int mx, int my);
