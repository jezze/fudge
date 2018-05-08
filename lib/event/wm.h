struct event_wmresize
{

    unsigned int x;
    unsigned int y;
    unsigned int w;
    unsigned int h;
    unsigned int padding;
    unsigned int lineheight;

};

struct event_wmkeypress
{

    unsigned char scancode;

};

struct event_wmkeyrelease
{

    unsigned char scancode;

};

struct event_wmmousepress
{

    unsigned int button;

};

struct event_wmmouserelease
{

    unsigned int button;

};

struct event_wmmousemove
{

    char relx;
    char rely;

};

void event_sendwmmap(unsigned int descriptor, unsigned int destination);
void event_sendwmunmap(unsigned int descriptor, unsigned int destination);
void event_sendwminit(unsigned int descriptor, unsigned int destination);
void event_sendwmexit(unsigned int descriptor, unsigned int destination);
void event_sendwmresize(unsigned int descriptor, unsigned int destination, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int padding, unsigned int lineheight);
void event_sendwmshow(unsigned int descriptor, unsigned int destination);
void event_sendwmhide(unsigned int descriptor, unsigned int destination);
void event_sendwmflush(unsigned int descriptor, unsigned int destination);
void event_sendwmkeypress(unsigned int descriptor, unsigned int destination, unsigned char scancode);
void event_sendwmkeyrelease(unsigned int descriptor, unsigned int destination, unsigned char scancode);
void event_sendwmmousemove(unsigned int descriptor, unsigned int destination, char relx, char rely);
void event_sendwmmousepress(unsigned int descriptor, unsigned int destination, unsigned int button);
void event_sendwmmouserelease(unsigned int descriptor, unsigned int destination, unsigned int button);
