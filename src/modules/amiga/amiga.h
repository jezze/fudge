#define AMIGA_BUS                       0x0008
#define AMIGA_TIMER                     0x0001
#define AMIGA_SERIAL                    0x0002
#define AMIGA_KEYBOARD                  0x0003
#define AMIGA_MOUSE                     0x0004
#define AMIGA_VIDEO                     0x0005
#define AMIGA_IRQ_PORTS                 3
#define AMIGA_IRQ_VERTB                 5
#define AMIGA_IRQ_RBF                   11
#define AMIGA_CIA_TA                    0
#define AMIGA_CIA_SP                    3

unsigned int amiga_setroutine(unsigned int irq, void (*routine)(unsigned int irq));
unsigned int amiga_unsetroutine(unsigned int irq, void (*routine)(unsigned int irq));
unsigned int amiga_setciaroutine(unsigned int irq, void (*routine)(unsigned int irq));
unsigned int amiga_unsetciaroutine(unsigned int irq, void (*routine)(unsigned int irq));
