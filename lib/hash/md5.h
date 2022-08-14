struct md5
{

    unsigned int lo;
    unsigned int hi;
    unsigned int a;
    unsigned int b;
    unsigned int c;
    unsigned int d;
    unsigned char buffer[64];

};

void md5_read(struct md5 *s, void *buffer, unsigned int count);
void md5_write(struct md5 *s, unsigned char digest[16]);
void md5_init(struct md5 *s);
