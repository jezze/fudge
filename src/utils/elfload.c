#include <fudge.h>
#include <abi.h>
#include <binary.h>

#define RELOCATE_UNSUPPORTED            0
#define RELOCATE_OK                     1
#define RELOCATE_RANGE                  2

static char kerneldata[8192];
static unsigned int kernelcount;
static char mapdata[4096];
static unsigned int mapcount;
static char lines[4096];
static unsigned int linescount;
static unsigned int width;

static void writeaddress(char *out, unsigned int address)
{

    cstring_write_fmt(out, width, 0, (width == 16) ? "%H8<u" : "%H8u", &address);

}

static void relocate(unsigned int address)
{

    unsigned int offset = 0;
    unsigned int i;

    for (i = 0; (offset = buffer_eachbyte(mapdata, mapcount, '\n', offset)); i = offset)
    {

        if (mapdata[i + width + 1] == 'T')
            writeaddress(&mapdata[i], cstring_read_value(&mapdata[i], width, 16) + address);

    }

}

static unsigned int findsymbol(char *data, unsigned int count, unsigned int length, char *symbol)
{

    unsigned int offset = 0;
    unsigned int i;

    for (i = 0; (offset = buffer_eachbyte(data, count, '\n', offset)); i = offset)
    {

        if ((data[i + width + 1] == 'T') || (data[i + width + 1] == 'A'))
        {

            if (buffer_match(&data[i + width + 3], symbol, length))
                return cstring_read_value(&data[i], width, 16);

        }

    }

    return 0;

}

static void updateundefined(void)
{

    unsigned int offset = 0;
    unsigned int i;

    for (i = 0; (offset = buffer_eachbyte(mapdata, mapcount, '\n', offset)); i = offset)
    {

        if (mapdata[i + width + 1] == 'U')
        {

            char *symbol = &mapdata[i + width + 3];
            unsigned int length = (offset - i) - (width + 3);
            unsigned int address = findsymbol(kerneldata, kernelcount, length, symbol);

            if (!address)
            {

                unsigned int underscore = buffer_findbyte(symbol, length, '_');
                char module[32];
                char data[4096];
                unsigned int count;

                cstring_write_fmt(module, 32, 0, "initrd:kernel/%w.ko.map\\0", symbol, &underscore);

                count = fs_read_path(1, module, data, 4096);

                if (count)
                    address = findsymbol(data, count, length, symbol);

            }

            if (address)
            {

                mapdata[i + width + 1] = 'A';

                writeaddress(&mapdata[i], address);

            }

        }

    }

}

/* the PCREL_HI20 results of a section, which its PCREL_LO12 relocations point back at */
static unsigned long hiplaces[4096];
static unsigned int hioffsets[4096];
static unsigned int hicount;

static unsigned int readword(unsigned int target, unsigned int id, unsigned int offset)
{

    unsigned int value;

    fs_read_full(1, target, id, &value, 4, offset);

    return value;

}

static void writedata(unsigned int write, unsigned int target, unsigned int id, void *data, unsigned int count, unsigned int offset)
{

    if (write)
        fs_write_all(1, target, id, data, count, offset);

}

static void writeword(unsigned int write, unsigned int target, unsigned int id, unsigned int offset, unsigned int value)
{

    writedata(write, target, id, &value, 4, offset);

}

static unsigned int findhi(unsigned long place, unsigned int *offset)
{

    unsigned int i;

    for (i = 0; i < hicount; i++)
    {

        if (hiplaces[i] == place)
        {

            *offset = hioffsets[i];

            return 1;

        }

    }

    return 0;

}

