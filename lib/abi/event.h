void event_open(void);
void event_close(void);
unsigned int event_read(union event_message *message);
unsigned int event_send(union event_message *message);
