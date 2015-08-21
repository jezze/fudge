struct text
{

    struct renderable base;
    unsigned int color;
    void *data;
    unsigned int count;
    char *string;

};

void text_render(struct renderable *self, struct ctrl_videosettings *settings, unsigned int line);
void text_assign(struct text *text, void *data, unsigned int count, char *string);
void text_init(struct text *text, unsigned int color);
