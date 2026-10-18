#define ICP_BUS                         0x0006
#define ICP_TIMER1                      0x0001
#define ICP_RTC                         0x0002
#define ICP_UART0                       0x0003

unsigned int icp_getbase(unsigned int id);
unsigned int icp_getirq(unsigned int id);
unsigned int icp_setroutine(unsigned int irq, void (*routine)(unsigned int irq));
unsigned int icp_unsetroutine(unsigned int irq, void (*routine)(unsigned int irq));
