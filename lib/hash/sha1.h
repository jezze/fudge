struct sha1
{

    unsigned int lo;
    unsigned int hi;
    unsigned int a;
    unsigned int b;
    unsigned int c;
    unsigned int d;
    unsigned int e;
    unsigned char buffer[64];

};

void sha1_read(struct sha1 *s, void *buffer, unsigned int count);
void sha1_write(struct sha1 *s, unsigned char digest[20]);
void sha1_init(struct sha1 *s);
