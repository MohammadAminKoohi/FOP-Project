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
int CheckInit(char *);
void init();
void GetParent(char *);
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
        return 0;
    }
    if (!strcmp(argv[1], "init"))
    {
        init();
        return 0;
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
        char path[] = "/home/aminkoohi/.magitconfig";
        DIR *dir = opendir(path);
        if (dir == NULL)
        {
            mkdir(path, 0777);
            if (!strcmp(mode, "user.name"))
            {
                FILE *user = fopen("/home/aminkoohi/.magitconfig/user.txt", "w");
                fprintf(user, "%s", str);
            }
            else
            {
                FILE *email = fopen("/home/aminkoohi/.magitconfig/email.txt", "w");
                fprintf(email, "%s", str);
            }
        }
        else
        {
            if (!strcmp(mode, "user.name"))
            {
                FILE *user = fopen("/home/aminkoohi/.magitconfig/user.txt", "w");
                fprintf(user, "%s", str);
            }
            else
            {
                FILE *email = fopen("/home/aminkoohi/.magitconfig/email.txt", "w");
                fprintf(email, "%s", str);
            }
        }
        closedir(dir);
        return;
    }
    else
    {
        return;
    }
}
void init()
{
    char *cwd;
    char buffer[PATH_MAX];
    cwd = getcwd(buffer, PATH_MAX);
    if (CheckInit(cwd))
    {
        puts("This is already a magit repository");
        return;
    }
    else
    {
        mkdir(".magit", 0777);
        mkdir(".magit/branch", 0777);
        mkdir(".magit/branch/main", 0777);
        mkdir(".magit/commits", 0777);
        return;
    }
}
int CheckInit(char *path)
{
    DIR *dir = opendir(path);
    struct dirent *fp;
    while ((fp = readdir(dir)) != NULL)
    {
        if (!strcmp(fp->d_name, ".magit"))
        {
            printf("%s\n", path);
            closedir(dir);
            return 1;
        }
    }
    closedir(dir);
    char *parent = malloc(strlen(path));
    strcpy(parent, path);
    GetParent(parent);
    if (strcmp(parent, "/home"))
    {
        CheckInit(parent);
    }
    else
    {
        return 0;
    }
}
void GetParent(char *path)
{
    char *loc = strrchr(path, '/');
    *loc = '\0';
    return;
}