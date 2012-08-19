#include <memory.h>
#include <modules.h>
#include <block/block.h>
#include <ext2/ext2.h>

static void read_superblock(struct block_interface *interface, struct ext2_superblock *sb)
{

    char buffer[1024];

    interface->read(interface, 2, 2, buffer);

    memory_copy(sb, buffer, sizeof (struct ext2_superblock));

}

static void read_blockgroup(struct block_interface *interface, unsigned int id, struct ext2_blockgroup *bg)
{

    char buffer[1024];
    unsigned int blocksize;
    unsigned int sectorsize;
    unsigned int nodegroup;
    struct ext2_superblock sb;

    read_superblock(interface, &sb);

    blocksize = 1024 << sb.blockSize;
    sectorsize = blocksize / 512;
    nodegroup = (id - 1) / sb.nodeCountGroup;

    interface->read(interface, 2 * sectorsize, sectorsize, buffer);

    memory_copy(bg, buffer + nodegroup * sizeof (struct ext2_blockgroup), sizeof (struct ext2_blockgroup));

}

static void read_node(struct block_interface *interface, unsigned int id, struct ext2_blockgroup *bg, struct ext2_node *node)
{

    char buffer[1024];
    unsigned int blocksize;
    unsigned int sectorsize;
    unsigned int nodesize;
    unsigned int nodeindex;
    unsigned int nodeblock;
    struct ext2_superblock sb;

    read_superblock(interface, &sb);

    blocksize = 1024 << sb.blockSize;
    sectorsize = blocksize / 512;
    nodesize = sb.nodeSize;
    nodeindex = (id - 1) % sb.nodeCountGroup;
    nodeblock = (id * nodesize) / blocksize;

    interface->read(interface, (bg->blockTableAddress + nodeblock) * sectorsize, sectorsize, buffer);

    memory_copy(node, buffer + nodesize * (nodeindex % (blocksize / nodesize)), sizeof (struct ext2_node));

}

static void read_content(struct block_interface *interface, struct ext2_node *node, void *buffer)
{

    unsigned int blocksize;
    unsigned int sectorsize;
    struct ext2_superblock sb;

    read_superblock(interface, &sb);

    blocksize = 1024 << sb.blockSize;
    sectorsize = blocksize / 512;

    interface->read(interface, (node->pointer0) * sectorsize, sectorsize, buffer);

}

static unsigned int validate(struct block_interface *interface)
{

    struct ext2_superblock sb;

    read_superblock(interface, &sb);

    if (sb.signature != 0xEF53)
        return 0;

    return 1;

}

void ext2_driver_init(struct ext2_driver *driver)
{

    memory_clear(driver, sizeof (struct ext2_driver));

    modules_driver_init(&driver->base, EXT2_DRIVER_TYPE, "ext2", 0, 0, 0);

    driver->validate = validate;
    driver->read_blockgroup = read_blockgroup;
    driver->read_node = read_node;
    driver->read_content = read_content;

}

