#define PLATFORM_BUS_TYPE               0x0001
#define PLATFORM_PIT_DEVICE_TYPE        0x8601
#define PLATFORM_RTC_DEVICE_TYPE        0x8602
#define PLATFORM_UART1_DEVICE_TYPE      0x8603
#define PLATFORM_UART2_DEVICE_TYPE      0x8604
#define PLATFORM_UART3_DEVICE_TYPE      0x8605
#define PLATFORM_UART4_DEVICE_TYPE      0x8606

unsigned short platform_bus_get_base(struct base_bus *bus, unsigned int type);
