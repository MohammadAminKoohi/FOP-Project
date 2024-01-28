#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#define INVALID              \
    puts("Invalid command"); \
    return 0;
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
int FileDir(char *);
void addn(int);
void reset(int, char *);

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
            if (argc <= 2)
            {
                INVALID
            }
            if (!strcmp(argv[2], "-n"))
            {
                int a = atoi(argv[3]);
                addn(a);
            }
            else if (!strcmp(argv[2], "-f"))
            {
                for (int i = 3; i < argc; i++)
                {
                    if (strcmp(repo, cwd))
                    {
                        char *path = malloc(PATH_MAX);
                        path = FindPath(argv[2]);
                        StageFolder(path);
                    }
                    add(FileDir(argv[i]), argv[i]);
                }
                return 0;
            }
            else
            {
                for (int i = 2; i < argc; i++)
                {
                    if (strcmp(repo, cwd))
                    {
                        char *path = malloc(PATH_MAX);
                        path = FindPath(argv[2]);
                        StageFolder(path);
                    }
                    add(FileDir(argv[i]), argv[i]);
                }
                return 0;
            }
        }
        else if (!strcmp(argv[1], "reset"))
        {
            if (argc <= 2)
            {
                INVALID
            }
            if (!strcmp(argv[2], "-f"))
            {
                for (int i = 3; i < argc; i++)
                {
                    reset(FileDir(argv[i]), argv[i]);
                }
                return 0;
            }
            else
            {
                for (int i = 2; i < argc; i++)
                {
                    reset(FileDir(argv[i]), argv[i]);
                }
                return 0;
            }
        }
        else
        {
            INVALID
        }
    }
    return 0;
}

int CheckStage(char *path)
{
    char *status_path = malloc(PATH_MAX);
    char *cwd;
    char buffer[PATH_MAX];
    cwd = getcwd(buffer, PATH_MAX);
    char *repo = CheckInit(cwd);
    sprintf(status_path, "%s/.magit/status.txt", repo);
    FILE *status = fopen(status_path, "r");
    char *line = malloc(PATH_MAX);
    while (fgets(line, PATH_MAX, status) != NULL)
    {
        line[strlen(line) - 1] = '\0';
        if (!strcmp(line, path))
        {
            line[strlen(line) - 1] = '\n';
            return 1;
        }
    }
    return 0;
}
int FileDir(char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    if (S_ISREG(path_stat.st_mode))
    {
        return File;
    }
    else if (S_ISDIR(path_stat.st_mode))
    {
        return Dir;
    }
    else
    {
        return -1;
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
    // fix here
    size_t size;
    while ((size = fread(buffer, 1, BUFFER_SIZE, sourceFile)) > 0)
    {
        fwrite(buffer, 1, size, destinationFile);
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
        fopen(".magit/status.txt", "w");
        fopen(".magit/branch/master/reset.txt", "w");
        return;
    }
}
void add(int mode, char *str)
{
    if (mode == -1)
    {
        puts("Invalid File or Directory");
        return;
    }
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
            puts("File not found\n"); // check if staged
            return;
        }
        if (CheckStage(File_Name))
        {
            return;
        }
        char *path = malloc(PATH_MAX);
        path = FindPath(str);
        char *copydest = malloc(PATH_MAX);
        char stats_path[PATH_MAX];
        char abs_path[PATH_MAX];
        sprintf(abs_path, "%s/%s", cwd, str);
        sprintf(stats_path, "%s/.magit/status.txt", repo);
        FILE *status = fopen(stats_path, "a");
        fprintf(status, "%s\n", abs_path);
        sprintf(copydest, "%s/.magit/stage%s", repo, path);
        copyFile(File_Name, copydest);
        return;
    }
}
void addn(int num)
{
    if (num == 0)
    {
        return;
    }
    char *cwd;
    char buffer[PATH_MAX];
    cwd = getcwd(buffer, PATH_MAX);
    char *repo = CheckInit(cwd);
    struct dirent *fp;
    char *path = malloc(PATH_MAX);
    DIR *dir = opendir(cwd);
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
        if (FileDir(fp->d_name) == Dir)
        {
            chdir(fp->d_name);
            addn(num - 1);
            chdir(cwd);
        }
        else if (FileDir(fp->d_name) == File)
        {
            char *File_Path = malloc(PATH_MAX);
            sprintf(File_Path, "%s/%s", cwd, fp->d_name);
            if (CheckStage(File_Path))
            {
                printf("%s\tstaged\n", fp->d_name);
            }
            else
            {
                printf("%s\t unstaged\n", fp->d_name);
            }
        }
    }

    return;
}
void reset(int mode, char *str)
{
    char *cwd;
    char buffer[PATH_MAX];
    cwd = getcwd(buffer, PATH_MAX);
    char *repo = CheckInit(cwd);
    if (mode == Dir)
    {
        char *rm_path = malloc(PATH_MAX);
        sprintf(rm_path, "rm -r %s/.magit/stage/%s", repo, str);
        printf("%s\n", rm_path);
        system(rm_path);
        char *status_path = malloc(PATH_MAX);
        sprintf(status_path, "%s/.magit/status.txt", repo);
        FILE *status = fopen(status_path, "r");
        char *line = malloc(PATH_MAX);
        char file_txt[1000][4096];
        int i = 0;
        char *loc = malloc(PATH_MAX);
        sprintf(loc, "%s/%s", cwd, str);
        printf("%s\n", loc);
        while (fgets(line, PATH_MAX, status) != NULL)
        {
            line[strlen(line) - 1] = '\0';
            if (!strncmp(line, loc, strlen(loc)))
            {
                line[strlen(line) - 1] = '\n';
                continue;
            }
            line[strlen(line) - 1] = '\n';
            strcpy(file_txt[i], line);
            i++;
        }
        fclose(status);
        status = fopen(status_path, "w");
        for (int j = 0; j < i; j++)
        {
            fprintf(status, "%s", file_txt[j]);
        }
        fclose(status);
    }
    else
    {
        char *rm_path = malloc(PATH_MAX);
        char *loc = malloc(PATH_MAX);
        sprintf(rm_path, "%s/.magit/stage/%s", repo, str);
        sprintf(loc, "%s/%s", cwd, str);
        if (!CheckStage(loc))
        {
            printf("%s is not staged\n", str);
            return;
        }
        remove(rm_path);
        char *status_path = malloc(PATH_MAX);
        sprintf(status_path, "%s/.magit/status.txt", repo);
        FILE *status = fopen(status_path, "r");
        char *line = malloc(PATH_MAX);
        char file_txt[1000][4096];
        int i = 0;
        printf("status path: %s\n", status_path);
        while (fgets(line, PATH_MAX, status) != NULL)
        {
            line[strlen(line) - 1] = '\0';
            if (!strcmp(line, loc))
            {
                line[strlen(line) - 1] = '\n';
                continue;
            }
            line[strlen(line) - 1] = '\n';
            strcpy(file_txt[i], line);
            i++;
        }
        fclose(status);
        status = fopen(status_path, "w");
        for (int j = 0; j < i; j++)
        {
            fprintf(status, "%s", file_txt[j]);
        }
        fclose(status);
    }
}