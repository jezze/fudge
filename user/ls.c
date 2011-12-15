#include <fudge.h>

char buffer[0x1000];

void write_content(char *content)
{

    unsigned int start = 0;
    unsigned int i;

    for (i = 0; content[i] != '\0'; i++)
    {

        if (content[i] == '\n')
        {

            content[i] = '\0';

            if (1)
                file_write_format(FILE_STDOUT, "d");
            else
                file_write_format(FILE_STDOUT, "-");

            file_write_format(FILE_STDOUT, "rwxrwxrwx\t%s\n", content + start);

            start = i + 1;

        }

    }

}

void main(int argc, char *argv[])
{

    int fd = file_open("tty", "pwd");
    file_read(fd, 0x1000, buffer);
    file_close(fd);

    write_content(buffer);

}

