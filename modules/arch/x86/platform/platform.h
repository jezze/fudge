#define PLATFORM_PIT_IRQ                0x00
#define PLATFORM_PIT_DEVICE_TYPE        0x8601
#define PLATFORM_RTC_IRQ                0x08
#define PLATFORM_RTC_DEVICE_TYPE        0x8602
#define PLATFORM_UART1_IRQ              0x04
#define PLATFORM_UART1_DEVICE_TYPE      0x8603
#define PLATFORM_UART2_IRQ              0x03
#define PLATFORM_UART2_DEVICE_TYPE      0x8604

struct platform_device
{

    struct base_device base;
    unsigned short registers;

};

struct platform_bus
{

    struct base_bus base;

};

void platform_init_bus(struct platform_bus *bus);
void platform_init_device(struct platform_device *device, unsigned int type, char *name, struct platform_bus *bus, unsigned short registers);
