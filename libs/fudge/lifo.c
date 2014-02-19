#include "define.h"
#include "lifo.h"

void lifo_stack_push(struct lifo_stack *stack, char c)
{

    if (stack->head < FUDGE_BSIZE)
    {

        stack->buffer[stack->head] = c;
        stack->head++;

    }

}

char lifo_stack_pop(struct lifo_stack *stack)
{

    return (stack->head > 0) ? stack->buffer[--stack->head] : 0;

}

void lifo_stack_clear(struct lifo_stack *stack)
{

    stack->head = 0;

}

unsigned int lifo_stack_isempty(struct lifo_stack *stack)
{

    return !stack->head;

}

