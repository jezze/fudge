void event_open(void);
void event_close(void);
struct event_header *event_read(void *ibuffer);
struct event_header *event_init(void *ibuffer, void *obuffer);
unsigned int event_send(struct event_header *header);
