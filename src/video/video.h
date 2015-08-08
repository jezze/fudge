void video_getmode(struct ctrl_videosettings *settings);
void video_setmode(struct ctrl_videosettings *settings);
void video_setcolormap(unsigned int offset, unsigned int size, unsigned int count, void *buffer);
void video_open(void);
void video_close(void);
void video_draw(unsigned int offset, unsigned int count, void *buffer);
