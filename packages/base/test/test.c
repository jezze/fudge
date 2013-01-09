#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main()
{

    write(STDOUT_FILENO, "test1\n", 6);
    fputs("test2\n", stdout);

    FILE *out = fopen("stdout", "a");

/*
    fputs("test3\n", out);
    fclose(out);
*/

    return 0;

}

