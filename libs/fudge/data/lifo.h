struct lifo_stack
{

    char buffer[FUDGE_BSIZE];
    unsigned int head;


};

void lifo_stack_push(struct lifo_stack *stack, char c);
char lifo_stack_pop(struct lifo_stack *stack);
void lifo_stack_clear(struct lifo_stack *stack);
unsigned int lifo_stack_isempty(struct lifo_stack *stack);
