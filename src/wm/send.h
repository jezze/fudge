void send_wmmap(unsigned int destination);
void send_wmresize(unsigned int destination, unsigned int x, unsigned int y, unsigned int w, unsigned int h);
void send_wmdraw(unsigned int destination, unsigned int x, unsigned int y, unsigned int w, unsigned int h);
void send_wmquit(unsigned int destination);
