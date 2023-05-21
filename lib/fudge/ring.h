struct ring
{

    char *buffer;
    unsigned int capacity;
    unsigned int head;
    unsigned int tail;

};

unsigned int ring_count(struct ring *ring);
unsigned int ring_avail(struct ring *ring);
unsigned int ring_isempty(struct ring *ring);
unsigned int ring_isfull(struct ring *ring);
unsigned int ring_skip(struct ring *ring, unsigned int count);
unsigned int ring_skip_reverse(struct ring *ring, unsigned int count);
unsigned int ring_read(struct ring *ring, void *buffer, unsigned int count);
unsigned int ring_read_all(struct ring *ring, void *buffer, unsigned int count);
unsigned int ring_read_reverse(struct ring *ring, void *buffer, unsigned int count);
unsigned int ring_read_allreverse(struct ring *ring, void *buffer, unsigned int count);
unsigned int ring_readcopy(struct ring *ring, void *buffer, unsigned int count);
unsigned int ring_write(struct ring *ring, void *buffer, unsigned int count);
unsigned int ring_write_all(struct ring *ring, void *buffer, unsigned int count);
unsigned int ring_write_reverse(struct ring *ring, void *buffer, unsigned int count);
unsigned int ring_write_allreverse(struct ring *ring, void *buffer, unsigned int count);
unsigned int ring_overwrite(struct ring *ring, void *buffer, unsigned int count);
unsigned int ring_move(struct ring *to, struct ring *from);
unsigned int ring_overmove(struct ring *to, struct ring *from);
unsigned int ring_copy(struct ring *to, struct ring *from);
unsigned int ring_overcopy(struct ring *to, struct ring *from);
unsigned int ring_each(struct ring *ring, char value);
unsigned int ring_find(struct ring *ring, char value);
unsigned int ring_find_reverse(struct ring *ring, char value);
void ring_reset(struct ring *ring);
void ring_init(struct ring *ring, unsigned int capacity, void *buffer);
