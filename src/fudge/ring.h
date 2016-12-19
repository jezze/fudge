struct ring
{

    char *buffer;
    unsigned int capacity;
    unsigned int head;
    unsigned int tail;

};

unsigned int ring_count(struct ring *ring);
unsigned int ring_isempty(struct ring *ring);
unsigned int ring_isfull(struct ring *ring);
unsigned int ring_skip(struct ring *ring, unsigned int count);
unsigned int ring_erase(struct ring *ring, unsigned int count);
unsigned int ring_read(struct ring *ring, void *buffer, unsigned int count);
unsigned int ring_write(struct ring *ring, void *buffer, unsigned int count);
unsigned int ring_overwrite(struct ring *ring, void *buffer, unsigned int count);
void ring_reset(struct ring *ring);
void ring_init(struct ring *ring, unsigned int capacity, void *buffer);
