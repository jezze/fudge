struct buffer_cfifo
{

    unsigned char *buffer;
    unsigned int head;
    unsigned int tail;
    unsigned int size;

};

unsigned int buffer_read_cfifo(struct buffer_cfifo *cfifo, unsigned int count, void *buffer);
unsigned int buffer_write_cfifo(struct buffer_cfifo *cfifo, unsigned int count, void *buffer);
void buffer_init_cfifo(struct buffer_cfifo *cfifo, unsigned int size, void *buffer);
