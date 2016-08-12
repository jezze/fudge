struct buffer
{

    unsigned int capacity;
    unsigned int count;
    char *memory;
    char *head;
    char *tail;

};

unsigned int buffer_skip(struct buffer *buffer, unsigned int count);
unsigned int buffer_erase(struct buffer *buffer, unsigned int count);
unsigned int buffer_read(struct buffer *buffer, void *memory, unsigned int count);
unsigned int buffer_write(struct buffer *buffer, void *memory, unsigned int count);
unsigned int buffer_copy(struct buffer *buffer, void *memory, unsigned int count);
void buffer_init(struct buffer *buffer, unsigned int capacity, void *memory);
