#define RENDERABLE_TYPE_WINDOW          1
#define RENDERABLE_TYPE_TEXT            2
#define RENDERABLE_TYPE_PANEL           3
#define RENDERABLE_TYPE_IMAGE           4

struct renderable
{

    struct list_item item;
    struct box size;
    unsigned int type;
    unsigned int count;
    unsigned int visible;

};

void renderable_register(unsigned int type, void (*renderer)(struct renderable *renderable, struct ctrl_videosettings *settings, void *drawdata, void *fontdata, unsigned int line));
void renderable_render(struct renderable *renderable, struct ctrl_videosettings *settings, void *drawdata, void *fontdata, unsigned int line);
void renderable_init(struct renderable *renderable, unsigned int type, unsigned int count);
