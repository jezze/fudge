void render_setmouse(int x, int y);
void render_place(struct widget *widget, struct util_region *region);
void render_damage(int x0, int y0, int x2, int y2);
void render_undamage(void);
void render_update(struct blit_display *display);
void render_init(void);
