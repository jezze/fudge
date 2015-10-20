void send_wmmap(unsigned int descriptor, unsigned int destination);
void send_wmunmap(unsigned int descriptor, unsigned int destination);
void send_wmmapnotify(unsigned int descriptor, unsigned int destination, unsigned int x, unsigned int y, unsigned int w, unsigned int h);
void send_wmexpose(unsigned int descriptor, unsigned int destination, unsigned int x, unsigned int y, unsigned int w, unsigned int h);
