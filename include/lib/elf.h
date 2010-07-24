#ifndef ELF_H
#define ELF_H

#define ELF_IDENTITY_MAGIC0 0x7F
#define ELF_IDENTITY_MAGIC1 'E'
#define ELF_IDENTITY_MAGIC2 'L'
#define ELF_IDENTITY_MAGIC3 'F'
#define ELF_IDENTITY_CLASS_NONE 0
#define ELF_IDENTITY_CLASS_32   1
#define ELF_IDENTITY_CLASS_64   2
#define ELF_IDENTITY_DATA_NONE 0
#define ELF_IDENTITY_DATA_2LSB 1
#define ELF_IDENTITY_DATA_2MSB 2

#define ELF_TYPE_NONE        0x0000
#define ELF_TYPE_RELOCATABLE 0x0001
#define ELF_TYPE_EXECUTABLE  0x0002
#define ELF_TYPE_DYNAMIC     0x0003
#define ELF_TYPE_CORE        0x0004
#define ELF_TYPE_LOPROC      0xFF00
#define ELF_TYPE_HIPROC      0xFFFF

#define ELF_MACHINE_NONE  0x0000
#define ELF_MACHINE_M32   0x0001
#define ELF_MACHINE_SPARC 0x0002
#define ELF_MACHINE_386   0x0003
#define ELF_MACHINE_68K   0x0004
#define ELF_MACHINE_88K   0x0005
#define ELF_MACHINE_860   0x0007
#define ELF_MACHINE_MIPS  0x0008

struct elf_header
{

    unsigned char identify[16];
    unsigned short type;
    unsigned short machine;
    unsigned int version;
    unsigned int entry;
    unsigned int programHeaderOffset;
    unsigned int sectionHeaderOffset;
    unsigned int flags;
    unsigned short headerSize;
    unsigned short programHeaderEntrySize;
    unsigned short programHeaderCount;
    unsigned short sectionHeaderEntrySize;
    unsigned short sectionHeaderCount;
    unsigned short sectionHeaderStringIndex;

};

struct elf_program_header
{

    unsigned int type;
    unsigned int offset;
    unsigned int virtualAddress;
    unsigned int physicalAddress;
    unsigned int fileSize;
    unsigned int memorySize;
    unsigned int flags;
    unsigned int align;

};

struct elf_section_header
{

    unsigned int name;
    unsigned int type;
    unsigned int flags;
    unsigned int address;
    unsigned int offset;
    unsigned int size;
    unsigned int link;
    unsigned int info;
    unsigned int align;
    unsigned int entrySize;

};



#endif

