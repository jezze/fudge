void event_open(void);
void event_close(void);
void event_read(union event_message *message);
unsigned int event_send(union event_message *message);
