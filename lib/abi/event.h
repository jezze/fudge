unsigned int event_pick(struct event_channel *channel);
unsigned int event_place(unsigned int id, struct event_channel *channel);
unsigned int event_listen(struct event_channel *channel);
void event_setsignal(unsigned int type, unsigned int (*callback)(struct event_channel *channel));
void event_initsignals(void);
