void vbe_begin16(void);
void vbe_end16(void);
struct vbe_info *vbe_getinfo(void);
struct vbe_mode *vbe_getvideomode(unsigned short modenum);
void vbe_setvideomode(unsigned short modenum);
void vbe_getedid(void);
