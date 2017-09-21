#include <abi.h>
#include <fudge.h>
#include <common.h>

struct bpb
{

	unsigned char bootjmp[3];
    unsigned char oem_name[8];
    unsigned char bytes_per_sector[2];
    unsigned char sectors_per_cluster;
    unsigned char reserved_sector_count[2];
    unsigned char table_count;
    unsigned char root_entry_count[2];
    unsigned char total_sectors_16[2];
    unsigned char media_type;
    unsigned char table_size_16[2];
    unsigned char sectors_per_track[2];
    unsigned char head_side_count[2];
    unsigned char hidden_sector_count[4];
    unsigned char total_sectors_32[4];

};

struct fat32
{

	struct bpb bpb;
    unsigned char table_size_32[4];
    unsigned char extended_flags[2];
    unsigned char fat_version[2];
    unsigned char root_cluster[4];
    unsigned char fat_info[2];
    unsigned char backup_BS_sector[2];
    unsigned char reserved_0[12];
    unsigned char drive_number;
    unsigned char reserved_1;
    unsigned char boot_signature;
    unsigned char volume_id[4];
    unsigned char volume_label[11];
    unsigned char fat_type_label[8];
 
};
 
struct fat16
{

	struct bpb bpb;
    unsigned char bios_drive_num;
    unsigned char reserved1;
    unsigned char boot_signature;
    unsigned char volume_id[4];
    unsigned char volume_label[11];
    unsigned char fat_type_label[8];
 
};
 
void main(void)
{

	char data[512];
    struct bpb *bpb = (struct bpb *)data;

    file_open(FILE_PI);
    file_readall(FILE_PI, data, 512);
    file_close(FILE_PI);
    file_open(FILE_PO);
    write_dec("Table count", bpb->table_count);
    file_close(FILE_PO);

}

