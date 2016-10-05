void print_fill(struct buffer *buffer, unsigned int source, unsigned int z, struct element_fill *fill);
void print_mouse(struct buffer *buffer, unsigned int source, unsigned int z, struct element_mouse *mouse);
void print_panel(struct buffer *buffer, unsigned int source, unsigned int z, struct element_panel *panel);
void print_text(struct buffer *buffer, unsigned int source, unsigned int z, struct element_text *text, void *textbuffer, unsigned int count);
void print_window(struct buffer *buffer, unsigned int source, unsigned int z, struct element_window *window);
