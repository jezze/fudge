#define RENDER_WINDOWBUTTON_TITLE       0
#define RENDER_WINDOWBUTTON_MENU        1
#define RENDER_WINDOWBUTTON_MINIMIZE    2
#define RENDER_WINDOWBUTTON_CLOSE       3

struct util_size render_getwindowminsize(struct widget *widget, struct util_size *limit);
struct util_size render_getwindowsize(struct widget *widget, struct util_size *limit);
struct util_region render_getwindowbutton(struct widget *widget, unsigned int button);
void render_setmouse(int x, int y, unsigned int type);
void render_place(struct widget *widget, struct util_region *placement);
void render_damage(int x0, int y0, int x2, int y2);
void render_undamage(void);
void render_update(struct blit_display *display);
void render_init(void);
