unsigned int event_place(unsigned int id, union event_message *message);
unsigned int event_listen(struct event_channel *channel);
void event_setsignal(unsigned int type, unsigned int (*callback)(struct event_channel *channel));
void event_clearsignal(unsigned int type);
void event_initsignals(void);
