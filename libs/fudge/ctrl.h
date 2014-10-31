enum
{

    CTRL_TYPE_NULL,
    CTRL_TYPE_NETWORKCTRL,
    CTRL_TYPE_VIDEOCTRL

};

struct ctrl_header
{

    unsigned int type;

};

struct ctrl_networkctrl
{

    struct ctrl_header header;
    unsigned char mac[6];

};

struct ctrl_videoctrl
{

    struct ctrl_header header;
    unsigned int w;
    unsigned int h;
    unsigned int bpp;

};

void ctrl_init_header(struct ctrl_header *header, unsigned int type);
void ctrl_init_networkctrl(struct ctrl_networkctrl *networkctrl);
void ctrl_init_videoctrl(struct ctrl_videoctrl *videoctrl);
