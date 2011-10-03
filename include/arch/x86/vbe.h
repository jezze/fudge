#ifndef ARCH_VBE_H
#define ARCH_VBE_H

struct vbe_controller_info
{

    char signature[4];
    short version;
    short oem[2];
    unsigned int capabilities[4];
    short modes[2];
    short memory;

};

struct vbe_mode_info
{

    short attributes;
    char winA;
    char winB;
    short granularity;
    short winSize;
    short segmentA;
    short segmentB;

};

#endif

