#define VIRT_BUS                        0x0007
#define VIRT_TIMER                      0x0001
#define VIRT_RTC                        0x0002
#define VIRT_UART0                      0x0003

unsigned long virt_getbase(unsigned int id);
unsigned int virt_getirq(unsigned int id);
unsigned int virt_setroutine(unsigned int irq, void (*routine)(unsigned int irq));
unsigned int virt_unsetroutine(unsigned int irq, void (*routine)(unsigned int irq));
