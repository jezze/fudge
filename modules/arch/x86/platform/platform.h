#define PLATFORM_BUS_TYPE               0x0001
#define PLATFORM_PIT_DEVICE_TYPE        0x0001
#define PLATFORM_RTC_DEVICE_TYPE        0x0002
#define PLATFORM_UART1_DEVICE_TYPE      0x0003
#define PLATFORM_UART2_DEVICE_TYPE      0x0004
#define PLATFORM_UART3_DEVICE_TYPE      0x0005
#define PLATFORM_UART4_DEVICE_TYPE      0x0006

unsigned short platform_getbase(struct base_bus *bus, unsigned int id);
