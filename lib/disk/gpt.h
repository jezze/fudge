struct gpt_header
{

    unsigned char signature[8];
    unsigned char revision[4];
    unsigned char headersize[4];
    unsigned char headercrc[4];
    unsigned char reserved[4];
    unsigned char currentlba[8];
    unsigned char backuplba[8];
    unsigned char firstlba[8];
    unsigned char lastlba[8];
    unsigned char guid[16];
    unsigned char entrylba[8];
    unsigned char entrycount[4];
    unsigned char entrysize[4];
    unsigned char entrycrc[4];

};

struct gpt_entry
{

    unsigned char type[16];
    unsigned char guid[16];
    unsigned char firstlba[8];
    unsigned char lastlba[8];
    unsigned char attributes[8];
    unsigned char name[72];

};

unsigned int gpt_read32(unsigned char *value);
unsigned int gpt_isused(struct gpt_entry *entry);
unsigned int gpt_validate(struct gpt_header *header);
