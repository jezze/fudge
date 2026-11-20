#define RENDER_DIRECTION_NONE           0
#define RENDER_DIRECTION_HORIZONTAL     1
#define RENDER_DIRECTION_VERTICAL       2
#define RENDER_WINDOWBUTTON_TITLE       0
#define RENDER_WINDOWBUTTON_MENU        1
#define RENDER_WINDOWBUTTON_MINIMIZE    2
#define RENDER_WINDOWBUTTON_CLOSE       3

struct util_size render_getwindowminsize(struct widget *widget, struct util_size *limit);
struct util_size render_getwindowsize(struct widget *widget, struct util_size *limit);
unsigned int render_getdirection(unsigned int flow);
struct util_region render_getcursor(struct widget *widget, unsigned int index);
struct util_region render_getcyclearrow(struct widget *widget, unsigned int forward);
struct util_region render_getwindowbutton(struct widget *widget, unsigned int button);
void render_setstate(struct widget *hover, struct widget *focus, struct widget *window);
void render_setmouse(int x, int y, unsigned int type);
void render_place(struct widget *widget, struct util_region *placement, struct util_region *clip);
void render_damage(int x0, int y0, int x2, int y2);
void render_undamage(void);
struct util_region render_update(struct blit_display *display);
unsigned int render_unit(unsigned int count);
void render_setunit(unsigned int size);
void render_init(void);
