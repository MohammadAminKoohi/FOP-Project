#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#define INVALID puts("Invalid command");

void config(int, char *, char *);
int CheckInit(DIR *, char *);
void init();
int main(int argc, char *argv[])
{
    if (!strcmp(argv[1], "config"))
    {
        if (argc != 4 && argc != 5)
        {
            INVALID
            return 0;
        }
        if (!strcmp(argv[2], "--global"))
        {
            if (!strcmp(argv[3], "user.name"))
            {
                config(1, argv[3], argv[4]);
            }
            else if (!strcmp(argv[3], "user.email"))
            {
                config(1, argv[3], argv[4]);
            }
            else
            {
                INVALID
            }
        }
        else
        {
            if (!strcmp(argv[2], "user.name"))
            {
                config(0, argv[2], argv[3]);
            }
            else if (!strcmp(argv[2], "user.email"))
            {
                config(0, argv[2], argv[3]);
            }
            else
            {
                INVALID
            }
        }
    }
    if (!strcmp(argv[1], "init"))
    {
        init();
    }
    else
    {
        INVALID
    }
    return 0;
}

void config(int global, char *mode, char *str)
{
    if (global)
    {
        char path[] = "/home/aminkoohi/.magit";
        DIR *dir = opendir(path);
        if (dir == NULL)
        {
            mkdir(path, 0777);
            if (!strcmp(mode, "user.name"))
            {
                FILE *user = fopen("/home/aminkoohi/.magit/user.txt", "w");
                fprintf(user, "%s", str);
            }
            else
            {
                FILE *email = fopen("/home/aminkoohi/.magit/email.txt", "w");
                fprintf(email, "%s", str);
            }
        }
        else
        {
            if (!strcmp(mode, "user.name"))
            {
                FILE *user = fopen("/home/aminkoohi/.magit/user.txt", "w");
                fprintf(user, "user.name: %s\n", str);
            }
            else
            {
                FILE *email = fopen("/home/aminkoohi/.magit/email.txt", "w");
                fprintf(email, "user.email: %s\n", str);
            }
        }
        closedir(dir);
    }
    else
    {
    }
}
void init()
{
    char *cwd;
    char buffer[PATH_MAX];
    cwd = getcwd(buffer, PATH_MAX);
    CheckInit(cwd);
}
int CheckInit(char *path)
{
    DIR *dir = opendir(path);
    while ((FILE *fp = readdir(dir)) != NULL)
    {
        if (!strcmp(readdir(dir)->d_name, ".magit"))
        {
            puts("Already a magit repository");
            return 0;
        }
    }
}