#define TEXT_TYPE_NORMAL                0x00
#define TEXT_TYPE_HIGHLIGHT             0x01

struct text_header
{

    unsigned int type;

};

struct text
{

    struct renderable base;
    struct text_header header;

};

void text_init(struct text *text, unsigned int type);
