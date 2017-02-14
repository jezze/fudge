void render_update(unsigned int descriptor, unsigned int w, unsigned int h);
void render_parse(unsigned int descriptor);
void render_complete(void);
void render_setvideo(unsigned int descriptor, unsigned int w, unsigned int h, unsigned int bpp);
void render_setpaint(unsigned int bpp);
void render_setcolormap(unsigned int descriptor);
void render_setmouse(unsigned int size);
void render_setfont(unsigned int descriptor, unsigned int lineheight);
void render_init();
