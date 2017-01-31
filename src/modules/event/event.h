void event_notifykeypress(unsigned char scancode);
void event_notifykeyrelease(unsigned char scancode);
void event_notifymousemove(char relx, char rely);
void event_notifymousepress(unsigned int button);
void event_notifymouserelease(unsigned int button);
void event_notifytimertick(unsigned int counter);
void event_notifyvideomode(unsigned int w, unsigned int h, unsigned int bpp);
