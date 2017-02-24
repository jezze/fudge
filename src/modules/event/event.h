void event_unicast(struct list *links, struct event_header *header, unsigned int count);
void event_multicast(struct list *links, struct event_header *header, unsigned int count);
unsigned int event_send(struct list *links, struct task_descriptor *descriptor, void *buffer, unsigned int count);
