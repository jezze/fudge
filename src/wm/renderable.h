#define RENDERABLE_TYPE_WINDOW          1
#define RENDERABLE_TYPE_TEXT            2
#define RENDERABLE_TYPE_PANEL           3
#define RENDERABLE_TYPE_MOUSE           4

struct renderable_header
{

    struct box size;
    unsigned int id;
    unsigned int type;
    unsigned int source;
    unsigned int z;
    unsigned int count;

};

struct renderable
{

    struct renderable_header header;
    struct list_item item;
    unsigned int modified;

};

void renderable_init(struct renderable *renderable, unsigned int type, unsigned int count);
