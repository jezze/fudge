#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/gdt.h>
#include <kernel/x86/idt.h>
#include <kernel/x86/tss.h>
#include <kernel/x86/arch.h>
#include <modules/arch/x86/uart/uart.h>

#define BUFMAX                          400
#define NUMREGS                         16
#define NUMREGBYTES                     (NUMREGS * 4)
#define STACKSIZE                       10000

int remote_debug;
static const char hexchars[] = "0123456789abcdef";
enum regnames {EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI, PC, PS, CS, SS, DS, ES, FS, GS};
int registers[NUMREGS];
int remcomStack[STACKSIZE / sizeof (int)];
int *stackPtr = &remcomStack[STACKSIZE / sizeof (int) - 1];
static struct arch_gdt *gdt = (struct arch_gdt *)ARCH_GDTADDRESS;
static struct arch_idt *idt = (struct arch_idt *)ARCH_IDTADDRESS;

extern void return_to_prog();
__asm__(".text");
__asm__(".globl return_to_prog");
__asm__("return_to_prog:");
__asm__("movw registers+44, %ss");
__asm__("movl registers+16, %esp");
__asm__("movl registers+4, %ecx");
__asm__("movl registers+8, %edx");
__asm__("movl registers+12, %ebx");
__asm__("movl registers+20, %ebp");
__asm__("movl registers+24, %esi");
__asm__("movl registers+28, %edi");
__asm__("movw registers+48, %ds");
__asm__("movw registers+52, %es");
__asm__("movw registers+56, %fs");
__asm__("movw registers+60, %gs");
__asm__("movl registers+36, %eax");
__asm__("pushl %eax");
__asm__("movl registers+40, %eax");
__asm__("pushl %eax");
__asm__("movl registers+32, %eax");
__asm__("pushl %eax");
__asm__("movl registers, %eax");
__asm__("iret");

int gdb_i386errcode;
int gdb_i386vector = -1;

#define SAVE_REGISTERS1()                       \
    __asm__("movl %eax, registers");           \
    __asm__("movl %ecx, registers+4");         \
    __asm__("movl %edx, registers+8");         \
    __asm__("movl %ebx, registers+12");        \
    __asm__("movl %ebp, registers+20");        \
    __asm__("movl %esi, registers+24");        \
    __asm__("movl %edi, registers+28");        \
    __asm__("movw $0, %ax");                   \
    __asm__("movw %ds, registers+48");         \
    __asm__("movw %ax, registers+50");         \
    __asm__("movw %es, registers+52");         \
    __asm__("movw %ax, registers+54");         \
    __asm__("movw %fs, registers+56");         \
    __asm__("movw %ax, registers+58");         \
    __asm__("movw %gs, registers+60");         \
    __asm__("movw %ax, registers+62")

#define SAVE_ERRCODE()                          \
    __asm__("popl %ebx");                      \
    __asm__("movl %ebx, gdb_i386errcode")

#define SAVE_REGISTERS2()                       \
    __asm__("popl %ebx");                      \
    __asm__("movl %ebx, registers+32");        \
    __asm__("popl %ebx");                      \
    __asm__("movl %ebx, registers+40");        \
    __asm__("movw %ax, registers+42");         \
    __asm__("popl %ebx");                      \
    __asm__("movl %ebx, registers+36");        \
    __asm__("movw %ss, registers+44");         \
    __asm__("movw %ax, registers+46");         \
    __asm__("movl %esp, registers+16")

#define CHECK_FAULT()                           \
    __asm__("cmpl $0, mem_fault_routine");     \
    __asm__("jne mem_fault")

#define CALL_HOOK()                             \
    __asm__("call _remcomHandler")

__asm__(".text");
__asm__("mem_fault:");
__asm__("popl %eax");
__asm__("movl %eax, gdb_i386errcode");
__asm__("popl %eax");
__asm__("movl mem_fault_routine, %eax");
__asm__("popl %ecx");
__asm__("popl %edx");
__asm__("leave");
__asm__("pushl %edx");
__asm__("pushl %ecx");
__asm__("pushl %eax");
__asm__("movl $0, %eax");
__asm__("movl %eax, mem_fault_routine");
__asm__("iret");

extern void _catchException3();
__asm__(".text");
__asm__(".globl _catchException3");
__asm__("_catchException3:");
SAVE_REGISTERS1();
SAVE_REGISTERS2();
__asm__("pushl $3");
CALL_HOOK();

