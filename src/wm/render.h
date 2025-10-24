void render_place(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, struct util_region *clip);
void render_damage(int x0, int y0, int x2, int y2);
void render_undamage(void);
void render_cache(void);
void render_update(struct blit_display *display, int mx, int my);
void render_init(void);
