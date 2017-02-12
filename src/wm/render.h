void render_update(unsigned int descriptor, unsigned int w, unsigned int h);
void render_parse(unsigned int descriptor);
void render_complete(void);
void render_initvideo(unsigned int descriptor, unsigned int w, unsigned int h, unsigned int bpp);
void render_initpaint(unsigned int bpp);
void render_initcolormap(unsigned int descriptor);
void render_initmouse(unsigned int size);
void render_initfont(unsigned int descriptor, unsigned int lineheight);
void render_init();
