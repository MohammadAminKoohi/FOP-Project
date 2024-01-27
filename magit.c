#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#define INVALID puts("Invalid command");
#define deb puts("debug")
#define Dir 0
#define File 1
#define BUFFER_SIZE 1024

void config(int, char *, char *);
char *CheckInit(char *);
void init();
void GetParent(char *);
char *FindPath(char *);
void add(int, char *);
void StageFolder(char *);
int copyFile(char *, char *);

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
    else if (!strcmp(argv[1], "init"))
    {
        if (argc != 2)
        {
            INVALID
            return 0;
        }
        init();
        return 0;
    }
    else
    {
        char *cwd;
        char buffer[PATH_MAX];
        cwd = getcwd(buffer, PATH_MAX);
        char *repo = CheckInit(cwd);
        if (repo == NULL)
        {
            puts("This is not a magit repository");
            return 0;
        }
        else if (!strcmp(argv[1], "add"))
        {
            if (strcmp(repo, cwd))
            {
                char *path = malloc(PATH_MAX);
                path = FindPath(argv[2]);
                StageFolder(path);
            }
            struct stat path_stat;
            stat(argv[2], &path_stat);
            if (S_ISREG(path_stat.st_mode))
            {
                add(File, argv[2]);
            }
            else if (S_ISDIR(path_stat.st_mode))
            {
                add(Dir, argv[2]);
            }
            else
            {
                puts("Invalid File or Directory");
            }
        }
    }
    return 0;
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
    char *parent = malloc(strlen(path) + 1);
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
int copyFile(char *sourcePath, char *destinationPath)
{
    destinationPath = destinationPath + 1;
    FILE *sourceFile = fopen(sourcePath, "rb");
    if (sourceFile == NULL)
    {
        printf("File does not exist\n");
        return -1;
    }
    chdir("/");
    chdir(destinationPath);
    FILE *destinationFile = fopen(destinationPath, "wb");
    char buffer[BUFFER_SIZE];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, sourceFile)) > 0)
    {
        size_t bytesWritten = fwrite(buffer, 1, bytesRead, destinationFile);
    }
    fclose(sourceFile);
    return 0;
}
char *FindPath(char *name)
{
    char *fcwd;
    char buffer[PATH_MAX];
    fcwd = getcwd(buffer, PATH_MAX);
    char *path = malloc(PATH_MAX);
    path = CheckInit(fcwd);
    fcwd = fcwd + strlen(path);
    sprintf(fcwd, "%s/%s", fcwd, name);
    return fcwd;
}
void StageFolder(char *path)
{
    char newpath[PATH_MAX];
    strcpy(newpath, path);
    char *cwd;
    char buffer[PATH_MAX];
    cwd = getcwd(buffer, PATH_MAX);
    char *repo = CheckInit(cwd);
    char *loc = strrchr(newpath, '/');
    *loc = '\0';
    char *token = strtok(newpath, "/");
    chdir("/");
    chdir(repo);
    chdir(".magit/stage");
    while (token != NULL)
    {
        printf("token: %s\n", token);
        mkdir(token, 0777);
        chdir(token);
        token = strtok(NULL, "/");
    }
    chdir(cwd);
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
        mkdir(".magit/stage", 0777);
        return;
    }
}
void add(int mode, char *str)
{
    char *cwd;
    char buffer[PATH_MAX];
    cwd = getcwd(buffer, PATH_MAX);
    char *repo = CheckInit(cwd);
    if (mode == Dir)
    {
        char *NewPath = malloc(PATH_MAX);
        NewPath = FindPath(str);
        if (NewPath[0] == '/')
        {
            NewPath = NewPath + 1;
        }
        printf("NewPath: %s\n", NewPath);
        printf("cwd: %s\n", cwd);
        printf("repo: %s\n", repo);
        printf("str: %s\n", str);
        chdir("/");
        chdir(repo);
        chdir(".magit/stage");
        mkdir(NewPath, 0777);
        chdir(cwd);
        struct dirent *fp;
        char *path = malloc(PATH_MAX);
        path = FindPath(str);
        DIR *dir = opendir(str);
        if (dir == NULL)
        {
            puts("Directory not found\n");
            return;
        }
        while ((fp = readdir(dir)) != NULL)
        {
            if (strcmp(fp->d_name, ".") == 0 || strcmp(fp->d_name, "..") == 0)
            {
                continue;
            }
            if (fp->d_type == DT_DIR)
            {
                char path[PATH_MAX];
                sprintf(path, "%s/%s", cwd, str);
                printf("path: %s\n", path);
                chdir("/");
                chdir(path);
                add(Dir, fp->d_name);
                chdir(cwd);
            }
            else if (fp->d_type == DT_REG)
            {
                char path[PATH_MAX];
                sprintf(path, "%s/%s", cwd, str);
                chdir("/");
                chdir(path);
                add(File, fp->d_name);
                chdir(cwd);
            }
            else
            {
                printf("%s Unknown type\n", fp->d_name);
            }
        }
    }
    else
    {
        char File_Name[PATH_MAX];
        sprintf(File_Name, "%s/%s", cwd, str);
        FILE *file = fopen(File_Name, "r");
        if (file == NULL)
        {
            puts("File not found\n");
            return;
        }
        char *path = malloc(PATH_MAX);
        path = FindPath(str);
        char *copydest = malloc(PATH_MAX);
        sprintf(copydest, "%s/.magit/stage%s", repo, path);
        copyFile(File_Name, copydest);
        return;
    }
}
