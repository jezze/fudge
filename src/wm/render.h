void render_update(unsigned int descriptor, struct ctrl_videosettings *settings);
void render_parse(unsigned int descriptor);
void render_complete(void);
void render_initvideo(unsigned int descriptor, struct ctrl_videosettings *settings);
void render_initcolormap(unsigned int descriptor);
void render_initfont(unsigned int descriptor, unsigned int lineheight);
void render_init();
