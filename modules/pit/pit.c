#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/vfs.h>
#include <kernel/event.h>
#include <kernel/modules.h>
#include <kernel/kernel.h>
#include <modules/stream/stream.h>
#include <modules/pit/pit.h>

static struct pit_device pitDevice;

static void pit_handle_irq()
{

    struct pit_device *pit = &pitDevice;

    pit->jiffies += 1;

    event_handle(EVENT_IRQ_PIT);

}

static unsigned int pit_device_stream_read(struct vfs_node *self, unsigned int count, void *buffer)
{

    char num[32];

    string_write_num(num, pitDevice.jiffies, 10);

    string_write(buffer, num);

    return string_length(buffer);

}

void pit_device_init(struct pit_device *device)
{

    modules_device_init(&device->base, PIT_DEVICE_TYPE);
    stream_device_init(&pitDevice.stream, "jiffies", pit_device_stream_read, 0);
    device->divisor = PIT_HERTZ / PIT_FREQUENCY;
    device->jiffies = 0;

    io_outb(0x43, 0x36);
    io_outb(0x40, (unsigned char)(device->divisor & 0xFF));
    io_outb(0x40, (unsigned char)((device->divisor >> 8) & 0xFF));

}

void init()
{

    pit_device_init(&pitDevice);

    kernel_register_irq(0x00, pit_handle_irq);

    modules_register_device(&pitDevice.stream.base);
    modules_register_device(&pitDevice.base);

}

