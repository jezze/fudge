void video_getmode(unsigned int descriptor, struct ctrl_videosettings *settings);
void video_setmode(unsigned int descriptor, struct ctrl_videosettings *settings);
void video_setcolormap(unsigned int descriptor, unsigned int offset, unsigned int count, void *buffer);
void video_open(unsigned int descriptor);
void video_close(unsigned int descriptor);
void video_draw(unsigned int descriptor, unsigned int offset, unsigned int count, void *buffer);
