#include <call.h>
#include <file.h>
#include <memory.h>
#include <string.h>

void get_path(char *buffer, char *arg)
{

    int cwd = file_open("/dev/cwd");
    unsigned int count = file_read(cwd, 256, buffer);

    if (arg)
    {

        if (arg[0] == '/')
            string_copy(buffer, arg);
        else
            string_concat(buffer, arg);

    }

    if (buffer[string_length(buffer) - 1] != '/')
        string_concat(buffer, "/");

    file_close(cwd);

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
            string_concat(str, path);
            string_concat(str, content + start);

            struct file_info *info = 0;

            call_info(str, info);

            if (1)
                file_write_string(FILE_STDOUT, "d");
            else
                file_write_string(FILE_STDOUT, "-");

            file_write_string(FILE_STDOUT, "rwxrwxrwx ");
            file_write_dec(FILE_STDOUT, info->length);
            file_write_string(FILE_STDOUT, "\t");
            file_write_string(FILE_STDOUT, info->name);
            file_write_string(FILE_STDOUT, "\n");

            start = i + 1;

        }

    }

}

void main(int argc, char *argv[])
{

    char path[256];

    if (argc == 1)
        get_path(path, 0);
    else
        get_path(path, argv[1]);

    int fd = file_open(path);

    char content[512];

    file_read(fd, 512, content);

    write_content(path, content);

    file_close(fd);

}

