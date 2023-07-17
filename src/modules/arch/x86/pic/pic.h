void pic_isr0(void);
void pic_isr1(void);
unsigned short pic_getirr(void);
unsigned short pic_getisr(void);
unsigned int pic_setroutine(unsigned int irq, void (*routine)(unsigned int irq));
unsigned int pic_unsetroutine(unsigned int irq);
void pic_disable(void);
