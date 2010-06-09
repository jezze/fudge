#ifndef VBE_H
#define VBE_H

typedef struct vbe_controller
{

    char signature[4];
    short version;
    short oemString[2];
    unsigned char capabilities[4];
    short modes[2];
    short totalMemory;


} vbe_controller_t;

extern uint32_t sum(uint32_t a1, uint32_t a2);
extern void vbe_set_controller(uint32_t num);
extern void vbe_init();

#endif
