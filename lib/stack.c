#include <lib/stack.h>

void stack_init(struct stack *stack, char *buffer, unsigned int size)
{

    stack->buffer = buffer;
    stack->size = size;
    stack->head = 0;

}

void stack_push(struct stack *stack, char c)
{

    if (stack->head < stack->size)
    {

        stack->buffer[stack->head] = c;
        stack->head++;

    }

}

char stack_pop(struct stack *stack)
{

    if (stack->head > 0)
    {

        char c = stack->buffer[stack->head - 1];
        stack->head--;

        return c;

    }

    return 0;

}

void stack_clear(struct stack *stack)
{

    stack->head = 0;

}

