#include <memory.h>
#include <string.h>
#include <kernel.h>
#include <arch/arm/arch.h>
#include <arch/arm/cpu.h>

#define PIC_REG_IRQ_STATUS      0x14000000
#define PIC_REG_IRQ_RAWSTAT     0x14000004
#define PIC_REG_IRQ_ENABLESET   0x14000008
#define PIC_REG_IRQ_ENABLECLR   0x1400000C
#define PIC_REG_INT_SOFTSET     0x14000010
#define PIC_REG_INT_SOFTCLR     0x14000014
#define PIC_REG_FIQ_STATUS      0x14000020
#define PIC_REG_FIQ_RAWSTAT     0x14000024
#define PIC_REG_FIQ_ENABLESET   0x14000028
#define PIC_REG_FIQ_ENABLECLR   0x1400002C

#define PIC_IRQ_SOFTWARE        (1 << 0)
#define PIC_IRQ_UART0           (1 << 1)
#define PIC_IRQ_UART1           (1 << 2)
#define PIC_IRQ_KEYBOARD        (1 << 3)
#define PIC_IRQ_MOUSE           (1 << 4)
#define PIC_IRQ_TIMER0          (1 << 5)
#define PIC_IRQ_TIMER1          (1 << 6)
#define PIC_IRQ_TIMER2          (1 << 7)
#define PIC_IRQ_RTC             (1 << 8)
#define PIC_IRQ_LM0             (1 << 9)
#define PIC_IRQ_LM1             (1 << 10)
#define PIC_IRQ_DISPLAY         (1 << 22)
#define PIC_IRQ_MMCI0           (1 << 23)
#define PIC_IRQ_MMCI1           (1 << 24)
#define PIC_IRQ_AUDIO           (1 << 25)
#define PIC_IRQ_SECONDARY       (1 << 26)
#define PIC_IRQ_ETHERNET        (1 << 27)
#define PIC_IRQ_TOUCH           (1 << 28)

#define UART_REG_DATA           0x16000000
#define UART_REG_FLAG           0x16000018

#define UART_FLAG_FULL          (1 << 5)

#define KMI_REG_KBD_CONTROL     0x18000000
#define KMI_REG_KBD_STATUS      0x18000004
#define KMI_REG_KBD_DATA        0x18000008
#define KMI_REG_KBD_CLOCK       0x1800000C
#define KMI_REG_KBD_INTERRUPT   0x18000010
#define KMI_REG_MOUSE_CONTROL   0x19000000
#define KMI_REG_MOUSE_STATUS    0x19000004
#define KMI_REG_MOUSE_DATA      0x19000008
#define KMI_REG_MOUSE_CLOCK     0x1900000C
#define KMI_REG_MOUSE_INTERRUPT 0x19000010

#define KMI_CONTROL_ENABLE      (1 << 2)
#define KMI_CONTROL_INTTX       (1 << 3)
#define KMI_CONTROL_INTRX       (1 << 4)

#define KMI_STATUS_FULL         (1 << 4)

static struct arch_interface interface;

static char mapUS[256] =
{
       0,   27,  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '-',  '+', '\b', '\t',
     'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  'o',  'p',  '[',  ']', '\n',    0,  'a',  's',
     'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';', '\'',  '`',    0, '\\',  'z',  'x',  'c',  'v',
     'b',  'n',  'm',  ',',  '.',  '/',    0,    0,    0,  ' ',    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,   27,  '!',  '@',  '#',  '$',  '%',  '^',  '&',  '*',  '(',  ')',  '_',  '=', '\b', '\t',
     'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  'O',  'P',  '{',  '}', '\n',    0,  'A',  'S',
     'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':', '\"',  '~',    0,  '|',  'Z',  'X',  'C',  'V',
     'B',  'N',  'M',  '<',  '>',  '?',    0,    0,    0,  ' ',    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0
};

unsigned int read_reg8(unsigned int address)
{

    volatile unsigned char *mem = (volatile unsigned char *)address;

    return *mem;

}

unsigned int read_reg32(unsigned int address)
{

    volatile unsigned int *mem = (volatile unsigned int *)address;

    return *mem;

}

void write_reg8(unsigned int address, unsigned char value)
{

    volatile unsigned char *mem = (volatile unsigned char *)address;

    *mem = value;

}

void write_reg32(unsigned int address, unsigned int value)
{

    volatile unsigned int *mem = (volatile unsigned int *)address;

    *mem = value;

}

static void putc(char c)
{

    while (read_reg32(UART_REG_FLAG) & UART_FLAG_FULL);

    write_reg8(UART_REG_DATA, c);

}

static void puts(const char *s)
{

    while (*s)
        putc(*s++);

}

static void enable_interrupts()
{

    char num[32];

    write_reg32(PIC_REG_IRQ_ENABLECLR, PIC_IRQ_KEYBOARD);

    write_reg32(KMI_REG_KBD_CONTROL, KMI_CONTROL_INTRX | KMI_CONTROL_INTTX);

    write_reg32(PIC_REG_IRQ_ENABLESET, read_reg32(PIC_REG_IRQ_ENABLESET) | PIC_IRQ_KEYBOARD);

    string_write_num(num, read_reg32(PIC_REG_IRQ_ENABLESET), 16);

    puts("CLR: ");
    puts(num);
    puts("\n");

    for (;;)
    {

        unsigned char x;

        while (!(read_reg32(KMI_REG_KBD_STATUS) & KMI_STATUS_FULL));

        x = read_reg8(KMI_REG_KBD_DATA);

        if (x < 0x80)
        {

            putc(mapUS[x]);

        }

    }

}

static void setup(struct kernel_interface *arch)
{

    puts("Fudge Console Text\n\n");

    enable_interrupts();

    for (;;);

}

void isr_undefined()
{

    puts("INTERRUPT\n");

    for (;;);

}

void arch_init_interface(struct arch_interface *interface)
{

    memory_clear(interface, sizeof (struct arch_interface));

    kernel_init_interface(&interface->base, setup, 0, 0, 0);

}

void arch_setup()
{

    arch_init_interface(&interface);

    kernel_register_interface(&interface.base);

}

