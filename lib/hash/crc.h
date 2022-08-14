struct crc
{

    unsigned int sum;
    unsigned int total;

};

void crc_read(struct crc *s, void *buffer, unsigned int count);
unsigned int crc_finalize(struct crc *s);
