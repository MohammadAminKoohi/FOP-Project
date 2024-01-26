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
char *CheckInit(char *);
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
                fclose(user);
            }
            else
            {
                FILE *email = fopen("/home/aminkoohi/.magitconfig/email.txt", "w");
                fprintf(email, "%s", str);
                fclose(email);
            }
        }
        else
        {
            if (!strcmp(mode, "user.name"))
            {
                FILE *user = fopen("/home/aminkoohi/.magitconfig/user.txt", "w");
                fprintf(user, "%s", str);
                fclose(user);
            }
            else
            {
                FILE *email = fopen("/home/aminkoohi/.magitconfig/email.txt", "w");
                fprintf(email, "%s", str);
                fclose(email);
            }
        }
        closedir(dir);
        return;
    }
    else
    {
        char *cwd;
        char buffer[PATH_MAX];
        cwd = getcwd(buffer, PATH_MAX);
        char *path = malloc(PATH_MAX);
        path = CheckInit(cwd);
        if (path != NULL)
        {
            strcat(path, "/.magit");
            if (!strcmp(mode, "user.name"))
            {
                strcat(path, "/user.txt");
                FILE *config = fopen(path, "w");
                fprintf(config, "%s", str);
                fclose(config);
            }
            else
            {
                strcat(path, "/email.txt");
                FILE *config = fopen(path, "w");
                fprintf(config, "%s", str);
                fclose(config);
            }
        }
        else
        {
            puts("This is not a magit repository");
        }
        return;
    }
}
void init()
{
    char *cwd;
    char buffer[PATH_MAX];
    cwd = getcwd(buffer, PATH_MAX);
    if (CheckInit(cwd) != NULL)
    {
        puts("This is already a magit repository");
        return;
    }
    else
    {
        mkdir(".magit", 0777);
        mkdir(".magit/branch", 0777);
        mkdir(".magit/branch/master", 0777);
        mkdir(".magit/commits", 0777);
        return;
    }
}
char *CheckInit(char *path)
{
    DIR *dir = opendir(path);
    struct dirent *fp;
    while ((fp = readdir(dir)) != NULL)
    {
        if (!strcmp(fp->d_name, ".magit"))
        {
            closedir(dir);
            return path;
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
        return NULL;
    }
}
void GetParent(char *path)
{
    char *loc = strrchr(path, '/');
    *loc = '\0';
    return;
}
