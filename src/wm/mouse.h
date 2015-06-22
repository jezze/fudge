struct mouse
{

    unsigned int num;
    unsigned char state;
    char relx;
    char rely;
    struct box size;

};

void mouse_handle(struct mouse *mouse, unsigned char value);
void mouse_draw(struct mouse *mouse, struct ctrl_videosettings *settings, unsigned int line);
void mouse_init(struct mouse *mouse);
