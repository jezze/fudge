struct ev_handlers
{

    void (*keypress)(struct event_header *header, struct event_keypress *keypress);
    void (*keyrelease)(struct event_header *header, struct event_keyrelease *keyrelease);
    void (*mousemove)(struct event_header *header, struct event_mousemove *mousemove);
    void (*mousepress)(struct event_header *header, struct event_mousepress *mousepress);
    void (*mouserelease)(struct event_header *header, struct event_mouserelease *mouserelease);
    void (*tick)(struct event_header *header, struct event_tick *tick);
    void (*videomode)(struct event_header *header, struct event_videomode *videomode);
    void (*wmmap)(struct event_header *header);
    void (*wmunmap)(struct event_header *header);
    void (*wmresize)(struct event_header *header, struct event_wmresize *wmresize);
    void (*wmshow)(struct event_header *header);
    void (*wmhide)(struct event_header *header);

};

unsigned int ev_read(struct ev_handlers *handlers, unsigned int descriptor);
