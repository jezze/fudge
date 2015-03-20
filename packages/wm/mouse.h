struct mouse
{

    unsigned int num;
    unsigned char state;
    struct box size;

};

void mouse_handle(struct mouse *mouse, unsigned char value);
void mouse_draw(struct mouse *mouse, unsigned int line);
void mouse_init(struct mouse *mouse);