static unsigned int relocateriscv(unsigned int write, unsigned int target, unsigned int id, unsigned int type, unsigned long value, unsigned long place, unsigned int fileoffset)
{

    unsigned int offset = value - place;
    unsigned int hi = (offset + 0x800) & 0xFFFFF000;
    unsigned int lo = offset & 0xFFF;
    unsigned long data;
    unsigned int word;

    switch (type)
    {

    case ELF_RELOC_RISCV_32:
        writeword(write, target, id, fileoffset, value);

        return 1;

    case ELF_RELOC_RISCV_64:
        writedata(write, target, id, &value, 8, fileoffset);

        return 1;

    case ELF_RELOC_RISCV_ADD32:
        writeword(write, target, id, fileoffset, readword(target, id, fileoffset) + value);

        return 1;

    case ELF_RELOC_RISCV_SUB32:
        writeword(write, target, id, fileoffset, readword(target, id, fileoffset) - value);

        return 1;

    case ELF_RELOC_RISCV_ADD64:
    case ELF_RELOC_RISCV_SUB64:
        fs_read_full(1, target, id, &data, 8, fileoffset);

        data = (type == ELF_RELOC_RISCV_ADD64) ? data + value : data - value;

        writedata(write, target, id, &data, 8, fileoffset);

        return 1;

    case ELF_RELOC_RISCV_CALL:
    case ELF_RELOC_RISCV_CALLPLT:
        writeword(write, target, id, fileoffset, (readword(target, id, fileoffset) & 0xFFF) | hi);
        writeword(write, target, id, fileoffset + 4, (readword(target, id, fileoffset + 4) & 0xFFFFF) | (lo << 20));

        return 1;

    case ELF_RELOC_RISCV_PCRELHI20:
        if (hicount == 4096)
            return 0;

        hiplaces[hicount] = place;
        hioffsets[hicount] = offset;
        hicount++;

        writeword(write, target, id, fileoffset, (readword(target, id, fileoffset) & 0xFFF) | hi);

        return 1;

    case ELF_RELOC_RISCV_PCRELLO12I:
    case ELF_RELOC_RISCV_PCRELLO12S:
        if (!findhi(value, &offset))
            return 0;

        lo = offset & 0xFFF;
        word = readword(target, id, fileoffset);
        word = (type == ELF_RELOC_RISCV_PCRELLO12I) ? (word & 0xFFFFF) | (lo << 20) : (word & 0x01FFF07F) | ((lo & 0xFE0) << 20) | ((lo & 0x1F) << 7);

        writeword(write, target, id, fileoffset, word);

        return 1;

    case ELF_RELOC_RISCV_JAL:
        word = readword(target, id, fileoffset) & 0xFFF;
        word |= ((offset >> 20) & 1) << 31 | ((offset >> 1) & 0x3FF) << 21 | ((offset >> 11) & 1) << 20 | ((offset >> 12) & 0xFF) << 12;

        writeword(write, target, id, fileoffset, word);

        return 1;

    case ELF_RELOC_RISCV_BRANCH:
        word = readword(target, id, fileoffset) & 0x01FFF07F;
        word |= ((offset >> 12) & 1) << 31 | ((offset >> 5) & 0x3F) << 25 | ((offset >> 1) & 0xF) << 8 | ((offset >> 11) & 1) << 7;

        writeword(write, target, id, fileoffset, word);

        return 1;

    case ELF_RELOC_RISCV_RELAX:
        return 1;

    }

    return 0;

}

static unsigned int encodebranch(unsigned int word, unsigned long offset, unsigned int bits, unsigned int shift)
{

    unsigned long range = 1UL << (bits + 1);
    unsigned int mask = (1 << bits) - 1;

    if (offset + range >= range * 2)
        return 0;

    return (word & ~(mask << shift)) | (((offset >> 2) & mask) << shift);

}

static unsigned int encodeadr(unsigned int word, unsigned long value)
{

    return (word & 0x9F00001F) | ((value & 3) << 29) | (((value >> 2) & 0x7FFFF) << 5);

}

static unsigned int encodelo12(unsigned int word, unsigned long value, unsigned int shift)
{

    return (word & 0xFFC003FF) | (((value & 0xFFF) >> shift) << 10);

}

