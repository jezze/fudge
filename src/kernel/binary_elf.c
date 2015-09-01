#include <fudge.h>
#include <format/elf.h>
#include "resource.h"
#include "vfs.h"
#include "binary.h"

static struct binary_protocol protocol;

static unsigned int readheader(struct vfs_channel *channel, unsigned int id, struct elf_header *header)
{

    return channel->protocol->read(channel->backend, id, 0, ELF_HEADER_SIZE, header);

}

static unsigned int readprogramheader(struct vfs_channel *channel, unsigned int id, struct elf_header *header, unsigned int index, struct elf_programheader *programheader)
{

    return channel->protocol->read(channel->backend, id, header->phoffset + index * header->phsize, header->phsize, programheader);

}

static unsigned int readsectionheader(struct vfs_channel *channel, unsigned int id, struct elf_header *header, unsigned int index, struct elf_sectionheader *sectionheader)
{

    return channel->protocol->read(channel->backend, id, header->shoffset + index * header->shsize, header->shsize, sectionheader);

}

static unsigned int readsectionheadertype(struct vfs_channel *channel, unsigned int id, struct elf_header *header, unsigned int type, struct elf_sectionheader *sectionheader)
{

    unsigned int i;

    for (i = 0; i < header->shcount; i++)
    {

        if (!readsectionheader(channel, id, header, i, sectionheader))
            return 0;

        if (sectionheader->type == type)
            return 1;

    }

    return 0;

}

static unsigned int protocol_match(struct vfs_channel *channel, unsigned int id)
{

    struct elf_header header;

    if (!readheader(channel, id, &header))
        return 0;

    return elf_validate(&header);

}

static unsigned long protocol_findsymbol(struct vfs_channel *channel, unsigned int id, unsigned int count, char *symbolname)
{

    struct elf_header header;
    struct elf_sectionheader symbolheader;
    struct elf_sectionheader stringheader;
    char strings[4096];
    unsigned int i;

    if (!readheader(channel, id, &header))
        return 0;

    if (!readsectionheadertype(channel, id, &header, ELF_SECTION_TYPE_SYMTAB, &symbolheader))
        return 0;

    if (!readsectionheader(channel, id, &header, symbolheader.link, &stringheader))
        return 0;

    if (stringheader.size > 4096)
        return 0;

    if (!channel->protocol->read(channel->backend, id, stringheader.offset, stringheader.size, strings))
        return 0;

    for (i = 0; i < symbolheader.size / symbolheader.esize; i++)
    {

        struct elf_symbol symbol;
        char *s;

        if (!channel->protocol->read(channel->backend, id, symbolheader.offset + i * symbolheader.esize, symbolheader.esize, &symbol))
            return 0;

        if (!symbol.shindex)
            continue;

        s = strings + symbol.name;

        if (s[count] == '\0' && memory_match(symbolname, s, count))
        {

            struct elf_sectionheader referenceheader;

            if (!readsectionheader(channel, id, &header, symbol.shindex, &referenceheader))
                return 0;

            return symbol.value + referenceheader.address + referenceheader.offset;

        }

    }

    return 0;

}

static unsigned long protocol_findentry(struct vfs_channel *channel, unsigned int id)
{

    struct elf_header header;

    if (!readheader(channel, id, &header))
        return 0;

    return header.entry;

}

static unsigned long protocol_findbase(struct vfs_channel *channel, unsigned int id, unsigned long address)
{

    struct elf_header header;
    unsigned int i;

    if (!readheader(channel, id, &header))
        return 0;

    for (i = 0; i < header.phcount; i++)
    {

        struct elf_programheader programheader;

        if (!readprogramheader(channel, id, &header, i, &programheader))
            return 0;

        if (programheader.vaddress <= address && programheader.vaddress + programheader.msize > address)
            return programheader.vaddress;

    }

    return 0;

}

static unsigned int protocol_copyprogram(struct vfs_channel *channel, unsigned int id)
{

    struct elf_header header;
    unsigned int i;

    if (!readheader(channel, id, &header))
        return 0;

    for (i = 0; i < header.phcount; i++)
    {

        struct elf_programheader programheader;

        if (!readprogramheader(channel, id, &header, i, &programheader))
            return 0;

        if (!channel->protocol->read(channel->backend, id, programheader.offset, programheader.fsize, (void *)programheader.vaddress))
            return 0;

        memory_clear((void *)(programheader.vaddress + programheader.offset + programheader.fsize), programheader.msize - programheader.fsize);

    }

    return 1;

}

static unsigned int protocol_relocate(struct vfs_channel *channel, unsigned int id, unsigned int address)
{

    struct elf_header header;
    struct elf_sectionheader relocationheader;
    struct elf_sectionheader dataheader;
    struct elf_sectionheader symbolheader;
    unsigned int i;

    if (!readheader(channel, id, &header))
        return 0;

    /* REMOVE */
    for (i = 0; i < header.shcount; i++)
    {

        struct elf_sectionheader referenceheader;

        if (!readsectionheader(channel, id, &header, i, &referenceheader))
            return 0;

        referenceheader.address += address;

        if (!channel->protocol->write(channel->backend, id, header.shoffset + i * header.shsize, header.shsize, &referenceheader))
            return 0;

    }

    if (!readsectionheadertype(channel, id, &header, ELF_SECTION_TYPE_REL, &relocationheader))
        return 0;

    if (!readsectionheader(channel, id, &header, relocationheader.link, &symbolheader))
        return 0;

    if (!readsectionheader(channel, id, &header, relocationheader.info, &dataheader))
        return 0;

    for (i = 0; i < relocationheader.size / relocationheader.esize; i++)
    {

        struct elf_relocation relocation;
        struct elf_symbol symbol;
        unsigned char type;
        unsigned char index;
        unsigned long *entry;
        unsigned int addend;

        if (!channel->protocol->read(channel->backend, id, relocationheader.offset + i * relocationheader.esize, relocationheader.esize, &relocation))
            return 0;

        type = relocation.info & 0x0F;
        index = relocation.info >> 8;

        if (!channel->protocol->read(channel->backend, id, symbolheader.offset + index * symbolheader.esize, symbolheader.esize, &symbol))
            return 0;

        entry = (unsigned long *)(address + dataheader.offset + relocation.offset);
        addend = 0;

        if (symbol.shindex)
        {

            struct elf_sectionheader referenceheader;

            if (!readsectionheader(channel, id, &header, symbol.shindex, &referenceheader))
                return 0;

            addend = address + referenceheader.offset + symbol.value;

        }

        switch (type)
        {

        case ELF_RELOC_TYPE_32:
            *entry += addend;

            break;

        case ELF_RELOC_TYPE_PC32:
            *entry += addend - (unsigned long)entry;

            break;

        }

    }

    return 1;

}

void binary_setupelf(void)
{

    binary_initprotocol(&protocol, protocol_match, protocol_findsymbol, protocol_findentry, protocol_findbase, protocol_copyprogram, protocol_relocate);
    resource_register(&protocol.resource);

}

