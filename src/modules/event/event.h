void event_unicast(struct list *links, struct event_header *header, unsigned int count);
void event_multicast(struct list *links, struct event_header *header, unsigned int count);
unsigned int event_send(struct list *links, struct service_state *state, void *buffer, unsigned int count);
