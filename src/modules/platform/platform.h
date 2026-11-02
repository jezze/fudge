#define PLATFORM_BUS                    0x0001

unsigned long platform_getbase(unsigned int id);
unsigned int platform_getirq(unsigned int id);
unsigned int platform_setroutine(unsigned int irq, void (*routine)(unsigned int irq));
unsigned int platform_unsetroutine(unsigned int irq, void (*routine)(unsigned int irq));
