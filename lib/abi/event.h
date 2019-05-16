unsigned int event_place(unsigned int id, union event_message *message);
unsigned int event_listen(struct event_channel *channel);
void event_setsignal(struct event_channel *channel, unsigned int type, void (*callback)(struct event_channel *channel));
void event_clearsignal(struct event_channel *channel, unsigned int type);
void event_initsignals(struct event_channel *channel);
void event_forward(struct event_channel *channel, unsigned int type);
void event_request(struct event_channel *channel, unsigned int type, unsigned int session);
void event_reply(struct event_channel *channel, unsigned int type);
