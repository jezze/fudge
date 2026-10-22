#define AMIGA_BUS                       0x0008
#define AMIGA_TIMER                     0x0001
#define AMIGA_SERIAL                    0x0002
#define AMIGA_IRQ_PORTS                 3
#define AMIGA_IRQ_RBF                   11

unsigned int amiga_setroutine(unsigned int irq, void (*routine)(unsigned int irq));
unsigned int amiga_unsetroutine(unsigned int irq, void (*routine)(unsigned int irq));
