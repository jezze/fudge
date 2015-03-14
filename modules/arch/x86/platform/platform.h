#define PLATFORM_BUS                    0x0001
#define PLATFORM_PIT                    0x0001
#define PLATFORM_RTC                    0x0002
#define PLATFORM_UART1                  0x0003
#define PLATFORM_UART2                  0x0004
#define PLATFORM_UART3                  0x0005
#define PLATFORM_UART4                  0x0006

unsigned short platform_getbase(unsigned int id);
unsigned short platform_getirq(unsigned int id);
