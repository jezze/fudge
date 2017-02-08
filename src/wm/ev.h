struct ev_handlers
{

    void (*keypress)(struct event_header *header, struct event_keypress *keypress);
    void (*keyrelease)(struct event_header *header, struct event_keyrelease *keyrelease);
    void (*mousemove)(struct event_header *header, struct event_mousemove *mousemove);
    void (*mousepress)(struct event_header *header, struct event_mousepress *mousepress);
    void (*mouserelease)(struct event_header *header, struct event_mouserelease *mouserelease);
    void (*timertick)(struct event_header *header, struct event_timertick *timertick);
    void (*videomode)(struct event_header *header, struct event_videomode *videomode);
    void (*wmmap)(struct event_header *header);
    void (*wmunmap)(struct event_header *header);
    void (*wmresize)(struct event_header *header, struct event_wmresize *wmresize);
    void (*wmshow)(struct event_header *header);
    void (*wmhide)(struct event_header *header);
    void (*wmflush)(struct event_header *header);

};

unsigned int ev_read(struct ev_handlers *handlers, unsigned int descriptor);
void ev_sendkeypress(unsigned int descriptor, unsigned int destination, unsigned char scancode);
void ev_sendkeyrelease(unsigned int descriptor, unsigned int destination, unsigned char scancode);
void ev_sendmousemove(unsigned int descriptor, unsigned int destination, char relx, char rely);
void ev_sendmousepress(unsigned int descriptor, unsigned int destination, unsigned int button);
void ev_sendmouserelease(unsigned int descriptor, unsigned int destination, unsigned int button);
void ev_sendwmmap(unsigned int descriptor, unsigned int destination);
void ev_sendwmunmap(unsigned int descriptor, unsigned int destination);
void ev_sendwmresize(unsigned int descriptor, unsigned int destination, unsigned int x, unsigned int y, unsigned int w, unsigned int h);
void ev_sendwmshow(unsigned int descriptor, unsigned int destination);
void ev_sendwmhide(unsigned int descriptor, unsigned int destination);
void ev_sendwmflush(unsigned int descriptor, unsigned int destination, unsigned int desc2, struct ring *ring);
