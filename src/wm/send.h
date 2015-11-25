void send_keypress(unsigned int descriptor, unsigned int destination, unsigned char scancode);
void send_keyrelease(unsigned int descriptor, unsigned int destination, unsigned char scancode);
void send_wmmap(unsigned int descriptor);
void send_wmunmap(unsigned int descriptor, unsigned int destination);
void send_wmresize(unsigned int descriptor, unsigned int destination, unsigned int x, unsigned int y, unsigned int w, unsigned int h);
void send_wmshow(unsigned int descriptor, unsigned int destination);
void send_wmhide(unsigned int descriptor, unsigned int destination);
