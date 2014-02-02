#include <fudge.h>
#include "token.h"
#include "parse.h"

static unsigned int open_path(unsigned int index, unsigned int indexw, unsigned int count, char *buffer)
{

    if (memory_match(buffer, "/", 1))
        return call_open(index, CALL_DR, count - 1, buffer + 1);

    return call_open(index, indexw, count, buffer);

}

static void open_pipe(unsigned int index, unsigned int index0, unsigned int index1)
{

    call_open(index, CALL_DR, 17, "system/pipe/clone");
    call_open(index0, index, 4, "../0");
    call_open(index1, index, 4, "../1");

}

static void close_pipe(unsigned int index, unsigned int index0, unsigned int index1)
{

    call_close(index0);
    call_close(index1);
    call_close(index);

}

static void redirect_input(struct input *input, char *buffer, unsigned int i, unsigned int p, unsigned int p0)
{

    if (input->path.count)
    {

        open_path(i, CALL_DW, input->path.count, buffer + input->path.index);

    }

    else if (input->data.count)
    {

        open_pipe(p, p0, i);
        call_write(p0, 0, input->data.count, buffer + input->data.index);

    }

}

static void redirect_output(struct output *output, char *buffer, unsigned int o)
{

    if (output->path.count)
        open_path(o, CALL_DW, output->path.count, buffer + output->path.index);

}

static void unredirect_input(struct input *input, unsigned int i, unsigned int p, unsigned int p0)
{

    if (input->path.count)
        call_close(i);
    else if (input->data.count)
        close_pipe(p, p0, i);

}

static void unredirect_output(struct output *output, unsigned int o)
{

    if (output->path.count)
        call_close(o);

}

static void execute_command(struct command *command, char *buffer)
{

    if (!open_path(CALL_E0, CALL_L4, command->binary.count, buffer + command->binary.index))
        return;

    redirect_input(&command->in0, buffer, CALL_I0, CALL_L5, CALL_L6);
    redirect_input(&command->in1, buffer, CALL_I1, CALL_L7, CALL_L8);
    redirect_output(&command->out0, buffer, CALL_O0);
    redirect_output(&command->out1, buffer, CALL_O1);
    call_spawn(CALL_E0);
    unredirect_input(&command->in0, CALL_I0, CALL_L5, CALL_L6);
    unredirect_input(&command->in1, CALL_I1, CALL_L7, CALL_L8);
    unredirect_output(&command->out0, CALL_O0);
    unredirect_output(&command->out1, CALL_O1);
    call_close(CALL_E0);

}

static void execute(struct token_state *state, struct expression *expression)
{

    unsigned int pindex;

    call_open(CALL_L0, CALL_I0, 0, 0);
    call_open(CALL_L1, CALL_I1, 0, 0);
    call_open(CALL_L2, CALL_O0, 0, 0);
    call_open(CALL_L3, CALL_O1, 0, 0);
    call_open(CALL_L4, CALL_DR, 4, "bin/");

    for (pindex = 0; pindex < expression->pipes; pindex++)
    {

        struct pipe *pipe = &expression->pipe[pindex];
        unsigned int clast = pipe->commands - 1;
        unsigned int cindex;

        for (cindex = 0; cindex < clast; cindex++)
            call_open(CALL_P0 + cindex, CALL_DR, 17, "system/pipe/clone");

        for (cindex = 0; cindex < clast; cindex++)
        {

            call_open(CALL_O0, CALL_P0 + cindex, 4, "../0");
            execute_command(&pipe->command[cindex], state->buffer);
            call_open(CALL_I0, CALL_P0 + cindex, 4, "../1");

        }

        for (cindex = 0; cindex < clast; cindex++)
            call_close(CALL_P0 + cindex);

        call_open(CALL_O0, CALL_L2, 0, 0);
        execute_command(&pipe->command[clast], state->buffer);

    }

    call_close(CALL_L0);
    call_close(CALL_L1);
    call_close(CALL_L2);
    call_close(CALL_L3);
    call_close(CALL_L4);

}

void main()
{

    char buffer[FUDGE_BSIZE];
    struct token_state state;
    struct expression expression;

    memory_clear(&expression, sizeof (struct expression));
    token_init_state(&state, call_read(CALL_I0, 0, FUDGE_BSIZE, buffer), buffer);

    if (parse(&state, &expression))
        execute(&state, &expression);

}

