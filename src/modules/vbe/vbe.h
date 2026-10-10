void vbe_lowmemstart(void);
void vbe_lowmemend(void);
struct vbe_info *vbe_getinfo(void);
struct vbe_mode *vbe_getvideomode(unsigned short modenum);
void vbe_setvideomode(unsigned short modenum);
void vbe_getedid(void);