static unsigned int relocateaarch64(unsigned int write, unsigned int target, unsigned int id, unsigned int type, unsigned long value, unsigned long place, unsigned int fileoffset)
{

    unsigned long offset = value - place;
    unsigned long page = (value & ~0xFFFUL) - (place & ~0xFFFUL);
    unsigned int word = readword(target, id, fileoffset);

    switch (type)
    {

    case ELF_RELOC_A64_ABS64:
        writedata(write, target, id, &value, 8, fileoffset);

        return RELOCATE_OK;

    case ELF_RELOC_A64_PREL64:
        writedata(write, target, id, &offset, 8, fileoffset);

        return RELOCATE_OK;

    case ELF_RELOC_A64_ABS32:
        writeword(write, target, id, fileoffset, value);

        return RELOCATE_OK;

    case ELF_RELOC_A64_PREL32:
        writeword(write, target, id, fileoffset, offset);

        return RELOCATE_OK;

    case ELF_RELOC_A64_CALL26:
    case ELF_RELOC_A64_JUMP26:
        word = encodebranch(word, offset, 26, 0);

        break;

    case ELF_RELOC_A64_CONDBR19:
        word = encodebranch(word, offset, 19, 5);

        break;

    case ELF_RELOC_A64_TSTBR14:
        word = encodebranch(word, offset, 14, 5);

        break;

    case ELF_RELOC_A64_ADRLO21:
        word = encodeadr(word, offset);

        break;

    case ELF_RELOC_A64_ADRHI21:
    case ELF_RELOC_A64_ADRHI21NC:
        word = encodeadr(word, page >> 12);

        break;

    case ELF_RELOC_A64_ADDLO12:
    case ELF_RELOC_A64_LDST8:
        word = encodelo12(word, value, 0);

        break;

    case ELF_RELOC_A64_LDST16:
        word = encodelo12(word, value, 1);

        break;

    case ELF_RELOC_A64_LDST32:
        word = encodelo12(word, value, 2);

        break;

    case ELF_RELOC_A64_LDST64:
        word = encodelo12(word, value, 3);

        break;

    case ELF_RELOC_A64_LDST128:
        word = encodelo12(word, value, 4);

        break;

    default:
        return RELOCATE_UNSUPPORTED;

    }

    if (!word)
        return RELOCATE_RANGE;

    writeword(write, target, id, fileoffset, word);

    return RELOCATE_OK;

}

static unsigned int findunresolved32(unsigned int source, unsigned int target, unsigned int id, struct elf_sectionheader *sectionheaders, unsigned int count)
{

    unsigned int unresolved = 0;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (sectionheaders[i].type == ELF_SECTION_TYPE_REL || sectionheaders[i].type == ELF_SECTION_TYPE_RELA)
        {

            struct elf_sectionheader *relocationheader = &sectionheaders[i];
            struct elf_sectionheader *symbolheader = &sectionheaders[relocationheader->link];
            struct elf_sectionheader *stringheader = &sectionheaders[symbolheader->link];
            char strings[4096];
            unsigned int j;

            if (stringheader->size > 4096)
                PANIC(source);

            fs_read_full(1, target, id, strings, stringheader->size, stringheader->offset);

            for (j = 0; j < relocationheader->size / relocationheader->esize; j++)
            {

                struct elf_relocationa relocation;
                struct elf_symbol symbol;

                fs_read_full(1, target, id, &relocation, relocationheader->esize, relocationheader->offset + j * relocationheader->esize);
                fs_read_full(1, target, id, &symbol, symbolheader->esize, symbolheader->offset + (relocation.info >> 8) * symbolheader->esize);

                if (!symbol.shindex && !findsymbol(mapdata, mapcount, cstring_length(strings + symbol.name), strings + symbol.name))
                {

                    channel_send_fmt(0, source, MESSAGE_TYPE_ERROR, "Unresolved symbol: %s\n", strings + symbol.name);

                    unresolved++;

                }

            }

        }

    }

    return unresolved;

}

