void write_key(char *key);
void write_value(void *value, unsigned int count);
void write_keyvalue(char *key, void *value, unsigned int count);
void write_string(char *key, char *value);
void write_boolean(char *key, unsigned int value);
void write_dec(char *key, unsigned int value);
void write_hex(char *key, unsigned int value);
void write_ratio(char *key, unsigned int value1, unsigned int value2);
