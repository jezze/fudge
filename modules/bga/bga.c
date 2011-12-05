#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/modules.h>
#include <modules/bga/bga.h>

#define BGA_INDEX 0x01CE
#define BGA_DATA  0x01CF

#define BGA_INDEX_ID          0
#define BGA_INDEX_XRES        1
#define BGA_INDEX_YRES        2
#define BGA_INDEX_BPP         3
#define BGA_INDEX_ENABLE      4
#define BGA_INDEX_BANK        5
#define BGA_INDEX_VIRT_WIDTH  6
#define BGA_INDEX_VIRT_HEIGHT 7
#define BGA_INDEX_XOFF        8
#define BGA_INDEX_YOFF        9

static struct bga_device device;

static void write_register(unsigned short index, unsigned short data)
{

    io_outw(BGA_INDEX, index);
    io_outw(BGA_DATA, data);

}

static unsigned short read_register(unsigned short index)
{

    io_outw(BGA_INDEX, index);

    return io_inw(BGA_DATA);

}

static void set_mode(unsigned int width, unsigned int height, unsigned int depth, unsigned int linear, unsigned int clear)
{

    write_register(BGA_INDEX_ENABLE, 0);
    write_register(BGA_INDEX_XRES, width);
    write_register(BGA_INDEX_YRES, height);
    write_register(BGA_INDEX_YRES, height);
    write_register(BGA_INDEX_BPP, depth);
    write_register(BGA_INDEX_ENABLE, 1);

}

void bga_device_init(struct bga_device *device)
{

    modules_device_init(&device->base, BGA_DEVICE_TYPE);

//    set_mode(640, 480, 24, 0, 0);

}

void init()
{

    bga_device_init(&device);

    modules_register_device(&device.base);

}

void destroy()
{

    modules_unregister_device(&device.base);

}

