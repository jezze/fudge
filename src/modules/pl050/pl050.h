unsigned int pl050_hasdata(unsigned int id);
unsigned char pl050_getdata(unsigned int id);
unsigned char pl050_command(unsigned int id, unsigned char value);
unsigned char pl050_poll(unsigned int id);
void pl050_flush(unsigned int id);
void pl050_enable(unsigned int id);
void pl050_disable(unsigned int id);
