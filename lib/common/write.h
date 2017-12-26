void write_keybuffer(unsigned int descriptor, char *key, void *data, unsigned int length);
void write_keystring(unsigned int descriptor, char *key, char *value);
void write_keyboolean(unsigned int descriptor, char *key, unsigned int value);
void write_keydec(unsigned int descriptor, char *key, unsigned int value);
void write_keyhex(unsigned int descriptor, char *key, unsigned int value);
