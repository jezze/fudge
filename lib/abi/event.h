#define EVENTS                          64

unsigned int event_pick(struct event_channel *channel);
unsigned int event_place(unsigned int id, struct event_channel *channel);
void event_initsignals(unsigned int (*signals[EVENTS])(struct event_channel *channel));