static unsigned int apply32(unsigned int source, unsigned int target, unsigned int id, struct elf_header *header, struct elf_sectionheader *sectionheaders, unsigned int base, unsigned int write)
{

    unsigned int unresolved = 0;
    unsigned int i;

    for (i = 0; i < header->shcount; i++)
    {

        if (sectionheaders[i].type == ELF_SECTION_TYPE_REL || sectionheaders[i].type == ELF_SECTION_TYPE_RELA)
        {

            struct elf_sectionheader *relocationheader = &sectionheaders[i];
            struct elf_sectionheader *dataheader = &sectionheaders[relocationheader->info];
            struct elf_sectionheader *symbolheader = &sectionheaders[relocationheader->link];
            struct elf_sectionheader *stringheader = &sectionheaders[symbolheader->link];
            char strings[4096];
            unsigned int j;

            if (stringheader->size > 4096)
                PANIC(source);

            fs_read_full(1, target, id, strings, stringheader->size, stringheader->offset);

            hicount = 0;

            for (j = 0; j < relocationheader->size / relocationheader->esize; j++)
            {

                struct elf_relocationa relocation;
                struct elf_symbol symbol;
                unsigned int address;
                unsigned int place;
                unsigned int offset;
                unsigned int value;

                fs_read_full(1, target, id, &relocation, relocationheader->esize, relocationheader->offset + j * relocationheader->esize);
                fs_read_full(1, target, id, &symbol, symbolheader->esize, symbolheader->offset + (relocation.info >> 8) * symbolheader->esize);

                if (relocationheader->type == ELF_SECTION_TYPE_RELA)
                    value = relocation.addend;
                else
                    fs_read_full(1, target, id, &value, 4, dataheader->offset + relocation.offset);

                address = (symbol.shindex) ? base + sectionheaders[symbol.shindex].offset + symbol.value : findsymbol(mapdata, mapcount, cstring_length(strings + symbol.name), strings + symbol.name);
                place = base + dataheader->offset + relocation.offset;

                if (header->machine == ELF_MACHINE_RISCV)
                {

                    if (!relocateriscv(write, target, id, relocation.info & 0xFF, address + relocation.addend, place, dataheader->offset + relocation.offset))
                    {

                        channel_send_fmt(0, source, MESSAGE_TYPE_ERROR, "Unsupported relocation: %s\n", strings + symbol.name);

                        unresolved++;

                    }

                    continue;

                }

                switch ((header->machine << 8) | (relocation.info & 0xFF))
                {

                case (ELF_MACHINE_386 << 8) | ELF_RELOC_TYPE_32:
                case (ELF_MACHINE_ARM << 8) | ELF_RELOC_ARM_ABS32:
                case (ELF_MACHINE_68K << 8) | ELF_RELOC_68K_32:
                    value += address;

                    break;

                case (ELF_MACHINE_386 << 8) | ELF_RELOC_TYPE_PC32:
                case (ELF_MACHINE_ARM << 8) | ELF_RELOC_ARM_REL32:
                case (ELF_MACHINE_68K << 8) | ELF_RELOC_68K_PC32:
                    value += address - place;

                    break;

                case (ELF_MACHINE_ARM << 8) | ELF_RELOC_ARM_CALL:
                case (ELF_MACHINE_ARM << 8) | ELF_RELOC_ARM_JUMP24:
                    offset = (value & 0x00FFFFFF) << 2;

                    if (offset & 0x02000000)
                        offset |= 0xFC000000;

                    offset += address - place;

                    if (offset + 0x02000000 >= 0x04000000)
                    {

                        channel_send_fmt(0, source, MESSAGE_TYPE_ERROR, "Branch out of range: %s\n", strings + symbol.name);

                        unresolved++;

                    }

                    value = (value & 0xFF000000) | ((offset >> 2) & 0x00FFFFFF);

                    break;

                case (ELF_MACHINE_ARM << 8) | ELF_RELOC_ARM_V4BX:
                    break;

                default:
                    channel_send_fmt(0, source, MESSAGE_TYPE_ERROR, "Unsupported relocation: %s\n", strings + symbol.name);

                    unresolved++;

                    break;

                }

                writedata(write, target, id, &value, 4, dataheader->offset + relocation.offset);

            }

        }

    }

    return unresolved;

}

static unsigned int link32(unsigned int source, unsigned int target, unsigned int id, struct elf_header *header, unsigned int base)
{

    struct elf_sectionheader sectionheaders[64];
    unsigned int textoffset = 0;
    unsigned int i;

    if (header->shcount >= 64)
        return 0;

    fs_read_full(1, target, id, sectionheaders, header->shsize * header->shcount, header->shoffset);

    if (findunresolved32(source, target, id, sectionheaders, header->shcount))
        return 0;

    for (i = 0; i < header->shcount; i++)
    {

        if (!textoffset && sectionheaders[i].type == ELF_SECTION_TYPE_PROGBITS)
            textoffset = sectionheaders[i].offset;

    }

    if (apply32(source, target, id, header, sectionheaders, base, 0))
        return 0;

    apply32(source, target, id, header, sectionheaders, base, 1);

    for (i = 0; i < header->shcount; i++)
    {

        if (sectionheaders[i].type == ELF_SECTION_TYPE_REL || sectionheaders[i].type == ELF_SECTION_TYPE_RELA)
        {

            sectionheaders[i].type = ELF_SECTION_TYPE_NULL;

            fs_write_all(1, target, id, &sectionheaders[i], header->shsize, header->shoffset + i * header->shsize);

        }

    }

    return textoffset;

}