extern void _catchException1();
__asm__(".text");
__asm__(".globl _catchException1");
__asm__("_catchException1:");
SAVE_REGISTERS1();
SAVE_REGISTERS2();
__asm__("pushl $1");
CALL_HOOK();

extern void _catchException0();
__asm__(".text");
__asm__(".globl _catchException0");
__asm__("_catchException0:");
SAVE_REGISTERS1();
SAVE_REGISTERS2();
__asm__("pushl $0");
CALL_HOOK();

extern void _catchException4();
__asm__(".text");
__asm__(".globl _catchException4");
__asm__("_catchException4:");
SAVE_REGISTERS1();
SAVE_REGISTERS2();
__asm__("pushl $4");
CALL_HOOK();

extern void _catchException5();
__asm__(".text");
__asm__(".globl _catchException5");
__asm__("_catchException5:");
SAVE_REGISTERS1();
SAVE_REGISTERS2();
__asm__("pushl $5");
CALL_HOOK();

extern void _catchException6();
__asm__(".text");
__asm__(".globl _catchException6");
__asm__("_catchException6:");
SAVE_REGISTERS1();
SAVE_REGISTERS2();
__asm__("pushl $6");
CALL_HOOK();

extern void _catchException7();
__asm__(".text");
__asm__(".globl _catchException7");
__asm__("_catchException7:");
SAVE_REGISTERS1();
SAVE_REGISTERS2();
__asm__("pushl $7");
CALL_HOOK();

extern void _catchException8();
__asm__(".text");
__asm__(".globl _catchException8");
__asm__("_catchException8:");
SAVE_REGISTERS1();
SAVE_ERRCODE();
SAVE_REGISTERS2();
__asm__("pushl $8");
CALL_HOOK();

extern void _catchException9();
__asm__(".text");
__asm__(".globl _catchException9");
__asm__("_catchException9:");
SAVE_REGISTERS1();
SAVE_REGISTERS2();
__asm__("pushl $9");
CALL_HOOK();

extern void _catchException10();
__asm__(".text");
__asm__(".globl _catchException10");
__asm__("_catchException10:");
SAVE_REGISTERS1();
SAVE_ERRCODE();
SAVE_REGISTERS2();
__asm__("pushl $10");
CALL_HOOK();

extern void _catchException12();
__asm__(".text");
__asm__(".globl _catchException12");
__asm__("_catchException12:");
SAVE_REGISTERS1();
SAVE_ERRCODE();
SAVE_REGISTERS2();
__asm__("pushl $12");
CALL_HOOK();

extern void _catchException16();
__asm__(".text");
__asm__(".globl _catchException16");
__asm__("_catchException16:");
SAVE_REGISTERS1();
SAVE_REGISTERS2();
__asm__("pushl $16");
CALL_HOOK();

extern void _catchException13();
__asm__(".text");
__asm__(".globl _catchException13");
__asm__("_catchException13:");
CHECK_FAULT();
SAVE_REGISTERS1();
SAVE_ERRCODE();
SAVE_REGISTERS2();
__asm__("pushl $13");
CALL_HOOK();

extern void _catchException11();
__asm__(".text");
__asm__(".globl _catchException11");
__asm__("_catchException11:");
CHECK_FAULT();
SAVE_REGISTERS1();
SAVE_ERRCODE();
SAVE_REGISTERS2();
__asm__("pushl $11");
CALL_HOOK();

extern void _catchException14();
__asm__(".text");
__asm__(".globl _catchException14");
__asm__("_catchException14:");
CHECK_FAULT();
SAVE_REGISTERS1();
SAVE_ERRCODE();
SAVE_REGISTERS2();
__asm__("pushl $14");
CALL_HOOK();

__asm__("_remcomHandler:");
__asm__("popl %eax");
__asm__("popl %eax");
__asm__("movl stackPtr, %esp");
__asm__("pushl %eax");
__asm__("call handle_exception");

int hex(char ch)
{

    if ((ch >= 'a') && (ch <= 'f'))
        return (ch - 'a' + 10);

    if ((ch >= '0') && (ch <= '9'))
        return (ch - '0');

    if ((ch >= 'A') && (ch <= 'F'))
        return (ch - 'A' + 10);

    return -1;

}

static char remcomInBuffer[BUFMAX];
static char remcomOutBuffer[BUFMAX];

