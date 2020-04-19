#include <fudge.h>
#include <kernel.h>
#include <modules/base/base.h>
#include <modules/system/system.h>

struct vbe_mode_info
{

    unsigned short attributes;
    unsigned char window_a;
    unsigned char window_b;
    unsigned short granularity;
    unsigned short window_size;
    unsigned short segment_a;
    unsigned short segment_b;
    unsigned int win_func_ptr;
    unsigned short pitch;
    unsigned short width;
    unsigned short height;
    unsigned char w_char;
    unsigned char y_char;
    unsigned char planes;
    unsigned char bpp;
    unsigned char banks;
    unsigned char memory_model;
    unsigned char bank_size;
    unsigned char image_pages;
    unsigned char reserved0;
    unsigned char red_mask;
    unsigned char red_position;
    unsigned char green_mask;
    unsigned char green_position;
    unsigned char blue_mask;
    unsigned char blue_position;
    unsigned char reserved_mask;
    unsigned char reserved_position;
    unsigned char direct_color_attributes;
    unsigned int framebuffer;
    unsigned int off_screen_mem_off;
    unsigned short off_screen_mem_size;

} __attribute__ ((packed));

extern void *realmode_gdt;
extern void _get_video_mode(int);
extern void _set_video_mode(int);

void module_init(void)
{

    struct vbe_mode_info *info = (struct vbe_mode_info *)0xd000;
    int (*getmode)(int) = (int (*)(int))0x8000;
    int (*setmode)(int) = (int (*)(int))0x8000;
    unsigned int mode_offset;

    debug_logs(DEBUG_INFO, "vbe loaded");
    memory_copy((void *)0x8000, (void *)(unsigned int)_get_video_mode, 0x1000);
    memory_copy((void *)0x9000, &realmode_gdt, 0x1000);
    getmode(0);
    debug_logs(DEBUG_INFO, "vbe worked!");
    debug_log16(DEBUG_INFO, "vbe width", info->width);
    debug_log16(DEBUG_INFO, "vbe height", info->height);
    debug_log8(DEBUG_INFO, "vbe bpp", info->bpp);

    memory_copy((void *)0x8000, (void *)(unsigned int)_set_video_mode, 0x1000);
    memory_copy((void *)0x9000, &realmode_gdt, 0x1000);
    setmode(0);
    debug_logs(DEBUG_INFO, "vbe worked!");

    for (mode_offset = 0; getmode(mode_offset); mode_offset += 2)
    {


    }

}

void module_register(void)
{

}

void module_unregister(void)
{

}

