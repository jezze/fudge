#include <fudge.h>

char buffer[0x4000];

void get_path(char *buffer)
{

    int fd = file_open("dev", "cwd");
    file_read(fd, 256, buffer);
    file_close(fd);

}

void write_content(char *path, char *content)
{

    char str[256];

    unsigned int start = 0;
    unsigned int i;

    for (i = 0; content[i] != '\0'; i++)
    {

        if (content[i] == '\n')
        {

            content[i] = '\0';

            memory_set(str, 0, 256);
            string_write_concat(str, path);
            string_write_concat(str, content + start);

//            struct file_info *info = 0;

//            file_info(str, info);

            if (1)
                file_write_format(FILE_STDOUT, "d");
            else
                file_write_format(FILE_STDOUT, "-");

            file_write_format(FILE_STDOUT, "rwxrwxrwx %d\t%s\n", 0, content + start);

            start = i + 1;

        }

    }

}

void main(int argc, char *argv[])
{

    char path[256];

    get_path(path);

    int fd = file_open("dev", "view");

    file_read(fd, 0x1000, buffer);

    write_content(path, buffer);

    file_close(fd);

    call_exit();

}

