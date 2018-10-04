void event_open(void);
void event_close(void);
struct event_header *event_read(void *message);
unsigned int event_send(struct event_header *header);