static unsigned int findunresolved64(unsigned int source, unsigned int target, unsigned int id, struct elf64_sectionheader *sectionheaders, unsigned int count)
{

    unsigned int unresolved = 0;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (sectionheaders[i].type == ELF_SECTION_TYPE_RELA)
        {

            struct elf64_sectionheader *relocationheader = &sectionheaders[i];
            struct elf64_sectionheader *symbolheader = &sectionheaders[relocationheader->link];
            struct elf64_sectionheader *stringheader = &sectionheaders[symbolheader->link];
            char strings[4096];
            unsigned int j;

            if (stringheader->size > 4096)
                PANIC(source);

            fs_read_full(1, target, id, strings, stringheader->size, stringheader->offset);

            for (j = 0; j < relocationheader->size / relocationheader->esize; j++)
            {

                struct elf64_relocation relocation;
                struct elf64_symbol symbol;

                fs_read_full(1, target, id, &relocation, relocationheader->esize, relocationheader->offset + j * relocationheader->esize);
                fs_read_full(1, target, id, &symbol, symbolheader->esize, symbolheader->offset + relocation.symbol * symbolheader->esize);

                if (!symbol.shindex && !findsymbol(mapdata, mapcount, cstring_length(strings + symbol.name), strings + symbol.name))
                {

                    channel_send_fmt(0, source, MESSAGE_TYPE_ERROR, "Unresolved symbol: %s\n", strings + symbol.name);

                    unresolved++;

                }

            }

        }

    }

    return unresolved;

}

static unsigned int apply64(unsigned int source, unsigned int target, unsigned int id, struct elf64_header *header, struct elf64_sectionheader *sectionheaders, unsigned int base, unsigned int write)
{

    unsigned int unresolved = 0;
    unsigned int i;

    for (i = 0; i < header->shcount; i++)
    {

        if (sectionheaders[i].type == ELF_SECTION_TYPE_RELA)
        {

            struct elf64_sectionheader *relocationheader = &sectionheaders[i];
            struct elf64_sectionheader *dataheader = &sectionheaders[relocationheader->info];
            struct elf64_sectionheader *symbolheader = &sectionheaders[relocationheader->link];
            struct elf64_sectionheader *stringheader = &sectionheaders[symbolheader->link];
            char strings[4096];
            unsigned int j;

            if (stringheader->size > 4096)
                PANIC(source);

            fs_read_full(1, target, id, strings, stringheader->size, stringheader->offset);

            hicount = 0;

            for (j = 0; j < relocationheader->size / relocationheader->esize; j++)
            {

                struct elf64_relocation relocation;
                struct elf64_symbol symbol;
                unsigned long value;
                unsigned int value32;

                fs_read_full(1, target, id, &relocation, relocationheader->esize, relocationheader->offset + j * relocationheader->esize);
                fs_read_full(1, target, id, &symbol, symbolheader->esize, symbolheader->offset + relocation.symbol * symbolheader->esize);

                value = (symbol.shindex) ? base + sectionheaders[symbol.shindex].offset + symbol.value : findsymbol(mapdata, mapcount, cstring_length(strings + symbol.name), strings + symbol.name);
                value += relocation.addend;

                if (header->machine == ELF_MACHINE_AARCH64)
                {

                    unsigned int status = relocateaarch64(write, target, id, relocation.type, value, base + dataheader->offset + relocation.offset, dataheader->offset + relocation.offset);

                    if (status != RELOCATE_OK)
                    {

                        channel_send_fmt(0, source, MESSAGE_TYPE_ERROR, (status == RELOCATE_RANGE) ? "Branch out of range: %s\n" : "Unsupported relocation: %s\n", strings + symbol.name);

                        unresolved++;

                    }

                    continue;

                }

                if (header->machine == ELF_MACHINE_RISCV)
                {

                    if (!relocateriscv(write, target, id, relocation.type, value, base + dataheader->offset + relocation.offset, dataheader->offset + relocation.offset))
                    {

                        channel_send_fmt(0, source, MESSAGE_TYPE_ERROR, "Unsupported relocation: %s\n", strings + symbol.name);

                        unresolved++;

                    }

                    continue;

                }

                switch (relocation.type)
                {

                case ELF_RELOC64_TYPE_64:
                    writedata(write, target, id, &value, 8, dataheader->offset + relocation.offset);

                    break;

                case ELF_RELOC64_TYPE_32:
                case ELF_RELOC64_TYPE_32S:
                    value32 = value;

                    writedata(write, target, id, &value32, 4, dataheader->offset + relocation.offset);

                    break;

                case ELF_RELOC64_TYPE_PC32:
                case ELF_RELOC64_TYPE_PLT32:
                    value32 = value - (base + dataheader->offset + relocation.offset);

                    writedata(write, target, id, &value32, 4, dataheader->offset + relocation.offset);

                    break;

                default:
                    channel_send_fmt(0, source, MESSAGE_TYPE_ERROR, "Unsupported relocation: %s\n", strings + symbol.name);

                    unresolved++;

                    break;

                }

            }

        }

    }

    return unresolved;

}

