struct lifo_stack
{

    char *buffer;
    unsigned int size;
    unsigned int head;

};

unsigned int lifo_stack_push(struct lifo_stack *stack, unsigned int count, void *data);
unsigned int lifo_stack_pop(struct lifo_stack *stack, unsigned int count);
void lifo_stack_clear(struct lifo_stack *stack);
void lifo_stack_init(struct lifo_stack *stack, unsigned int size, void *buffer);
