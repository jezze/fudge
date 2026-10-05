struct fat32
{
    unsigned int table_size_32;
    unsigned short extended_flags;
    unsigned short fat_version;
    unsigned int root_cluster;
    unsigned short fat_info;
    unsigned short backup_BS_sector;
    unsigned char reserved_0[12];
    unsigned char drive_number;
    unsigned char reserved_1;
    unsigned char boot_signature;
    unsigned int volume_id;
    unsigned char volume_label[11];
    unsigned char fat_type_label[8];

} __attribute__((packed));

struct fat16
{
    unsigned char bios_drive_num;
    unsigned char reserved1;
    unsigned char boot_signature;
    unsigned int volume_id;
    unsigned char volume_label[11];
    unsigned char fat_type_label[8];

} __attribute__((packed));

struct fat
{
    unsigned char bootjmp[3];
    unsigned char oem_name[8];
    unsigned short bytes_per_sector;
    unsigned char sectors_per_cluster;
    unsigned short reserved_sector_count;
    unsigned char table_count;
    unsigned short root_entry_count;
    unsigned short total_sectors_16;
    unsigned char media_type;
    unsigned short table_size_16;
    unsigned short sectors_per_track;
    unsigned short head_side_count;
    unsigned int hidden_sector_count;
    unsigned int total_sectors_32;
    unsigned char extended_section[54];

} __attribute__((packed));

#define FAT_ATTRIBUTE_VOLUME            0x08
#define FAT_ATTRIBUTE_ARCHIVE           0x20
#define FAT_ATTRIBUTE_DIRECTORY         0x10
#define FAT_ATTRIBUTE_LONGNAME          0x0F
#define FAT_LOWERCASE_NAME              0x08
#define FAT_LOWERCASE_EXTENSION         0x10
#define FAT_ENTRY_END                   0x00
#define FAT_ENTRY_DELETED               0xE5
#define FAT_CLUSTER_MASK                0x0FFFFFFF
#define FAT_CLUSTER_END                 0x0FFFFFF8

struct fat_entry
{

    unsigned char name[11];
    unsigned char attributes;
    unsigned char lowercase;
    unsigned char createtimefine;
    unsigned short createtime;
    unsigned short createdate;
    unsigned short accessdate;
    unsigned short clusterhigh;
    unsigned short modifytime;
    unsigned short modifydate;
    unsigned short clusterlow;
    unsigned int size;

} __attribute__((packed));

struct fat_longname
{

    unsigned char order;
    unsigned short name1[5];
    unsigned char attributes;
    unsigned char type;
    unsigned char checksum;
    unsigned short name2[6];
    unsigned short cluster;
    unsigned short name3[2];

} __attribute__((packed));

unsigned int fat_validate(struct fat *fat);