char *getpacket(void)
{

    char *buffer = &remcomInBuffer[0];
    unsigned char checksum;
    unsigned char xmitcsum;
    int count;
    char ch;

    while (1)
    {

        while ((ch = uart_get()) != '$');

    retry:
        checksum = 0;
        xmitcsum = -1;
        count = 0;

        while (count < BUFMAX - 1)
        {

            ch = uart_get();

            if (ch == '$')
                goto retry;

            if (ch == '#')
                break;

            checksum = checksum + ch;
            buffer[count] = ch;
            count = count + 1;

        }

        buffer[count] = 0;

        if (ch == '#')
        {

            ch = uart_get();
            xmitcsum = hex(ch) << 4;
            ch = uart_get();
            xmitcsum += hex(ch);

            if (checksum != xmitcsum)
            {

                if (remote_debug)
                {

                    /* TODO
                    fprintf(stderr, "bad checksum.  My count = 0x%x, sent=0x%x. buf=%s\n", checksum, xmitcsum, buffer);
                    */

                }

                uart_put('-');
            }

            else
            {

                uart_put('+');

                if (buffer[2] == ':')
                {

                    uart_put(buffer[0]);
                    uart_put(buffer[1]);

                    return &buffer[3];

                }

                return &buffer[0];

            }

        }

    }

}

void putpacket(char *buffer)
{

    do
    {

        unsigned char checksum;
        int count;
        char ch;

        uart_put('$');

        checksum = 0;
        count = 0;

        while ((ch = buffer[count]))
        {

            uart_put(ch);

            checksum += ch;
            count += 1;

        }

        uart_put('#');
        uart_put(hexchars[checksum >> 4]);
        uart_put(hexchars[checksum % 16]);

    }

    while (uart_get() != '+');

}

void debug_error(char *format)
{

    /* TODO
    if (remote_debug)
        fprintf(stderr, format, parm);
    */

}

static void (*volatile mem_fault_routine)() = 0;
static volatile int mem_err = 0;

void set_mem_err(void)
{

    mem_err = 1;

}

int get_char(char *addr)
{

    return *addr;

}

void set_char(char *addr, int val)
{

    *addr = val;

}

char *mem2hex(char *mem, char *buf, int count, int may_fault)
{

    int i;

    if (may_fault)
        mem_fault_routine = set_mem_err;

    for (i = 0; i < count; i++)
    {

        unsigned char ch;

        ch = get_char(mem++);

        if (may_fault && mem_err)
            return buf;

        *buf++ = hexchars[ch >> 4];
        *buf++ = hexchars[ch % 16];

    }

    *buf = 0;

    if (may_fault)
        mem_fault_routine = 0;

    return buf;

}

char *hex2mem(char *buf, char *mem, int count, int may_fault)
{

    int i;

    if (may_fault)
        mem_fault_routine = set_mem_err;

    for (i = 0; i < count; i++)
    {

        unsigned char ch;

        ch = hex(*buf++) << 4;
        ch = ch + hex(*buf++);

        set_char(mem++, ch);

        if (may_fault && mem_err)
            return mem;

    }

    if (may_fault)
        mem_fault_routine = 0;

    return (mem);

}

int computeSignal(int exceptionVector)
{

    int sigval;

    switch (exceptionVector)
    {

    case 0:
        sigval = 8;

        break;

    case 1:
        sigval = 5;

        break;

    case 3:
        sigval = 5;

        break;

    case 4:
        sigval = 16;

        break;

    case 5:
        sigval = 16;

        break;

    case 6:
        sigval = 4;

        break;

    case 7:
        sigval = 8;

        break;

    case 8:
        sigval = 7;

        break;

    case 9:
        sigval = 11;

        break;

    case 10:
        sigval = 11;

        break;

    case 11:
        sigval = 11;

        break;

    case 12:
        sigval = 11;

        break;

    case 13:
        sigval = 11;

        break;

    case 14:
        sigval = 11;

        break;

    case 16:
        sigval = 7;

        break;

    default:
        sigval = 7;

    }

    return sigval;

}

int hexToInt(char **ptr, int *intValue)
{

    int numChars = 0;

    *intValue = 0;

    while (**ptr)
    {

        int hexValue = hex(**ptr);

        if (hexValue >= 0)
        {

            *intValue = (*intValue << 4) | hexValue;
            numChars++;

        }

        else
            break;

        (*ptr)++;

    }

    return numChars;

}

