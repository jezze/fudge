#define ELF_HEADER_SIZE                 52

#define ELF_IDENTITY_MAGIC0             0x7F
#define ELF_IDENTITY_MAGIC1             'E'
#define ELF_IDENTITY_MAGIC2             'L'
#define ELF_IDENTITY_MAGIC3             'F'
#define ELF_IDENTITY_CLASS_NONE         0
#define ELF_IDENTITY_CLASS_32           1
#define ELF_IDENTITY_CLASS_64           2
#define ELF_IDENTITY_DATA_NONE          0
#define ELF_IDENTITY_DATA_2LSB          1
#define ELF_IDENTITY_DATA_2MSB          2

#define ELF_TYPE_NONE                   0x0000
#define ELF_TYPE_RELOCATABLE            0x0001
#define ELF_TYPE_EXECUTABLE             0x0002
#define ELF_TYPE_DYNAMIC                0x0003
#define ELF_TYPE_CORE                   0x0004
#define ELF_TYPE_LOPROC                 0xFF00
#define ELF_TYPE_HIPROC                 0xFFFF

#define ELF_MACHINE_NONE                0x0000
#define ELF_MACHINE_M32                 0x0001
#define ELF_MACHINE_SPARC               0x0002
#define ELF_MACHINE_386                 0x0003
#define ELF_MACHINE_68K                 0x0004
#define ELF_MACHINE_88K                 0x0005
#define ELF_MACHINE_860                 0x0007
#define ELF_MACHINE_MIPS                0x0008

#define ELF_SECTION_INDEX_UNDEFINED     0x0000
#define ELF_SECTION_INDEX_LORESERVE     0xFF00
#define ELF_SECTION_INDEX_LOPROC        0xFF00
#define ELF_SECTION_INDEX_HIPROC        0xFF1F
#define ELF_SECTION_INDEX_ABS           0xFFF1
#define ELF_SECTION_INDEX_COMMON        0xFFF2
#define ELF_SECTION_INDEX_HIRESERVE     0xFFFF

#define ELF_SECTION_TYPE_NULL           0x00000000
#define ELF_SECTION_TYPE_PROGBITS       0x00000001
#define ELF_SECTION_TYPE_SYMTAB         0x00000002
#define ELF_SECTION_TYPE_STRTAB         0x00000003
#define ELF_SECTION_TYPE_RELA           0x00000004
#define ELF_SECTION_TYPE_HASH           0x00000005
#define ELF_SECTION_TYPE_DYNAMIC        0x00000006
#define ELF_SECTION_TYPE_NOTE           0x00000007
#define ELF_SECTION_TYPE_NOBITS         0x00000008
#define ELF_SECTION_TYPE_REL            0x00000009
#define ELF_SECTION_TYPE_SHLIB          0x0000000A
#define ELF_SECTION_TYPE_DYNSYM         0x0000000B
#define ELF_SECTION_TYPE_LOPROC         0x70000000
#define ELF_SECTION_TYPE_HIPROC         0x7FFFFFFF
#define ELF_SECTION_TYPE_LOUSER         0x80000000
#define ELF_SECTION_TYPE_HIUSER         0xFFFFFFFF

#define ELF_SECTION_FLAG_WRITE          0x00000001
#define ELF_SECTION_FLAG_ALLOC          0x00000002
#define ELF_SECTION_FLAG_EXEC           0x00000004
#define ELF_SECTION_FLAG_MASK           0xF0000000

#define ELF_RELOC_TYPE_NONE             0x00000000
#define ELF_RELOC_TYPE_32               0x00000001
#define ELF_RELOC_TYPE_PC32             0x00000002
#define ELF_RELOC_TYPE_GOT32            0x00000003
#define ELF_RELOC_TYPE_PLT32            0x00000004
#define ELF_RELOC_TYPE_COPY             0x00000005
#define ELF_RELOC_TYPE_GLOB_DAT         0x00000006
#define ELF_RELOC_TYPE_JMP_SLOT         0x00000007
#define ELF_RELOC_TYPE_RELATIVE         0x00000008
#define ELF_RELOC_TYPE_GOTOFF           0x00000009
#define ELF_RELOC_TYPE_GOTPC            0x0000000A

struct elf_header
{

    unsigned char identify[16];
    unsigned short type;
    unsigned short machine;
    unsigned int version;
    unsigned int entry;
    unsigned int phoffset;
    unsigned int shoffset;
    unsigned int flags;
    unsigned short size;
    unsigned short phsize;
    unsigned short phcount;
    unsigned short shsize;
    unsigned short shcount;
    unsigned short shstringindex;

};

struct elf_programheader
{

    unsigned int type;
    unsigned int offset;
    unsigned int vaddress;
    unsigned int paddress;
    unsigned int fsize;
    unsigned int msize;
    unsigned int flags;
    unsigned int align;

};

struct elf_sectionheader
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
    unsigned int esize;

};

struct elf_symbol
{

    unsigned int name;
    unsigned int value;
    unsigned int size;
    unsigned char info;
    unsigned char other;
    unsigned short shindex;

};

struct elf_relocation
{

    unsigned int offset;
    unsigned int info;

};

unsigned int elf_validate(struct elf_header *header);
