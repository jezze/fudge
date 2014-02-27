#define ARCH_BIOS_BASE                  0x00000000
#define ARCH_BIOS_LIMIT                 0x00100000
#define ARCH_BIOS_SIZE                  (ARCH_BIOS_LIMIT - ARCH_KERNEL_BASE)
#define ARCH_KCODE_BASE                 0x00100000
#define ARCH_KCODE_LIMIT                0x00280000
#define ARCH_KCODE_SIZE                 (ARCH_KCODE_LIMIT - ARCH_KCODE_BASE)
#define ARCH_KSTACK_BASE                0x00280000
#define ARCH_KSTACK_LIMIT               0x00300000
#define ARCH_KSTACK_SIZE                (ARCH_KSTACK_LIMIT - ARCH_KSTACK_BASE)
#define ARCH_DIRECTORY_BASE             0x00300000
#define ARCH_DIRECTORY_LIMIT            0x00340000
#define ARCH_DIRECTORY_SIZE             (ARCH_DIRECTORY_LIMIT - ARCH_DIRECTORY_BASE)
#define ARCH_TABLE_KCODE_BASE           0x00340000
#define ARCH_TABLE_KCODE_LIMIT          0x00380000
#define ARCH_TABLE_KCODE_SIZE           (ARCH_TABLE_KCODE_LIMIT - ARCH_TABLE_KCODE_BASE)
#define ARCH_TABLE_UCODE_BASE           0x00380000
#define ARCH_TABLE_UCODE_LIMIT          0x003C0000
#define ARCH_TABLE_UCODE_SIZE           (ARCH_TABLE_UCODE_LIMIT - ARCH_TABLE_UCODE_BASE)
#define ARCH_TABLE_USTACK_BASE          0x003C0000
#define ARCH_TABLE_USTACK_LIMIT         0x00400000
#define ARCH_TABLE_USTACK_SIZE          (ARCH_TABLE_USTACK_LIMIT - ARCH_TABLE_USTACK_BASE)
#define ARCH_UCODE_BASE                 0x00800000
#define ARCH_UCODE_LIMIT                0x00C00000
#define ARCH_UCODE_SIZE                 (ARCH_UCODE_LIMIT - ARCH_UCODE_BASE)
#define ARCH_USTACK_BASE                0x00C00000
#define ARCH_USTACK_LIMIT               0x01000000
#define ARCH_USTACK_SIZE                (ARCH_USTACK_LIMIT - ARCH_USTACK_BASE)

void arch_halt();
void arch_usermode(unsigned int code, unsigned int data, unsigned int ip, unsigned int sp);
void arch_isr_generalfault();
void arch_isr_pagefault();
void arch_isr_syscall();
unsigned short arch_segment();
unsigned short arch_schedule(struct cpu_general *general, struct cpu_interrupt *interrupt);
void arch_setup(unsigned int count, struct kernel_module *modules);