void handle_exception(int exceptionVector)
{

    int sigval, stepping;
    int addr, length;
    char *ptr;

    gdb_i386vector = exceptionVector;

    if (remote_debug)
    {

        /* TODO
        printf ("vector=%d, sr=0x%x, pc=0x%x\n", exceptionVector, registers[PS], registers[PC]);
        */

    }

    sigval = computeSignal(exceptionVector);
    ptr = remcomOutBuffer;
    *ptr++ = 'T';
    *ptr++ = hexchars[sigval >> 4];
    *ptr++ = hexchars[sigval & 0xf];
    *ptr++ = hexchars[ESP];
    *ptr++ = ':';
    ptr = mem2hex((char *)&registers[ESP], ptr, 4, 0);
    *ptr++ = ';';
    *ptr++ = hexchars[EBP];
    *ptr++ = ':';
    ptr = mem2hex((char *)&registers[EBP], ptr, 4, 0);
    *ptr++ = ';';
    *ptr++ = hexchars[PC];
    *ptr++ = ':';
    ptr = mem2hex((char *)&registers[PC], ptr, 4, 0);
    *ptr++ = ';';
    *ptr = '\0';

    putpacket(remcomOutBuffer);

    stepping = 0;

    for (;;)
    {

        remcomOutBuffer[0] = 0;
        ptr = getpacket();

        switch (*ptr++)
        {

        case '?':
            remcomOutBuffer[0] = 'S';
            remcomOutBuffer[1] = hexchars[sigval >> 4];
            remcomOutBuffer[2] = hexchars[sigval % 16];
            remcomOutBuffer[3] = 0;

            break;

        case 'd':
            remote_debug = !(remote_debug);

            break;

        case 'g':
            mem2hex((char *)registers, remcomOutBuffer, NUMREGBYTES, 0);

            break;

        case 'G':
            hex2mem(ptr, (char *)registers, NUMREGBYTES, 0);
            ascii_copy(remcomOutBuffer, "OK");

            break;

        case 'P':
        {

            int regno;

            if (hexToInt(&ptr, &regno) && *ptr++ == '=')
                if (regno >= 0 && regno < NUMREGS)
                {

                    hex2mem(ptr, (char *)&registers[regno], 4, 0);
                    ascii_copy(remcomOutBuffer, "OK");

                    break;

                }

            ascii_copy(remcomOutBuffer, "E01");

            break;

        }

        case 'm':
            if (hexToInt(&ptr, &addr))
                if (*(ptr++) == ',')
                    if (hexToInt(&ptr, &length))
                    {

                        ptr = 0;
                        mem_err = 0;

                        mem2hex((char *)addr, remcomOutBuffer, length, 1);

                        if (mem_err)
                        {

                            ascii_copy(remcomOutBuffer, "E03");
                            debug_error("memory fault");

                        }

                    }

                    if (ptr)
                    {

                        ascii_copy(remcomOutBuffer, "E01");

                    }

            break;

        case 'M':
            if (hexToInt(&ptr, &addr))
                if (*(ptr++) == ',')
                    if (hexToInt(&ptr, &length))
                        if (*(ptr++) == ':')
                        {

                            mem_err = 0;

                            hex2mem(ptr, (char *)addr, length, 1);

                            if (mem_err)
                            {

                                ascii_copy(remcomOutBuffer, "E03");
                                debug_error("memory fault");

                            }

                            else
                            {

                                ascii_copy(remcomOutBuffer, "OK");

                            }

                            ptr = 0;

                        }

            if (ptr)
            {

                ascii_copy(remcomOutBuffer, "E02");

            }

            break;

        case 's':
            stepping = 1;

        case 'c':
            if (hexToInt(&ptr, &addr))
                registers[PC] = addr;

            registers[PS] &= 0xfffffeff;

            if (stepping)
                registers[PS] |= 0x100;

            return_to_prog();

            break;

        case 'k':
            break;

        }

        putpacket(remcomOutBuffer);

    }

}

void module_init(void)
{

    stackPtr = &remcomStack[STACKSIZE / sizeof (int) - 1];

    idt_setdescriptor(&idt->pointer, 0, _catchException0, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 1, _catchException1, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 3, _catchException3, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 4, _catchException4, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 5, _catchException5, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 6, _catchException6, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 7, _catchException7, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 8, _catchException8, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 9, _catchException9, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 10, _catchException10, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 11, _catchException11, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 12, _catchException12, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 13, _catchException13, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 14, _catchException14, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 16, _catchException16, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    __asm__("int $3");

}

