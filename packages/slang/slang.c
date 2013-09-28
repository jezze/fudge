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

static void execute_command(struct command *command, char *buffer)
{

    if (!open_path(CALL_E0, CALL_L3, command->binary.count, buffer + command->binary.index))
        return;

    if (command->in0.path.count)
    {

        open_path(CALL_I0, CALL_DW, command->in0.path.count, buffer + command->in0.path.index);

    }

    else if (command->in0.data.count)
    {

        open_pipe(CALL_L5, CALL_L6, CALL_I0);
        call_write(CALL_L6, 0, command->in0.data.count, buffer + command->in0.data.index);

    }

    if (command->in1.path.count)
    {

        open_path(CALL_I1, CALL_DW, command->in1.path.count, buffer + command->in1.path.index);

    }

    else if (command->in1.data.count)
    {

        open_pipe(CALL_L7, CALL_L8, CALL_I1);
        call_write(CALL_L8, 0, command->in1.data.count, buffer + command->in1.data.index);

    }

    if (command->out0.path.count)
        open_path(CALL_O0, CALL_DW, command->out0.path.count, buffer + command->out0.path.index);

    if (command->out1.path.count)
        open_path(CALL_O1, CALL_DW, command->out1.path.count, buffer + command->out1.path.index);

    call_spawn(CALL_E0);

    if (command->in0.path.count)
        call_close(CALL_I0);
    else if (command->in0.data.count)
        close_pipe(CALL_L5, CALL_L6, CALL_I0);

    if (command->in1.path.count)
        call_close(CALL_I1);
    else if (command->in1.data.count)
        close_pipe(CALL_L7, CALL_L8, CALL_I1);

    if (command->out0.path.count)
        call_close(CALL_O0);

    if (command->out1.path.count)
        call_close(CALL_O1);

    call_close(CALL_E0);

}

static void execute(struct token_state *state, struct expression *expression)
{

    unsigned int pindex;

    call_open(CALL_L0, CALL_I0, 0, 0);
    call_open(CALL_L1, CALL_O0, 0, 0);
    call_open(CALL_L2, CALL_I1, 0, 0);
    call_open(CALL_L3, CALL_DR, 4, "bin/");

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

        call_open(CALL_O0, CALL_L1, 0, 0);
        execute_command(&pipe->command[clast], state->buffer);

    }

    call_close(CALL_L0);
    call_close(CALL_L1);
    call_close(CALL_L2);
    call_close(CALL_L3);

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

