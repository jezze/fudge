#include <memory.h>
#include <modules.h>
#include <vfs.h>
#include <nodefs/nodefs.h>
#include <arch/x86/rtc/rtc.h>

static struct rtc_device device;
static struct nodefs_node timestamp;

static unsigned int timestamp_read(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    memory_copy(buffer, 0, 0);

    return 0;

}

void init()
{

    rtc_device_init(&device, RTC_IRQ);
    modules_register_device(&device.base);

    nodefs_register_node(&timestamp, "rtc_timestamp", &device.base.base, timestamp_read, 0);

}

void destroy()
{

    modules_unregister_device(&device.base);

}

