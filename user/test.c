#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{

    write(STDOUT_FILENO, "test1\n", 6);

    fputs("test2\n", stdout);

    FILE *out = fopen("stdout", "w");
    fputs("test3\n", out);
//    fclose(out);

    return 0;

}

