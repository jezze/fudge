#define PIC_IRQ_SOFTWARE                (1 << 0)
#define PIC_IRQ_UART0                   (1 << 1)
#define PIC_IRQ_UART1                   (1 << 2)
#define PIC_IRQ_KEYBOARD                (1 << 3)
#define PIC_IRQ_MOUSE                   (1 << 4)
#define PIC_IRQ_TIMER0                  (1 << 5)
#define PIC_IRQ_TIMER1                  (1 << 6)
#define PIC_IRQ_TIMER2                  (1 << 7)
#define PIC_IRQ_RTC                     (1 << 8)
#define PIC_IRQ_LM0                     (1 << 9)
#define PIC_IRQ_LM1                     (1 << 10)
#define PIC_IRQ_DISPLAY                 (1 << 22)
#define PIC_IRQ_MMCI0                   (1 << 23)
#define PIC_IRQ_MMCI1                   (1 << 24)
#define PIC_IRQ_AUDIO                   (1 << 25)
#define PIC_IRQ_SECONDARY               (1 << 26)
#define PIC_IRQ_ETHERNET                (1 << 27)
#define PIC_IRQ_TOUCH                   (1 << 28)

void pic_enableirq(unsigned int irq);
void pic_disableirq(unsigned int irq);
void pic_enablefiq(unsigned int fiq);
void pic_disablefiq(unsigned int fiq);
void pic_setup(void);
