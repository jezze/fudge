#include "pic.h"
#include "reg.h"
#include "timer.h"

#include "uart.h"

#define TIMER_REG_TIMER0_LOAD           0x13000000
#define TIMER_REG_TIMER0_CONTROL        0x13000008
#define TIMER_REG_TIMER0_INTCLR         0x1300000C
#define TIMER_REG_TIMER1_LOAD           0x13000100
#define TIMER_REG_TIMER1_CONTROL        0x13000108
#define TIMER_REG_TIMER1_INTCLR         0x1300010C
#define TIMER_REG_TIMER2_LOAD           0x13000200
#define TIMER_REG_TIMER2_CONTROL        0x13000208
#define TIMER_REG_TIMER2_INTCLR         0x1300020C

#define TIMER_CONTROL_ONESHOT           (1 << 0)
#define TIMER_CONTROL_SIZE32            (1 << 1)
#define TIMER_CONTROL_INTENABLE         (1 << 5)
#define TIMER_CONTROL_MODEPERIODIC      (1 << 6)
#define TIMER_CONTROL_ENABLE            (1 << 7)

void timer_irq(void)
{

    reg_write32(TIMER_REG_TIMER0_INTCLR, 1);
    uart_puts("TIMER0\n");

}

void timer_setup(void)
{

    reg_write32(TIMER_REG_TIMER0_LOAD, 0x00FFFFFF);
    reg_write32(TIMER_REG_TIMER0_CONTROL, TIMER_CONTROL_SIZE32 | TIMER_CONTROL_INTENABLE | TIMER_CONTROL_MODEPERIODIC | TIMER_CONTROL_ENABLE);
    reg_write32(TIMER_REG_TIMER0_INTCLR, ~0);
    pic_enableirq(PIC_IRQ_TIMER0);

}

