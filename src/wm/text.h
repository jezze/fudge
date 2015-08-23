struct text
{

    struct renderable base;
    unsigned int color;
    unsigned int count;
    char *string;

};

void text_render(struct renderable *self, struct ctrl_videosettings *settings, void *drawdata, void *fontdata, unsigned int line);
void text_assign(struct text *text, unsigned int count, char *string);
void text_init(struct text *text, unsigned int color);
