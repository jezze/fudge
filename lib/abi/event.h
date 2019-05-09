#define EVENTS                          64

unsigned int event_pick(struct event_channel *channel);
unsigned int event_place(unsigned int id, struct event_channel *channel);
unsigned int event_listen(unsigned int (*signals[EVENTS])(struct event_channel *channel), struct event_channel *channel);
void event_setsignal(unsigned int (*signals[EVENTS])(struct event_channel *channel), unsigned int type, unsigned int (*callback)(struct event_channel *channel));
void event_initsignals(unsigned int (*signals[EVENTS])(struct event_channel *channel));
