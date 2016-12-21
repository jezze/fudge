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
unsigned int ring_skipreverse(struct ring *ring, unsigned int count);
unsigned int ring_read(struct ring *ring, void *buffer, unsigned int count);
unsigned int ring_readreverse(struct ring *ring, void *buffer, unsigned int count);
unsigned int ring_write(struct ring *ring, void *buffer, unsigned int count);
unsigned int ring_writereverse(struct ring *ring, void *buffer, unsigned int count);
unsigned int ring_overwrite(struct ring *ring, void *buffer, unsigned int count);
unsigned int ring_move(struct ring *to, struct ring *from);
unsigned int ring_overmove(struct ring *to, struct ring *from);
unsigned int ring_copy(struct ring *to, struct ring *from);
unsigned int ring_overcopy(struct ring *to, struct ring *from);
unsigned int ring_find(struct ring *ring, char value);
unsigned int ring_findreverse(struct ring *ring, char value);
void ring_init(struct ring *ring, unsigned int capacity, void *buffer);
