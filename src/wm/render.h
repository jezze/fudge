void render_flush(unsigned int descriptor);
void render_write(void *buffer, unsigned int count);
void render_complete(void);
void render_setvideo(unsigned int descriptor, unsigned int w, unsigned int h, unsigned int bpp);
void render_setdraw(unsigned int w, unsigned int h, unsigned int bpp);
void render_setcolormap(unsigned int descriptor);
void render_setmouse(struct widget_mouse *mouse, unsigned int size);
void render_setfont(unsigned int descriptor, unsigned int lineheight, unsigned int padding);
void render_init();