static unsigned int link64(unsigned int source, unsigned int target, unsigned int id, struct elf64_header *header, unsigned int base)
{

    struct elf64_sectionheader sectionheaders[64];
    unsigned int textoffset = 0;
    unsigned int i;

    if (header->shcount >= 64)
        return 0;

    fs_read_full(1, target, id, sectionheaders, header->shsize * header->shcount, header->shoffset);

    if (findunresolved64(source, target, id, sectionheaders, header->shcount))
        return 0;

    for (i = 0; i < header->shcount; i++)
    {

        if (!textoffset && sectionheaders[i].type == ELF_SECTION_TYPE_PROGBITS)
            textoffset = sectionheaders[i].offset;

    }

    if (apply64(source, target, id, header, sectionheaders, base, 0))
        return 0;

    apply64(source, target, id, header, sectionheaders, base, 1);

    for (i = 0; i < header->shcount; i++)
    {

        if (sectionheaders[i].type == ELF_SECTION_TYPE_RELA)
        {

            sectionheaders[i].type = ELF_SECTION_TYPE_NULL;

            fs_write_all(1, target, id, &sectionheaders[i], header->shsize, header->shoffset + i * header->shsize);

        }

    }

    return textoffset;

}

static void onmain(struct message *message)
{

    kernelcount = fs_read_path(1, "initrd:kernel/fudge.map", kerneldata, 8192);

}

static void load(unsigned int source, char *path)
{

    unsigned int target = fs_auth(path);

    if (target)
    {

        unsigned int id = fs_walk(1, target, 0, path);

        if (id)
        {

            struct elf64_header header;

            if (fs_read_full(1, target, id, &header, ELF64_HEADER_SIZE, 0) >= ELF_HEADER_SIZE && elf_validate((struct elf_header *)&header))
            {

                unsigned int class = (sizeof (void *) == 8) ? ELF_IDENTITY_CLASS_64 : ELF_IDENTITY_CLASS_32;

                if (header.identify[ELF_IDENTITY_CLASS] != class)
                {

                    channel_send_fmt(0, source, MESSAGE_TYPE_ERROR, "Unsupported module format: %s\n", path);

                }

                else
                {

                    unsigned int address = fs_map(1, target, id);

                    if (address)
                    {

                        char mapname[256];

                        cstring_write_fmt(mapname, 256, 0, "%s.map\\0", path);

                        mapcount = fs_read_path(1, mapname, mapdata, 4096);

                        if (mapcount)
                        {

                            unsigned int textoffset;

                            updateundefined();

                            textoffset = (class == ELF_IDENTITY_CLASS_64) ? link64(source, target, id, &header, address) : link32(source, target, id, (struct elf_header *)&header, address);

                            if (textoffset)
                            {

                                relocate(address + textoffset);
                                fs_write_all(1, target, fs_walk(1, target, 0, mapname), mapdata, mapcount, 0);
                                call_load(target, id);

                            }

                        }

                    }

                }

            }

        }

    }

}

static void ondata(struct message *message)
{

    linescount += buffer_write(lines, 4096, message->data, message->length, linescount);

}

static void onterm(struct message *message)
{

    unsigned int start = 0;
    unsigned int end;

    for (end = 0; end < linescount; end++)
    {

        if (lines[end] == '\n')
        {

            lines[end] = '\0';

            if (end > start)
                load(message->source, lines + start);

            start = end + 1;

        }

    }

}

static void onpath(struct message *message)
{

    load(message->source, message->data);

}

void init(void)
{

    width = sizeof (void *) * 2;

    channel_bind(MESSAGE_TYPE_DATA, ondata);
    channel_bind(MESSAGE_TYPE_MAIN, onmain);
    channel_bind(MESSAGE_TYPE_PATH, onpath);
    channel_bind(MESSAGE_TYPE_TERM, onterm);

}

