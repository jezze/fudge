#include <fudge.h>
#include <kernel.h>
#include <modules/block/block.h>
#include "ext2.h"

static void read_superblock(struct base_block *interface, struct base_device *device, struct ext2_superblock *sb)
{

    char buffer[1024];

    interface->read(device, 2, 2, buffer);

    memory_copy(sb, buffer, sizeof (struct ext2_superblock));

}

static void read_blockgroup(struct base_block *interface, struct base_device *device, unsigned int id, struct ext2_blockgroup *bg)
{

    struct ext2_superblock sb;
    unsigned char buffer[1024];
    unsigned int blocksize;
    unsigned int sectorsize;
    unsigned int nodegroup;

    read_superblock(interface, device, &sb);

    blocksize = 1024 << sb.blockSize;
    sectorsize = blocksize / 512;
    nodegroup = (id - 1) / sb.nodeCountGroup;

    interface->read(device, 2 * sectorsize, sectorsize, buffer);
    memory_copy(bg, buffer + nodegroup * sizeof (struct ext2_blockgroup), sizeof (struct ext2_blockgroup));

}

static void read_node(struct base_block *interface, struct base_device *device, unsigned int id, struct ext2_blockgroup *bg, struct ext2_node *node)
{

    struct ext2_superblock sb;
    unsigned char buffer[1024];
    unsigned int blocksize;
    unsigned int sectorsize;
    unsigned int nodesize;
    unsigned int nodeindex;
    unsigned int nodeblock;

    read_superblock(interface, device, &sb);

    blocksize = 1024 << sb.blockSize;
    sectorsize = blocksize / 512;
    nodesize = sb.nodeSize;
    nodeindex = (id - 1) % sb.nodeCountGroup;
    nodeblock = (id * nodesize) / blocksize;

    interface->read(device, (bg->blockTableAddress + nodeblock) * sectorsize, sectorsize, buffer);
    memory_copy(node, buffer + nodesize * (nodeindex % (blocksize / nodesize)), sizeof (struct ext2_node));

}

static void read_content(struct base_block *interface, struct base_device *device, struct ext2_node *node, void *buffer)
{

    struct ext2_superblock sb;
    unsigned int blocksize;
    unsigned int sectorsize;

    read_superblock(interface, device, &sb);

    blocksize = 1024 << sb.blockSize;
    sectorsize = blocksize / 512;

    interface->read(device, (node->pointer0) * sectorsize, sectorsize, buffer);

}

static unsigned int validate(struct base_block *interface, struct base_device *device)
{

    struct ext2_superblock sb;

    read_superblock(interface, device, &sb);

    if (sb.signature != 0xEF53)
        return 0;

    return 1;

}

void ext2_init_protocol(struct ext2_protocol *protocol)
{

    block_init_protocol(&protocol->base, "ext2");

    protocol->validate = validate;
    protocol->read_blockgroup = read_blockgroup;
    protocol->read_node = read_node;
    protocol->read_content = read_content;

}

