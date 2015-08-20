void send_wmmap(unsigned int descriptor, unsigned int destination);
void send_wmunmap(unsigned int descriptor, unsigned int destination);
void send_wmready(unsigned int descriptor, unsigned int destination);
void send_wmadd(unsigned int descriptor, unsigned int destination, unsigned int count, void *data);
