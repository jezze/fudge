unsigned int kv_match(struct message_wmevent *event, char *key);
unsigned int kv_getvalue(struct message_wmevent *event, char *key, unsigned int base);
char *kv_getstring(struct message_wmevent *event, char *key);
