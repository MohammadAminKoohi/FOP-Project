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
void undo();
int CheckStage(char *);
void redo();
void commit(int, char **);
void logg(int, char **);
int filecounter(char *);
char *changedir(char *);
void printcwd();
void checkout(int, char **);
void branch(int, char **);
void clean();
void status(int, char **);
char *branchfinder(int);
int lastid();
void commitstatus(int, char **);
int comparefiles(char *, char *);
void alias(int, char **, int);
int aliasfind(int, char **);
void set(int, char **);
void replace(int, char **);
void del(int, char **);
char *shortcutfinder(char *);

typedef struct
{
    int id;
    char message[72];
    char branch[100];
    char author[100];
    char author_email[100];
    int prev;
    int filecount;
    struct tm *time;
} commit_info;

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
            else if (!strncmp(argv[3], "alias.", 6))
            {
                alias(argc, argv, 1);
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
            else if (!strncmp(argv[2], "alias.", 6))
            {
                alias(argc, argv, 0);
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
            char *addpath = malloc(PATH_MAX);
            sprintf(addpath, "%s/.magit/add.txt", repo);
            FILE *addfile = fopen(addpath, "a");
            if (argc <= 2)
            {
                INVALID
            }
            if (!strcmp(argv[2], "-redo"))
            {
                redo();
            }
            else if (!strcmp(argv[2], "-n"))
            {
                if (argc <= 3)
                {
                    INVALID
                }
                int a = atoi(argv[3]);
                addn(a);
            }
            else if (!strcmp(argv[2], "-f"))
            {
                if (argc <= 3)
                {
                    INVALID
                }

                for (int i = 3; i < argc; i++)
                {
                    char *abspath = malloc(PATH_MAX);
                    sprintf(abspath, "%s/%s", cwd, argv[i]);
                    if (CheckStage(abspath))
                    {
                        // delete from stage folder
                        char *path = malloc(PATH_MAX);
                        path = FindPath(argv[2]);
                        char *rm_path = malloc(PATH_MAX);
                        sprintf(rm_path, "rm -r %s/.magit/stage%s", repo, path);
                        system(rm_path);
                    }
                    char tmp[PATH_MAX];
                    strcpy(tmp, argv[i]);
                    char *x = changedir(argv[i]);
                    char *newcwd = malloc(PATH_MAX);
                    char buffer[PATH_MAX];
                    newcwd = getcwd(buffer, PATH_MAX);
                    if (strcmp(repo, newcwd))
                    {
                        char *path = malloc(PATH_MAX);
                        path = FindPath(argv[2]);
                        StageFolder(path);
                    }
                    add(FileDir(x), changedir(x));
                    fprintf(addfile, "%s ", tmp);
                    chdir(cwd);
                }
                fprintf(addfile, "\n");
                return 0;
            }
            else
            {
                for (int i = 2; i < argc; i++)
                {
                    char tmp[PATH_MAX];
                    strcpy(tmp, argv[i]);
                    char *abspath = malloc(PATH_MAX);
                    sprintf(abspath, "%s/%s", cwd, argv[i]);
                    if (CheckStage(abspath))
                    {
                        // delete from stage folder
                        char *path = malloc(PATH_MAX);
                        path = FindPath(argv[2]);
                        char *rm_path = malloc(PATH_MAX);
                        sprintf(rm_path, "rm -r %s/.magit/stage%s", repo, path);
                        system(rm_path);
                    }
                    char *x = changedir(argv[i]);
                    char *newcwd = malloc(PATH_MAX);
                    char buffer[PATH_MAX];
                    newcwd = getcwd(buffer, PATH_MAX);
                    if (strcmp(repo, newcwd))
                    {
                        char *path = malloc(PATH_MAX);
                        path = FindPath(argv[2]);
                        StageFolder(path);
                    }
                    fprintf(addfile, "%s ", tmp);
                    add(FileDir(x), x);
                    chdir(cwd);
                }
                fprintf(addfile, "\n");
                return 0;
            }
        }
        else if (!strcmp(argv[1], "reset"))
        {
            char *resetpath = malloc(PATH_MAX);
            sprintf(resetpath, "%s/.magit/reset.txt", repo);
            FILE *resetfile = fopen(resetpath, "a");
            if (argc <= 2)
            {
                INVALID
            }
            if (!strcmp(argv[2], "-undo"))
            {
                undo();
            }
            else if (!strcmp(argv[2], "-f"))
            {
                for (int i = 3; i < argc; i++)
                {
                    char tmp[PATH_MAX];
                    strcpy(tmp, argv[i]);
                    char *x = changedir(argv[i]);
                    char *newcwd = malloc(PATH_MAX);
                    char buffer[PATH_MAX];
                    newcwd = getcwd(buffer, PATH_MAX);
                    char *abspath = malloc(PATH_MAX);
                    sprintf(abspath, "%s/%s", newcwd, x);
                    if (!CheckStage(abspath))
                    {
                        printf("%s is not staged\n", x);
                        continue;
                    }
                    fprintf(resetfile, "%s ", tmp);
                    reset(FileDir(x), x);
                    chdir(cwd);
                }
                fprintf(resetfile, "\n");
                return 0;
            }
            else
            {
                for (int i = 2; i < argc; i++)
                {
                    char tmp[PATH_MAX];
                    strcpy(tmp, argv[i]);
                    char *x = changedir(argv[i]);
                    char *newcwd = malloc(PATH_MAX);
                    char buffer[PATH_MAX];
                    newcwd = getcwd(buffer, PATH_MAX);
                    char *abspath = malloc(PATH_MAX);
                    sprintf(abspath, "%s/%s", newcwd, x);
                    if (!CheckStage(abspath))
                    {
                        printf("%s is not staged\n", x);
                        continue;
                    }
                    fprintf(resetfile, "%s ", tmp);
                    reset(FileDir(x), x);
                    chdir(cwd);
                }
                fprintf(resetfile, "\n");
                return 0;
            }
        }
        else if (!strcmp(argv[1], "commit"))
        {
            if (!strcmp(argv[2], "-s"))
            {
                char *message = malloc(PATH_MAX);
                message = shortcutfinder(argv[3]);
                if (message == NULL)
                {
                    puts("shortcut doesn't exist!");
                    return 0;
                }
                char *commit_command = malloc(PATH_MAX);
                sprintf(commit_command, "magit commit -m \"%s\"", message);
                system(commit_command);
            }
            else if (!strcmp(argv[2], "-m"))
            {
                if (argc <= 3)
                {
                    INVALID
                }
                if (strlen(argv[3]) > 72)
                {
                    puts("commit message must be less than 72 characters");
                    return 0;
                }
                commit(argc, argv);
            }
            else
            {
                INVALID
            }
        }
        else if (!strcmp(argv[1], "set"))
        {
            if (argc != 6 || strcmp(argv[2], "-m") || strcmp(argv[4], "-s"))
            {
                INVALID
            }
            set(argc, argv);
        }
        else if (!strcmp(argv[1], "replace"))
        {
            if (argc != 6 || strcmp(argv[2], "-m") || strcmp(argv[4], "-s"))
            {
                INVALID
            }
            replace(argc, argv);
        }
        else if (!strcmp(argv[1], "remove"))
        {
            if (argc != 4 || strcmp(argv[2], "-s"))
            {
                INVALID
            }
            del(argc, argv);
        }
        else if (!strcmp(argv[1], "log"))
        {
            logg(argc, argv);
        }
        else if (!strcmp(argv[1], "checkout"))
        {
            checkout(argc, argv);
        }
        else if (!strcmp(argv[1], "branch"))
        {
            branch(argc, argv);
        }
        else if (!strcmp(argv[1], "status"))
        {
            status(argc, argv);
            commitstatus(argc, argv);
        }
        else
        {
            int check = aliasfind(argc, argv);
            if (check == 0)
            {
                INVALID;
            }
        }
    }
    return 0;
}

void clean()
{
    // clear repo except .magit
    char *cwd;
    char buffer[PATH_MAX];
    cwd = getcwd(buffer, PATH_MAX);
    char *repo = CheckInit(cwd);
    DIR *dir = opendir(repo);
    struct dirent *fp;
    while ((fp = readdir(dir)) != NULL)
    {
        if (strcmp(fp->d_name, ".magit") == 0 || strcmp(fp->d_name, ".") == 0 || strcmp(fp->d_name, "..") == 0)
        {
            continue;
        }
        if (fp->d_type == DT_DIR)
        {
            char *path = malloc(PATH_MAX);
            sprintf(path, "%s/%s", repo, fp->d_name);
            char *command = malloc(PATH_MAX);
            sprintf(command, "rm -r %s", path);
            system(command);
        }
        else if (fp->d_type == DT_REG)
        {
            char *path = malloc(PATH_MAX);
            sprintf(path, "%s/%s", repo, fp->d_name);
            char *command = malloc(PATH_MAX);
            sprintf(command, "rm %s", path);
            system(command);
        }
    }
    return;
}
void printcwd()
{
    char *cwd;
    char buffer[PATH_MAX];
    cwd = getcwd(buffer, PATH_MAX);
    printf("%s\n", cwd);
    return;
}
char *changedir(char *path)
{
    // take a directory and change directory to parent of it and tokenize the last part of path and return it
    char *loc = strrchr(path, '/');
    if (loc == NULL)
    {
        return path;
    }
    *loc = '\0';
    char *token = strtok(path, "/");
    while (token != NULL)
    {
        chdir(token);
        token = strtok(NULL, "/");
    }
    return loc + 1;
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
    char *folder_path = malloc(PATH_MAX);
    char *tmp = malloc(PATH_MAX);
    strcpy(tmp, path);
    tmp = tmp + strlen(repo);
    sprintf(folder_path, "%s/.magit/stage%s", repo, tmp);
    DIR *dir = opendir(folder_path);
    while (fgets(line, PATH_MAX, status) != NULL)
    {
        line[strlen(line) - 1] = '\0';
        if (!strncmp(line, path, strlen(path)))
        {
            line[strlen(line)] = '\n';
            return 1;
        }
    }
    if (dir != NULL)
    {
        return 1;
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
    FILE *destinationFile = fopen(destinationPath, "r");
    if (destinationFile != NULL)
    {
        // delete the file
        char *rm_path = malloc(PATH_MAX);
        sprintf(rm_path, "rm %s", destinationPath);
        system(rm_path);
    }
    destinationFile = fopen(destinationPath, "wb");
    char buffer[BUFFER_SIZE];
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
    char *fcwd = malloc(PATH_MAX);
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
        puts("repository reinitialized");
        system("rm -r .magit");
    }
    mkdir(".magit", 0777);
    mkdir(".magit/branch", 0777);
    mkdir(".magit/branch/master", 0777);
    mkdir(".magit/commits", 0777);
    mkdir(".magit/stage", 0777);
    mkdir(".magit/shortcuts", 0777);
    fopen(".magit/status.txt", "w");
    fopen(".magit/reset.txt", "w");
    fopen(".magit/add.txt", "w");
    FILE *branch = fopen(".magit/commits/branch.txt", "w");
    FILE *previd = fopen(".magit/commits/previd.txt", "w");
    FILE *last_id = fopen(".magit/commits/last_id.txt", "w");
    FILE *log = fopen(".magit/commits/log.txt", "w");
    FILE *prevbranch = fopen(".magit/commits/prevbranch.txt", "w");
    FILE *list = fopen(".magit/branch/master/list.txt", "w");
    fprintf(branch, "master");
    fprintf(prevbranch, "master");
    fprintf(previd, "0");
    fprintf(last_id, "-1");
    return;
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
    char *stats_path = malloc(PATH_MAX);
    sprintf(stats_path, "%s/.magit/status.txt", repo);
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
        char abs_path[PATH_MAX];
        sprintf(abs_path, "%s/%s", cwd, str);
        FILE *status = fopen(stats_path, "a");
        fprintf(status, "%s\n", abs_path);
        fclose(status);
        return;
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
        char *path = malloc(PATH_MAX);
        path = FindPath(str);
        char *copydest = malloc(PATH_MAX);
        char stats_path[PATH_MAX];
        char abs_path[PATH_MAX];
        sprintf(abs_path, "%s/%s", cwd, str);
        sprintf(stats_path, "%s/.magit/status.txt", repo);
        FILE *stats = fopen(stats_path, "a");
        fprintf(stats, "%s\n", abs_path);
        sprintf(copydest, "%s/.magit/stage%s", repo, path);
        copyFile(File_Name, copydest);
        fclose(file);
        fclose(stats);
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
        char *loc = malloc(PATH_MAX);
        char *purepath = malloc(PATH_MAX);
        sprintf(loc, "%s/%s", cwd, str);
        strcpy(purepath, loc);
        if (!CheckStage(loc))
        {
            printf("%s is not staged\n", str);
            return;
        }
        purepath = purepath + strlen(repo);
        sprintf(rm_path, "rm -r %s/.magit/stage%s", repo, purepath);
        system(rm_path);
        char *status_path = malloc(PATH_MAX);
        sprintf(status_path, "%s/.magit/status.txt", repo);
        FILE *status = fopen(status_path, "r");
        char *line = malloc(PATH_MAX);
        char file_txt[1000][4096];
        int i = 0;
        while (fgets(line, PATH_MAX, status) != NULL)
        {
            line[strlen(line) - 1] = '\0';
            if (!strncmp(line, loc, strlen(loc)))
            {
                line[strlen(line)] = '\n';
                continue;
            }
            line[strlen(line)] = '\n';
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
        char *purepath = malloc(PATH_MAX);
        sprintf(loc, "%s/%s", cwd, str);
        strcpy(purepath, loc);
        if (!CheckStage(loc))
        {
            printf("%s is not staged\n", str);
            return;
        }
        purepath = purepath + strlen(repo);
        sprintf(rm_path, "rm -r %s/.magit/stage/%s", repo, purepath);
        system(rm_path);
        char *status_path = malloc(PATH_MAX);
        sprintf(status_path, "%s/.magit/status.txt", repo);
        FILE *status = fopen(status_path, "r");
        char *line = malloc(PATH_MAX);
        char file_txt[1000][4096];
        int i = 0;
        while (fgets(line, PATH_MAX, status) != NULL)
        {
            line[strlen(line) - 1] = '\0';
            if (!strcmp(line, loc))
            {
                line[strlen(line)] = '\n';
                continue;
            }
            line[strlen(line)] = '\n';
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
void undo()
{
    char *addfilepath = malloc(PATH_MAX);
    char *cwd;
    char buffer[PATH_MAX];
    cwd = getcwd(buffer, PATH_MAX);
    char *repo = CheckInit(cwd);
    sprintf(addfilepath, "%s/.magit/add.txt", repo);
    FILE *addfile = fopen(addfilepath, "r");
    char *line = malloc(PATH_MAX);
    char file_txt[1000][4096];
    int i = 0;
    while (fgets(line, PATH_MAX, addfile) != NULL)
    {
        line[strlen(line) - 1] = '\0';
        strcpy(file_txt[i], line);
        i++;
    }
    fclose(addfile);
    addfile = fopen(addfilepath, "w");
    char *system_command = malloc(PATH_MAX + 20);
    sprintf(system_command, "magit reset %s", file_txt[i - 1]);
    system(system_command);
    for (int j = 0; j < i - 1; j++)
    {
        file_txt[j][strlen(file_txt[j])] = '\n';
        fprintf(addfile, "%s", file_txt[j]);
    }
    fclose(addfile);
}
void redo()
{
    char *resetfilepath = malloc(PATH_MAX);
    char *cwd;
    char buffer[PATH_MAX];
    cwd = getcwd(buffer, PATH_MAX);
    char *repo = CheckInit(cwd);
    sprintf(resetfilepath, "%s/.magit/reset.txt", repo);
    FILE *resetfile = fopen(resetfilepath, "r");
    char *line = malloc(PATH_MAX);
    char file_txt[1000][4096];
    int i = 0;
    while (fgets(line, PATH_MAX, resetfile) != NULL)
    {
        line[strlen(line) - 1] = '\0';
        strcpy(file_txt[i], line);
        i++;
    }
    for (int j = i - 1; j >= 0; j--)
    {
        char *system_command = malloc(PATH_MAX + 20);
        sprintf(system_command, "magit add %s", file_txt[j]);
        system(system_command);
    }
    fclose(resetfile);
    resetfile = fopen(resetfilepath, "w");
    fclose(resetfile);
    return;
}
int filecounter(char *path)
{
    // count the files in the path recursively
    int count = 0;
    struct dirent *fp;
    DIR *dir = opendir(path);
    while ((fp = readdir(dir)) != NULL)
    {
        if (strcmp(fp->d_name, ".") == 0 || strcmp(fp->d_name, "..") == 0 || strcmp(fp->d_name, ".magit") == 0)
        {
            continue;
        }
        if (fp->d_type == DT_DIR)
        {
            char *path2 = malloc(PATH_MAX);
            sprintf(path2, "%s/%s", path, fp->d_name);
            count += filecounter(path2);
        }
        else if (fp->d_type == DT_REG)
        {
            count++;
        }
    }
    return count;
}
void commit(int argc, char **argv)
{
    char *cwd;
    char buffer[PATH_MAX];
    cwd = getcwd(buffer, PATH_MAX);
    char *repo = CheckInit(cwd);
    chdir(repo);
    chdir(".magit");
    char user[] = "user.txt";
    char email[] = "email.txt";
    FILE *user_file = fopen(user, "r");
    FILE *email_file = fopen(email, "r");
    if (user_file == NULL || email_file == NULL)
    {
        deb;
        // check global config
        chdir("/");
        chdir("/home/aminkoohi/.magitconfig");
        user_file = fopen(user, "r");
        email_file = fopen(email, "r");
        chdir(cwd);
        if (user_file == NULL || email_file == NULL)
        {
            puts("you must set user name and email before committing");
            return;
        }
    }
    chdir(cwd);
    char *user_name = malloc(PATH_MAX);
    char *user_email = malloc(PATH_MAX);
    fgets(user_name, PATH_MAX, user_file);
    fgets(user_email, PATH_MAX, email_file);
    user_name[strlen(user_name)] = '\0';
    user_email[strlen(user_email)] = '\0';
    fclose(user_file);
    fclose(email_file);

    // branch name get
    char *branch_path = malloc(PATH_MAX);
    sprintf(branch_path, "%s/.magit/commits/branch.txt", repo);
    FILE *branch_file = fopen(branch_path, "r");
    char *branch_name = malloc(PATH_MAX);
    fgets(branch_name, PATH_MAX, branch_file);
    branch_name[strlen(branch_name)] = '\0';
    fclose(branch_file);

    // commit id get
    char *id_path = malloc(PATH_MAX);
    sprintf(id_path, "%s/.magit/commits/last_id.txt", repo);
    FILE *id_file = fopen(id_path, "r");
    char *id = malloc(PATH_MAX);
    fgets(id, PATH_MAX, id_file);
    id[strlen(id)] = '\0';

    // commit prev get
    char *prev_path = malloc(PATH_MAX);
    sprintf(prev_path, "%s/.magit/commits/previd.txt", repo);
    FILE *prev_file = fopen(prev_path, "r");
    char *prev = malloc(PATH_MAX);
    fgets(prev, PATH_MAX, prev_file);
    prev[strlen(prev)] = '\0';

    // file count get
    char *stage_path = malloc(PATH_MAX);
    sprintf(stage_path, "%s/.magit/stage", repo);
    int file_count = filecounter(stage_path);
    if (file_count == 0)
    {
        puts("nothing to commit");
        return;
    }

    commit_info *comm = malloc(sizeof(commit_info));
    comm->id = atoi(id) + 1;
    comm->prev = atoi(prev);
    comm->filecount = file_count;
    strcpy(comm->branch, branch_name);
    strcpy(comm->message, argv[3]);
    strcpy(comm->author, user_name);
    strcpy(comm->author_email, user_email);
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    comm->time = &tm;
    printf("[%d %s] %s %d/%d/%d %d:%d:%d\n", comm->id, comm->message, comm->branch, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    char *comm_path = malloc(PATH_MAX);
    sprintf(comm_path, "%s/.magit/branch/%s/%d", repo, comm->branch, comm->id);
    mkdir(comm_path, 0777);
    char *comm_info_path = malloc(PATH_MAX);

    // copy all from stage to comm_path
    if (comm->id == 0)
    {
        char *stage_path = malloc(PATH_MAX);
        sprintf(stage_path, "%s/.magit/stage", repo);
        char *cp_commad = malloc(PATH_MAX);
        DIR *dir = opendir(stage_path);
        struct dirent *fp;
        dir = opendir(stage_path);
        while ((fp = readdir(dir)) != NULL)
        {
            if (strcmp(fp->d_name, ".") == 0 || strcmp(fp->d_name, "..") == 0 || strcmp(fp->d_name, ".magit") == 0)
            {
                continue;
            }
            if (fp->d_type == DT_DIR)
            {
                char *path = malloc(PATH_MAX);
                sprintf(path, "%s/%s", stage_path, fp->d_name);
                char *cp_commad = malloc(PATH_MAX);
                sprintf(cp_commad, "cp -r %s %s", path, comm_path);
                system(cp_commad);
            }
            else if (fp->d_type == DT_REG)
            {
                char *path = malloc(PATH_MAX);
                sprintf(path, "%s/%s", stage_path, fp->d_name);
                char *cp_commad = malloc(PATH_MAX);
                sprintf(cp_commad, "cp %s %s", path, comm_path);
                system(cp_commad);
            }
        }
    }
    else
    {
        // check if list.txt has any commit inside if not read prev commit from head.txt
        char *list_path = malloc(PATH_MAX);
        sprintf(list_path, "%s/.magit/branch/%s/list.txt", repo, comm->branch);
        FILE *list_file = fopen(list_path, "r");
        char *line = malloc(PATH_MAX);
        char *prev_branch = malloc(PATH_MAX);
        int i = 0;
        while (fgets(line, PATH_MAX, list_file) != NULL)
        {
            i++;
        }
        fclose(list_file);
        char *pre_branch_path = malloc(PATH_MAX);
        int prev_id;
        if (i == 0)
        {
            sprintf(pre_branch_path, "%s/.magit/branch/%s/head.txt", repo, comm->branch);
            FILE *pre_branch_file = fopen(pre_branch_path, "r");
            char *line = malloc(PATH_MAX);
            fgets(line, PATH_MAX, pre_branch_file);
            prev_id = atoi(line);
            fgets(line, PATH_MAX, pre_branch_file);
            strcpy(prev_branch, line);
            fclose(pre_branch_file);
        }
        else
        {
            prev_id = atoi(line);
            strcpy(prev_branch, comm->branch);
        }

        char *prev_comm_path = malloc(PATH_MAX);
        sprintf(prev_comm_path, "%s/.magit/branch/%s/%d", repo, prev_branch, prev_id);
        char *cp_commad = malloc(PATH_MAX);
        // copy all from prev_comm_path to comm_path
        DIR *dir = opendir(prev_comm_path);
        struct dirent *fp;
        dir = opendir(prev_comm_path);
        while ((fp = readdir(dir)) != NULL)
        {
            if (strcmp(fp->d_name, ".") == 0 || strcmp(fp->d_name, "..") == 0 || strcmp(fp->d_name, ".magit") == 0)
            {
                continue;
            }
            if (fp->d_type == DT_DIR)
            {
                char *path = malloc(PATH_MAX);
                sprintf(path, "%s/%s", prev_comm_path, fp->d_name);
                char *cp_commad = malloc(PATH_MAX);
                sprintf(cp_commad, "cp -r %s %s", path, comm_path);
                system(cp_commad);
            }
            else if (fp->d_type == DT_REG)
            {
                char *path = malloc(PATH_MAX);
                sprintf(path, "%s/%s", prev_comm_path, fp->d_name);
                char *cp_commad = malloc(PATH_MAX);
                sprintf(cp_commad, "cp %s %s", path, comm_path);
                system(cp_commad);
            }
        }
        // copy all from stage to comm_path
        char *stage_path = malloc(PATH_MAX);
        sprintf(stage_path, "%s/.magit/stage", repo);
        char *cp_commad2 = malloc(PATH_MAX);
        DIR *dir2 = opendir(stage_path);
        struct dirent *fp2;
        dir2 = opendir(stage_path);
        while ((fp2 = readdir(dir2)) != NULL)
        {
            if (strcmp(fp2->d_name, ".") == 0 || strcmp(fp2->d_name, "..") == 0 || strcmp(fp2->d_name, ".magit") == 0)
            {
                continue;
            }
            if (fp2->d_type == DT_DIR)
            {
                char *path = malloc(PATH_MAX);
                sprintf(path, "%s/%s", stage_path, fp2->d_name);
                char *cp_commad = malloc(PATH_MAX);
                sprintf(cp_commad, "cp -r %s %s", path, comm_path);
                system(cp_commad);
            }
            else if (fp2->d_type == DT_REG)
            {
                char *path = malloc(PATH_MAX);
                sprintf(path, "%s/%s", stage_path, fp2->d_name);
                char *cp_commad = malloc(PATH_MAX);
                sprintf(cp_commad, "cp %s %s", path, comm_path);
                system(cp_commad);
            }
        }
    }

    // update branch head
    char *list_path = malloc(PATH_MAX);
    sprintf(list_path, "%s/.magit/branch/%s/list.txt", repo, comm->branch);
    FILE *list_file = fopen(list_path, "a");
    fprintf(list_file, "%d\n", comm->id);
    fclose(list_file);

    // update text files like id last id and log
    char *log_path = malloc(PATH_MAX);
    sprintf(log_path, "%s/.magit/commits/log.txt", repo);
    FILE *log_file = fopen(log_path, "a");
    fprintf(log_file, "%s\n", comm->author);
    fprintf(log_file, "%d\n\"%s\"\n%s\n%d/%d/%d %d:%d:%d\n%d\n", comm->id, comm->message, comm->branch, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, comm->filecount);
    fclose(log_file);
    FILE *prev_file2 = fopen(prev_path, "w");
    fprintf(prev_file2, "%d", comm->id);
    fclose(prev_file2);
    id_file = fopen(id_path, "w");
    fprintf(id_file, "%d", comm->id);
    fclose(id_file);
    prev_file = fopen(prev_path, "w");
    fprintf(prev_file, "%d", comm->id);
    fclose(prev_file);

    // commit info creation
    sprintf(comm_info_path, "%s/.magit/branch/%s/info%d.txt", repo, comm->branch, comm->id);
    FILE *comm_info_file = fopen(comm_info_path, "w");
    fprintf(comm_info_file, "%s\n", comm->author);
    fprintf(comm_info_file, "%d\n%s\n%s\n%d-%d-%d %d:%d:%d\n%d\n", comm->id, comm->message, comm->branch, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, comm->filecount);

    chdir(repo);
    chdir(".magit");
    system("rm -r stage");
    mkdir("stage", 0777);
    system("rm status.txt && touch status.txt");
    system("rm add.txt && touch add.txt");
    system("rm reset.txt && touch reset.txt");
    return;
}
void set(int argc, char **argv)
{
    char *cwd = malloc(PATH_MAX);
    cwd = getcwd(cwd, PATH_MAX);
    char *repo = CheckInit(cwd);
    char *folder = malloc(PATH_MAX);
    sprintf(folder, "%s/.magit/shortcuts", repo);
    char *file_name = malloc(PATH_MAX);
    strcpy(file_name, argv[5]);
    char *file_path = malloc(PATH_MAX);
    sprintf(file_path, "%s/%s", folder, file_name);
    FILE *file = fopen(file_path, "w");
    fprintf(file, "%s", argv[3]);
    fclose(file);
    return;
}
void replace(int argc, char **argv)
{
    char *cwd = malloc(PATH_MAX);
    cwd = getcwd(cwd, PATH_MAX);
    char *repo = CheckInit(cwd);
    char *folder = malloc(PATH_MAX);
    sprintf(folder, "%s/.magit/shortcuts", repo);
    char *file_name = malloc(PATH_MAX);
    strcpy(file_name, argv[5]);
    char *file_path = malloc(PATH_MAX);
    sprintf(file_path, "%s/%s", folder, file_name);
    FILE *file = fopen(file_path, "r");
    if (file == NULL)
    {
        puts("You can't replace a shortcut that doesn't exist!");
        return;
    }
    file = fopen(file_path, "w");
    fprintf(file, "%s", argv[3]);
    fclose(file);
    return;
}
void del(int argc, char **argv)
{
    char *cwd = malloc(PATH_MAX);
    cwd = getcwd(cwd, PATH_MAX);
    char *repo = CheckInit(cwd);
    char *folder = malloc(PATH_MAX);
    sprintf(folder, "%s/.magit/shortcuts", repo);
    char *file_name = malloc(PATH_MAX);
    strcpy(file_name, argv[3]);
    char *file_path = malloc(PATH_MAX);
    sprintf(file_path, "%s/%s", folder, file_name);
    FILE *file = fopen(file_path, "r");
    if (file == NULL)
    {
        puts("You can't delete a shortcut that doesn't exist!");
        return;
    }
    fclose(file);
    char *rm_command = malloc(PATH_MAX);
    sprintf(rm_command, "rm %s", file_path);
    system(rm_command);
    return;
}
char *shortcutfinder(char *name)
{
    char *cwd = malloc(PATH_MAX);
    cwd = getcwd(cwd, PATH_MAX);
    char *repo = CheckInit(cwd);
    char *folder = malloc(PATH_MAX);
    sprintf(folder, "%s/.magit/shortcuts", repo);
    char *file_name = malloc(PATH_MAX);
    strcpy(file_name, name);
    char *file_path = malloc(PATH_MAX);
    sprintf(file_path, "%s/%s", folder, file_name);
    FILE *file = fopen(file_path, "r");
    if (file == NULL)
    {
        puts("this shortcut doesn't exist!");
        return NULL;
    }
    char *line = malloc(PATH_MAX);
    fgets(line, PATH_MAX, file);
    fclose(file);
    return line;
}
void logg(int argc, char **argv)
{
    char *cwd;
    char buffer[PATH_MAX];
    cwd = getcwd(buffer, PATH_MAX);
    char *repo = CheckInit(cwd);
    char *log_path = malloc(PATH_MAX);
    sprintf(log_path, "%s/.magit/commits/log.txt", repo);
    if (argc == 2)
    {
        char log[1000][4096];
        FILE *log_file = fopen(log_path, "r");
        char *line = malloc(PATH_MAX);
        int i = 0;
        while (fgets(line, PATH_MAX, log_file) != NULL)
        {
            strcpy(log[i], line);
            i++;
        }
        commit_info comm[i];
        int k = 0;
        for (int j = 0; j < i; j++)
        {
            if (j % 6 == 0)
            {
                comm[k].time = malloc(sizeof(struct tm));
                strcpy(comm[k].author, log[j]);
                comm[k].id = atoi(log[j + 1]);
                strcpy(comm[k].message, log[j + 2]);
                strcpy(comm[k].branch, log[j + 3]);
                sscanf(log[j + 4], "%d/%d/%d %d:%d:%d", &comm[k].time->tm_year, &comm[k].time->tm_mon, &comm[k].time->tm_mday, &comm[k].time->tm_hour, &comm[k].time->tm_min, &comm[k].time->tm_sec);
                comm[k].filecount = atoi(log[j + 5]);
                k++;
            }
        }
        for (int j = k - 1; j >= 0; j--)
        {
            printf("author: %s", comm[j].author);
            printf("commit id: %d\n", comm[j].id);
            printf("message: %s", comm[j].message);
            printf("branch: %s", comm[j].branch);
            printf("time: %d/%d/%d %d:%d:%d\n", comm[j].time->tm_year, comm[j].time->tm_mon, comm[j].time->tm_mday, comm[j].time->tm_hour, comm[j].time->tm_min, comm[j].time->tm_sec);
            printf("file count: %d\n", comm[j].filecount);
            printf("\n");
        }
    }
    else if (argc == 4 && !strcmp(argv[2], "-n"))
    {
        char log[1000][4096];
        FILE *log_file = fopen(log_path, "r");
        char *line = malloc(PATH_MAX);
        int i = 0;
        int n = atoi(argv[3]);
        while (fgets(line, PATH_MAX, log_file) != NULL)
        {
            strcpy(log[i], line);
            i++;
        }
        commit_info comm[i];
        int k = 0;
        for (int j = 0; j < i; j++)
        {
            if (j % 6 == 0)
            {
                comm[k].time = malloc(sizeof(struct tm));
                strcpy(comm[k].author, log[j]);
                comm[k].id = atoi(log[j + 1]);
                strcpy(comm[k].message, log[j + 2]);
                strcpy(comm[k].branch, log[j + 3]);
                sscanf(log[j + 4], "%d/%d/%d %d:%d:%d", &comm[k].time->tm_year, &comm[k].time->tm_mon, &comm[k].time->tm_mday, &comm[k].time->tm_hour, &comm[k].time->tm_min, &comm[k].time->tm_sec);
                comm[k].filecount = atoi(log[j + 5]);
                k++;
            }
        }
        if (n > k)
        {
            n = k;
        }
        for (int j = k - 1; j >= k - n; j--)
        {
            printf("author: %s", comm[j].author);
            printf("commit id: %d\n", comm[j].id);
            printf("message: %s", comm[j].message);
            printf("branch: %s\n", comm[j].branch);
            printf("time: %d/%d/%d %d:%d:%d\n", comm[j].time->tm_year, comm[j].time->tm_mon, comm[j].time->tm_mday, comm[j].time->tm_hour, comm[j].time->tm_min, comm[j].time->tm_sec);
            printf("file count: %d\n", comm[j].filecount);
            printf("\n");
        }
    }
    else if (argc == 4 && !strcmp(argv[2], "-branch"))
    {
        char log[1000][4096];
        FILE *log_file = fopen(log_path, "r");
        char *line = malloc(PATH_MAX);
        int i = 0;
        while (fgets(line, PATH_MAX, log_file) != NULL)
        {
            strcpy(log[i], line);
            i++;
        }
        commit_info comm[i];
        int k = 0;
        for (int j = 0; j < i; j++)
        {
            if (j % 6 == 0)
            {
                comm[k].time = malloc(sizeof(struct tm));
                strcpy(comm[k].author, log[j]);
                comm[k].id = atoi(log[j + 1]);
                strcpy(comm[k].message, log[j + 2]);
                strcpy(comm[k].branch, log[j + 3]);
                sscanf(log[j + 4], "%d/%d/%d %d:%d:%d", &comm[k].time->tm_year, &comm[k].time->tm_mon, &comm[k].time->tm_mday, &comm[k].time->tm_hour, &comm[k].time->tm_min, &comm[k].time->tm_sec);
                comm[k].filecount = atoi(log[j + 5]);
                k++;
            }
        }
        for (int j = k - 1; j >= 0; j--)
        {
            if (!strncmp(comm[j].branch, argv[3], strlen(argv[3])))
            {
                printf("author: %s", comm[j].author);
                printf("commit id: %d\n", comm[j].id);
                printf("message: %s", comm[j].message);
                printf("branch: %s", comm[j].branch);
                printf("time: %d/%d/%d %d:%d:%d\n", comm[j].time->tm_year, comm[j].time->tm_mon, comm[j].time->tm_mday, comm[j].time->tm_hour, comm[j].time->tm_min, comm[j].time->tm_sec);
                printf("file count: %d\n", comm[j].filecount);
                printf("\n");
            }
        }
    }
    else if (argc == 4 && !strcmp(argv[2], "-author"))
    {
        char log[1000][4096];
        FILE *log_file = fopen(log_path, "r");
        char *line = malloc(PATH_MAX);
        int i = 0;
        while (fgets(line, PATH_MAX, log_file) != NULL)
        {
            strcpy(log[i], line);
            i++;
        }
        commit_info comm[i];
        int k = 0;
        for (int j = 0; j < i; j++)
        {
            if (j % 6 == 0)
            {
                comm[k].time = malloc(sizeof(struct tm));
                strcpy(comm[k].author, log[j]);
                comm[k].id = atoi(log[j + 1]);
                strcpy(comm[k].message, log[j + 2]);
                strcpy(comm[k].branch, log[j + 3]);
                sscanf(log[j + 4], "%d/%d/%d %d:%d:%d", &comm[k].time->tm_year, &comm[k].time->tm_mon, &comm[k].time->tm_mday, &comm[k].time->tm_hour, &comm[k].time->tm_min, &comm[k].time->tm_sec);
                comm[k].filecount = atoi(log[j + 5]);
                k++;
            }
        }
        for (int j = k - 1; j >= 0; j--)
        {
            // fix later
            if (!strncmp(comm[j].author, argv[3], strlen(argv[3])))
            {
                printf("author: %s", comm[j].author);
                printf("commit id: %d\n", comm[j].id);
                printf("message: %s", comm[j].message);
                printf("branch: %s", comm[j].branch);
                printf("time: %d/%d/%d %d:%d:%d\n", comm[j].time->tm_year, comm[j].time->tm_mon, comm[j].time->tm_mday, comm[j].time->tm_hour, comm[j].time->tm_min, comm[j].time->tm_sec);
                printf("file count: %d\n", comm[j].filecount);
                printf("\n");
            }
        }
    }
    else if (argc == 5 && !strcmp(argv[2], "-since"))
    {
        struct tm tm;
        sscanf(argv[3], "%d/%d/%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
        sscanf(argv[4], "%d:%d", &tm.tm_hour, &tm.tm_min);
        char log[1000][4096];
        FILE *log_file = fopen(log_path, "r");
        char *line = malloc(PATH_MAX);
        int i = 0;
        int n = atoi(argv[3]);
        while (fgets(line, PATH_MAX, log_file) != NULL)
        {
            strcpy(log[i], line);
            i++;
        }
        commit_info comm[i];
        int k = 0;
        for (int j = 0; j < i; j++)
        {
            if (j % 6 == 0)
            {
                comm[k].time = malloc(sizeof(struct tm));
                strcpy(comm[k].author, log[j]);
                comm[k].id = atoi(log[j + 1]);
                strcpy(comm[k].message, log[j + 2]);
                strcpy(comm[k].branch, log[j + 3]);
                sscanf(log[j + 4], "%d/%d/%d %d:%d:%d", &comm[k].time->tm_year, &comm[k].time->tm_mon, &comm[k].time->tm_mday, &comm[k].time->tm_hour, &comm[k].time->tm_min, &comm[k].time->tm_sec);
                comm[k].filecount = atoi(log[j + 5]);
                k++;
            }
        }
        if (n > k)
        {
            n = k;
        }
        for (int j = k - 1; j >= k - n; j--)
        {
            if (comm[j].time->tm_year > tm.tm_year)
            {
                printf("author: %s", comm[j].author);
                printf("commit id: %d\n", comm[j].id);
                printf("message: %s", comm[j].message);
                printf("branch: %s", comm[j].branch);
                printf("time: %d/%d/%d %d:%d:%d\n", comm[j].time->tm_year, comm[j].time->tm_mon, comm[j].time->tm_mday, comm[j].time->tm_hour, comm[j].time->tm_min, comm[j].time->tm_sec);
                printf("file count: %d\n", comm[j].filecount);
                printf("\n");
            }
            else if (comm[j].time->tm_year == tm.tm_year && comm[j].time->tm_mon > tm.tm_mon)
            {
                printf("author: %s", comm[j].author);
                printf("commit id: %d\n", comm[j].id);
                printf("message: %s", comm[j].message);
                printf("branch: %s", comm[j].branch);
                printf("time: %d/%d/%d %d:%d:%d\n", comm[j].time->tm_year, comm[j].time->tm_mon, comm[j].time->tm_mday, comm[j].time->tm_hour, comm[j].time->tm_min, comm[j].time->tm_sec);
                printf("file count: %d\n", comm[j].filecount);
                printf("\n");
            }
            else if (comm[j].time->tm_year == tm.tm_year && comm[j].time->tm_mon == tm.tm_mon && comm[j].time->tm_mday > tm.tm_mday)
            {
                printf("author: %s", comm[j].author);
                printf("commit id: %d\n", comm[j].id);
                printf("message: %s", comm[j].message);
                printf("branch: %s", comm[j].branch);
                printf("time: %d/%d/%d %d:%d:%d\n", comm[j].time->tm_year, comm[j].time->tm_mon, comm[j].time->tm_mday, comm[j].time->tm_hour, comm[j].time->tm_min, comm[j].time->tm_sec);
                printf("file count: %d\n", comm[j].filecount);
                printf("\n");
            }
            else if (comm[j].time->tm_year == tm.tm_year && comm[j].time->tm_mon == tm.tm_mon && comm[j].time->tm_mday == tm.tm_mday && comm[j].time->tm_hour > tm.tm_hour)
            {
                printf("author: %s", comm[j].author);
                printf("commit id: %d\n", comm[j].id);
                printf("message: %s", comm[j].message);
                printf("branch: %s", comm[j].branch);
                printf("time: %d/%d/%d %d:%d:%d\n", comm[j].time->tm_year, comm[j].time->tm_mon, comm[j].time->tm_mday, comm[j].time->tm_hour, comm[j].time->tm_min, comm[j].time->tm_sec);
                printf("file count: %d\n", comm[j].filecount);
                printf("\n");
            }
            else if (comm[j].time->tm_year == tm.tm_year && comm[j].time->tm_mon == tm.tm_mon && comm[j].time->tm_mday == tm.tm_mday && comm[j].time->tm_hour == tm.tm_hour && comm[j].time->tm_min > tm.tm_min)
            {
                printf("author: %s", comm[j].author);
                printf("commit id: %d\n", comm[j].id);
                printf("message: %s", comm[j].message);
                printf("branch: %s", comm[j].branch);
                printf("time: %d/%d/%d %d:%d:%d\n", comm[j].time->tm_year, comm[j].time->tm_mon, comm[j].time->tm_mday, comm[j].time->tm_hour, comm[j].time->tm_min, comm[j].time->tm_sec);
                printf("file count: %d\n", comm[j].filecount);
                printf("\n");
            }
            else if (comm[j].time->tm_year == tm.tm_year && comm[j].time->tm_mon == tm.tm_mon && comm[j].time->tm_mday == tm.tm_mday && comm[j].time->tm_hour == tm.tm_hour && comm[j].time->tm_min == tm.tm_min && comm[j].time->tm_sec > tm.tm_sec)
            {
                printf("author: %s", comm[j].author);
                printf("commit id: %d\n", comm[j].id);
                printf("message: %s", comm[j].message);
                printf("branch: %s", comm[j].branch);
                printf("time: %d/%d/%d %d:%d:%d\n", comm[j].time->tm_year, comm[j].time->tm_mon, comm[j].time->tm_mday, comm[j].time->tm_hour, comm[j].time->tm_min, comm[j].time->tm_sec);
                printf("file count: %d\n", comm[j].filecount);
                printf("\n");
            }
        }
    }
    else if (argc == 5 && !strcmp(argv[2], "-before"))
    {
        struct tm tm;
        sscanf(argv[3], "%d/%d/%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
        sscanf(argv[4], "%d:%d", &tm.tm_hour, &tm.tm_min);
        char log[1000][4096];
        FILE *log_file = fopen(log_path, "r");
        char *line = malloc(PATH_MAX);
        int i = 0;
        int n = atoi(argv[3]);
        while (fgets(line, PATH_MAX, log_file) != NULL)
        {
            strcpy(log[i], line);
            i++;
        }
        commit_info comm[i];
        int k = 0;
        for (int j = 0; j < i; j++)
        {
            if (j % 6 == 0)
            {
                comm[k].time = malloc(sizeof(struct tm));
                strcpy(comm[k].author, log[j]);
                comm[k].id = atoi(log[j + 1]);
                strcpy(comm[k].message, log[j + 2]);
                strcpy(comm[k].branch, log[j + 3]);
                sscanf(log[j + 4], "%d/%d/%d %d:%d:%d", &comm[k].time->tm_year, &comm[k].time->tm_mon, &comm[k].time->tm_mday, &comm[k].time->tm_hour, &comm[k].time->tm_min, &comm[k].time->tm_sec);
                comm[k].filecount = atoi(log[j + 5]);
                k++;
            }
        }
        if (n > k)
        {
            n = k;
        }
        for (int j = k - 1; j >= k - n; j--)
        {
            if (comm[j].time->tm_year < tm.tm_year)
            {
                printf("author: %s", comm[j].author);
                printf("commit id: %d\n", comm[j].id);
                printf("message: %s", comm[j].message);
                printf("branch: %s", comm[j].branch);
                printf("time: %d/%d/%d %d:%d:%d\n", comm[j].time->tm_year, comm[j].time->tm_mon, comm[j].time->tm_mday, comm[j].time->tm_hour, comm[j].time->tm_min, comm[j].time->tm_sec);
                printf("file count: %d\n", comm[j].filecount);
                printf("\n");
            }
            else if (comm[j].time->tm_year == tm.tm_year && comm[j].time->tm_mon < tm.tm_mon)
            {
                printf("author: %s", comm[j].author);
                printf("commit id: %d\n", comm[j].id);
                printf("message: %s", comm[j].message);
                printf("branch: %s", comm[j].branch);
                printf("time: %d/%d/%d %d:%d:%d\n", comm[j].time->tm_year, comm[j].time->tm_mon, comm[j].time->tm_mday, comm[j].time->tm_hour, comm[j].time->tm_min, comm[j].time->tm_sec);
                printf("file count: %d\n", comm[j].filecount);
                printf("\n");
            }
            else if (comm[j].time->tm_year == tm.tm_year && comm[j].time->tm_mon == tm.tm_mon && comm[j].time->tm_mday < tm.tm_mday)
            {
                printf("author: %s", comm[j].author);
                printf("commit id: %d\n", comm[j].id);
                printf("message: %s", comm[j].message);
                printf("branch: %s", comm[j].branch);
                printf("time: %d/%d/%d %d:%d:%d\n", comm[j].time->tm_year, comm[j].time->tm_mon, comm[j].time->tm_mday, comm[j].time->tm_hour, comm[j].time->tm_min, comm[j].time->tm_sec);
                printf("file count: %d\n", comm[j].filecount);
                printf("\n");
            }
            else if (comm[j].time->tm_year == tm.tm_year && comm[j].time->tm_mon == tm.tm_mon && comm[j].time->tm_mday == tm.tm_mday && comm[j].time->tm_hour < tm.tm_hour)
            {
                printf("author: %s", comm[j].author);
                printf("commit id: %d\n", comm[j].id);
                printf("message: %s", comm[j].message);
                printf("branch: %s", comm[j].branch);
                printf("time: %d/%d/%d %d:%d:%d\n", comm[j].time->tm_year, comm[j].time->tm_mon, comm[j].time->tm_mday, comm[j].time->tm_hour, comm[j].time->tm_min, comm[j].time->tm_sec);
                printf("file count: %d\n", comm[j].filecount);
                printf("\n");
            }
            else if (comm[j].time->tm_year == tm.tm_year && comm[j].time->tm_mon == tm.tm_mon && comm[j].time->tm_mday == tm.tm_mday && comm[j].time->tm_hour == tm.tm_hour && comm[j].time->tm_min < tm.tm_min)
            {
                printf("author: %s", comm[j].author);
                printf("commit id: %d\n", comm[j].id);
                printf("message: %s", comm[j].message);
                printf("branch: %s", comm[j].branch);
                printf("time: %d/%d/%d %d:%d:%d\n", comm[j].time->tm_year, comm[j].time->tm_mon, comm[j].time->tm_mday, comm[j].time->tm_hour, comm[j].time->tm_min, comm[j].time->tm_sec);
                printf("file count: %d\n", comm[j].filecount);
                printf("\n");
            }
        }
    }
    else if (argc == 4 && !strcmp(argv[2], "-search"))
    {
        char log[1000][4096];
        FILE *log_file = fopen(log_path, "r");
        char *line = malloc(PATH_MAX);
        int i = 0;
        while (fgets(line, PATH_MAX, log_file) != NULL)
        {
            strcpy(log[i], line);
            i++;
        }
        commit_info comm[i];
        int k = 0;
        for (int j = 0; j < i; j++)
        {
            if (j % 6 == 0)
            {
                comm[k].time = malloc(sizeof(struct tm));
                strcpy(comm[k].author, log[j]);
                comm[k].id = atoi(log[j + 1]);
                strcpy(comm[k].message, log[j + 2]);
                strcpy(comm[k].branch, log[j + 3]);
                sscanf(log[j + 4], "%d/%d/%d %d:%d:%d", &comm[k].time->tm_year, &comm[k].time->tm_mon, &comm[k].time->tm_mday, &comm[k].time->tm_hour, &comm[k].time->tm_min, &comm[k].time->tm_sec);
                comm[k].filecount = atoi(log[j + 5]);
                k++;
            }
        }
        for (int j = k - 1; j >= 0; j--)
        {
            char *search = strstr(comm[j].message, argv[3]);
            if (search != NULL)
            {
                printf("author: %s", comm[j].author);
                printf("commit id: %d\n", comm[j].id);
                printf("message: %s", comm[j].message);
                printf("branch: %s", comm[j].branch);
                printf("time: %d/%d/%d %d:%d:%d\n", comm[j].time->tm_year, comm[j].time->tm_mon, comm[j].time->tm_mday, comm[j].time->tm_hour, comm[j].time->tm_min, comm[j].time->tm_sec);
                printf("file count: %d\n", comm[j].filecount);
                printf("\n");
            }
        }
    }
}
void checkout(int argc, char **argv)
{
    if (!strcmp(argv[2], "HEAD"))
    {
        // get current branch from branch.txt
        // get last commit id from branch folder list.txt
        // checkout files from that commit
        char *branch_path = malloc(PATH_MAX);
        char *cwd;
        char *buffer = malloc(PATH_MAX);
        cwd = getcwd(buffer, PATH_MAX);
        char *repo = CheckInit(cwd);
        sprintf(branch_path, "%s/.magit/commits/branch.txt", repo);
        FILE *branch_file = fopen(branch_path, "r");
        char *branch_name = malloc(PATH_MAX);
        fgets(branch_name, PATH_MAX, branch_file);
        fclose(branch_file);
        sprintf(branch_path, "%s/.magit/branch/%s/list.txt", repo, branch_name);
        FILE *list_file = fopen(branch_path, "r");
        int ids[1000], i = 0;
        char *line = malloc(PATH_MAX);
        while (fgets(line, 1000, list_file) != NULL)
        {
            line[strcspn(line, "\n")] = '\0';
            ids[i] = atoi(line);
            i++;
        }
        if (i == 0)
        {
            char *checkout_commnad = malloc(PATH_MAX);
            sprintf(checkout_commnad, "magit checkout %s", branch_name);
            system(checkout_commnad);
        }
        else
        {
            char *checkout_command = malloc(PATH_MAX);
            sprintf(checkout_command, "magit checkout %d", ids[i - 1]);
            system(checkout_command);
        }
    }
    else if (!strncmp(argv[2], "HEAD-", 5))
    {
        // get current branch from branch.txt
        // get last commit id from branch folder list.txt
        // checkout files from that commit
        char *branch_path = malloc(PATH_MAX);
        char *cwd;
        char *buffer = malloc(PATH_MAX);
        cwd = getcwd(buffer, PATH_MAX);
        char *repo = CheckInit(cwd);
        sprintf(branch_path, "%s/.magit/commits/branch.txt", repo);
        FILE *branch_file = fopen(branch_path, "r");
        char *branch_name = malloc(PATH_MAX);
        fgets(branch_name, PATH_MAX, branch_file);
        fclose(branch_file);
        sprintf(branch_path, "%s/.magit/branch/%s/list.txt", repo, branch_name);
        FILE *list_file = fopen(branch_path, "r");
        int ids[1000], i = 0;
        char *line = malloc(PATH_MAX);
        while (fgets(line, 1000, list_file) != NULL)
        {
            line[strcspn(line, "\n")] = '\0';
            ids[i] = atoi(line);
            i++;
        }
        int n;
        sscanf(argv[2], "HEAD-%d", &n);
        if (n > i)
        {
            puts("you cant go back that much");
            return;
        }
        else if (n == i)
        {
            sprintf(branch_path, "%s/.magit/branch/%s/head.txt", repo, branch_name);
            FILE *head_file = fopen(branch_path, "r");
            char *head_id = malloc(PATH_MAX);
            fgets(head_id, PATH_MAX, head_file);
            char *head_branch = malloc(PATH_MAX);
            fgets(head_branch, PATH_MAX, head_file);
            fclose(head_file);
            char *checkout_command = malloc(PATH_MAX);
            sprintf(checkout_command, "magit checkout %s", head_branch);
            system(checkout_command);
        }
        else
        {
            char *checkout_command = malloc(PATH_MAX);
            sprintf(checkout_command, "magit checkout %d", ids[i - n - 1]);
            system(checkout_command);
        }
    }
    else
    {
        char *cwd;
        char *buffer = malloc(PATH_MAX);
        cwd = getcwd(buffer, PATH_MAX);
        char *repo = CheckInit(cwd);
        char *branch_path = malloc(PATH_MAX);
        sprintf(branch_path, "%s/.magit/branch", repo);
        DIR *dir = opendir(branch_path);
        struct dirent *fp;
        int flag = 0;
        // get lastid from commit folder
        char *lastid_path = malloc(PATH_MAX);
        sprintf(lastid_path, "%s/.magit/commits/last_id.txt", repo);
        FILE *lastid_file = fopen(lastid_path, "r");
        char *lastid = malloc(PATH_MAX);
        fgets(lastid, PATH_MAX, lastid_file);
        int last_id = atoi(lastid);
        while ((fp = readdir(dir)) != NULL)
        {
            if (strcmp(fp->d_name, ".") == 0 || strcmp(fp->d_name, "..") == 0)
            {
                continue;
            }
            if (fp->d_type == DT_DIR)
            {
                if (!strcmp(fp->d_name, argv[2]))
                {
                    flag = 1;
                    break;
                }
            }
        }
        if (flag)
        {
            // check if we have un commited files in stage folder
            char *stage_path = malloc(PATH_MAX);
            sprintf(stage_path, "%s/.magit/stage", repo);
            int count = filecounter(stage_path);
            if (count > 0)
            {
                puts("you have uncommited files in stage folder");
                return;
            }
            // chxeck if any commit exist in the branch and if exist copy all files from the last commit to the current directory
            sprintf(branch_path, "%s/.magit/branch/%s", repo, argv[2]);
            dir = opendir(branch_path);
            int max_id = -1;
            while ((fp = readdir(dir)) != NULL)
            {
                if (strcmp(fp->d_name, ".") == 0 || strcmp(fp->d_name, "..") == 0 || strcmp(fp->d_name, ".magit") == 0)
                {
                    continue;
                }
                if (fp->d_type == DT_DIR)
                {
                    int id = atoi(fp->d_name);
                    if (id > max_id)
                    {
                        max_id = id;
                    }
                }
            }
            if (last_id == -1)
            {
                puts("no commits in this branch");
                return;
            }
            // if no commit exist and we have to checkout to parent commit
            if (max_id == -1)
            {
                // get the head commit id and branch from info.txt and copy files from that commit into repo
                char *info_path = malloc(PATH_MAX);
                sprintf(info_path, "%s/.magit/branch/%s/head.txt", repo, argv[2]);
                FILE *info_file = fopen(info_path, "r");
                char *line = malloc(PATH_MAX);
                fgets(line, PATH_MAX, info_file);
                int id = atoi(line);
                fgets(line, PATH_MAX, info_file);
                char *branch = malloc(PATH_MAX);
                strcpy(branch, line);
                fclose(info_file);
                char *last_commit_path = malloc(PATH_MAX);
                sprintf(last_commit_path, "%s/.magit/branch/%s/%d", repo, branch, id);
                dir = opendir(last_commit_path);
                clean();
                while ((fp = readdir(dir)) != NULL)
                {
                    if (strcmp(fp->d_name, ".") == 0 || strcmp(fp->d_name, "..") == 0 || strcmp(fp->d_name, ".magit") == 0)
                    {
                        continue;
                    }
                    if (fp->d_type == DT_DIR)
                    {
                        char *path = malloc(PATH_MAX);
                        sprintf(path, "%s/%s", last_commit_path, fp->d_name);
                        char *cp_commad = malloc(PATH_MAX);
                        sprintf(cp_commad, "cp -r %s %s", path, repo);
                        system(cp_commad);
                    }
                    else if (fp->d_type == DT_REG)
                    {
                        char *path = malloc(PATH_MAX);
                        sprintf(path, "%s/%s", last_commit_path, fp->d_name);
                        char *cp_commad = malloc(PATH_MAX);
                        sprintf(cp_commad, "cp %s %s", path, repo);
                        system(cp_commad);
                    }
                }
                // update commit folder txts
                char *commit_path = malloc(PATH_MAX);
                sprintf(commit_path, "%s/.magit/commits/branch.txt", repo);
                FILE *branch_file = fopen(commit_path, "w");
                fprintf(branch_file, "%s", argv[2]);
                fclose(branch_file);
                sprintf(commit_path, "%s/.magit/commits/previd.txt", repo);
                FILE *prev_file = fopen(commit_path, "w");
                fprintf(prev_file, "%d", id);
                fclose(prev_file);
                // debug prints

                return;
            }
            // copy files from last commit
            clean();
            char *last_commit_path = malloc(PATH_MAX);
            sprintf(last_commit_path, "%s/.magit/branch/%s/%d", repo, argv[2], max_id);
            dir = opendir(last_commit_path);
            while ((fp = readdir(dir)) != NULL)
            {
                if (strcmp(fp->d_name, ".") == 0 || strcmp(fp->d_name, "..") == 0 || strcmp(fp->d_name, ".magit") == 0)
                {
                    continue;
                }
                if (fp->d_type == DT_DIR)
                {
                    char *path = malloc(PATH_MAX);
                    sprintf(path, "%s/%s", last_commit_path, fp->d_name);
                    char *cp_commad = malloc(PATH_MAX);
                    sprintf(cp_commad, "cp -r %s %s", path, repo);
                    system(cp_commad);
                }
                else if (fp->d_type == DT_REG)
                {
                    char *path = malloc(PATH_MAX);
                    sprintf(path, "%s/%s", last_commit_path, fp->d_name);
                    char *cp_commad = malloc(PATH_MAX);
                    sprintf(cp_commad, "cp %s %s", path, repo);
                    system(cp_commad);
                }
            }
            // update commit folder txts
            char *commit_path = malloc(PATH_MAX);
            sprintf(commit_path, "%s/.magit/commits/previd.txt", repo);
            FILE *prev_file = fopen(commit_path, "w");
            fprintf(prev_file, "%d", max_id);
            fclose(prev_file);

            char *branchfile_path = malloc(PATH_MAX);
            sprintf(branchfile_path, "%s/.magit/commits/branch.txt", repo);
            FILE *branchfile = fopen(branchfile_path, "w");
            fprintf(branchfile, "%s", argv[2]);
            return;
        }
        else
        { // check if any thing other than numbers are in argv2 error
            for (int i = 0; i < strlen(argv[2]); i++)
            {
                if (argv[2][i] < '0' || argv[2][i] > '9')
                {
                    puts("branch or commit not found");
                    return;
                }
            }
            // search all branch folders for a commite with argv[2] id
            int flag = 0;
            char *branch_path = malloc(PATH_MAX);
            sprintf(branch_path, "%s/.magit/branch", repo);
            dir = opendir(branch_path);
            struct dirent *fp;
            char *branch_name = malloc(PATH_MAX);
            while ((fp = readdir(dir)) != NULL)
            {
                if (strcmp(fp->d_name, ".") == 0 || strcmp(fp->d_name, "..") == 0 || strcmp(fp->d_name, ".magit") == 0)
                {
                    continue;
                }
                if (fp->d_type == DT_DIR)
                {
                    sprintf(branch_path, "%s/.magit/branch/%s", repo, fp->d_name);
                    DIR *dir2 = opendir(branch_path);
                    struct dirent *fp2;
                    while ((fp2 = readdir(dir2)) != NULL)
                    {
                        if (strcmp(fp2->d_name, ".") == 0 || strcmp(fp2->d_name, "..") == 0 || strcmp(fp2->d_name, ".magit") == 0)
                        {
                            continue;
                        }
                        if (fp2->d_type == DT_DIR)
                        {
                            int id = atoi(fp2->d_name);
                            if (id == atoi(argv[2]))
                            {
                                flag = 1;
                                strcpy(branch_name, fp->d_name);
                                break;
                            }
                        }
                    }
                    if (flag)
                    {
                        break;
                    }
                }
            }
            if (flag)
            {
                // check if we have un commited files in stage folder
                char *stage_path = malloc(PATH_MAX);
                sprintf(stage_path, "%s/.magit/stage", repo);
                int count = filecounter(stage_path);
                if (count > 0)
                {
                    puts("you have uncommited files in stage folder");
                    return;
                }
                clean();
                // copy files from commit
                char *last_commit_path = malloc(PATH_MAX);
                sprintf(last_commit_path, "%s/.magit/branch/%s/%d", repo, branch_name, atoi(argv[2]));
                dir = opendir(last_commit_path);
                struct dirent *fp;
                while ((fp = readdir(dir)) != NULL)
                {
                    if (strcmp(fp->d_name, ".") == 0 || strcmp(fp->d_name, "..") == 0 || strcmp(fp->d_name, ".magit") == 0)
                    {
                        continue;
                    }
                    if (fp->d_type == DT_DIR)
                    {
                        char *path = malloc(PATH_MAX);
                        sprintf(path, "%s/%s", last_commit_path, fp->d_name);
                        char *cp_commad = malloc(PATH_MAX);
                        sprintf(cp_commad, "cp -r %s %s", path, repo);
                        system(cp_commad);
                    }
                    else if (fp->d_type == DT_REG)
                    {
                        char *path = malloc(PATH_MAX);
                        sprintf(path, "%s/%s", last_commit_path, fp->d_name);
                        char *cp_commad = malloc(PATH_MAX);
                        sprintf(cp_commad, "cp %s %s", path, repo);
                        system(cp_commad);
                    }
                }
                // update commit folder txts
                char *commit_path = malloc(PATH_MAX);
                sprintf(commit_path, "%s/.magit/commits/previd.txt", repo);
                FILE *prev_file = fopen(commit_path, "w");
                fprintf(prev_file, "%d", atoi(argv[2]));
                fclose(prev_file);
                sprintf(commit_path, "%s/.magit/commits/branch.txt", repo);
                FILE *branch_file = fopen(commit_path, "w");
                fprintf(branch_file, "%s", branch_name);
                fclose(branch_file);
                return;
            }
            else
            {
                puts("branch or commit not found");
                return;
            }
        }
    }
}
void branch(int argc, char **argv)
{
    char *branch_path = malloc(PATH_MAX);
    char *cwd;
    char buffer[PATH_MAX];
    cwd = getcwd(buffer, PATH_MAX);
    char *repo = CheckInit(cwd);
    sprintf(branch_path, "%s/.magit/branch", repo);
    DIR *dir = opendir(branch_path);
    struct dirent *fp;
    if (argc == 2 && !strcmp(argv[1], "branch"))
    {
        while ((fp = readdir(dir)) != NULL)
        {
            if (strcmp(fp->d_name, ".") == 0 || strcmp(fp->d_name, "..") == 0)
            {
                continue;
            }
            if (fp->d_type == DT_DIR)
            {
                printf("%s\n", fp->d_name);
            }
        }
        return;
    }
    else
    {
        while ((fp = readdir(dir)) != NULL)
        {
            if (strcmp(fp->d_name, ".") == 0 || strcmp(fp->d_name, "..") == 0 || strcmp(fp->d_name, ".magit") == 0)
            {
                continue;
            }
            if (fp->d_type == DT_DIR)
            {
                if (!strcmp(fp->d_name, argv[2]))
                {
                    puts("branch already exists");
                    return;
                }
            }
        }
        char *branch_path2 = malloc(PATH_MAX);
        sprintf(branch_path2, "%s/.magit/branch/%s", repo, argv[2]);
        mkdir(branch_path2, 0777);
        char *branch_path3 = malloc(PATH_MAX);
        sprintf(branch_path3, "%s/.magit/branch/%s/head.txt", repo, argv[2]);
        FILE *branch_file = fopen(branch_path3, "w");
        char *previd_path = malloc(PATH_MAX);
        sprintf(previd_path, "%s/.magit/commits/previd.txt", repo);
        FILE *previd_file = fopen(previd_path, "r");
        char *line = malloc(PATH_MAX);
        fgets(line, PATH_MAX, previd_file);
        fprintf(branch_file, "%s\n", line);
        fclose(previd_file);
        char *branchfile_path = malloc(PATH_MAX);
        sprintf(branchfile_path, "%s/.magit/commits/branch.txt", repo);
        FILE *branchfile = fopen(branchfile_path, "r");
        fgets(line, PATH_MAX, branchfile);
        fprintf(branch_file, "%s", line);
        fclose(branchfile);
        fclose(branch_file);
        char *list_path = malloc(PATH_MAX);
        sprintf(list_path, "%s/.magit/branch/%s/list.txt", repo, argv[2]);
        FILE *list_file = fopen(list_path, "w");
        fclose(list_file);
        return;
    }
}
char *branchfinder(int id)
{
    char *cwd;
    char buffer[PATH_MAX];
    cwd = getcwd(buffer, PATH_MAX);
    char *repo = CheckInit(cwd);
    char *branch_path = malloc(PATH_MAX);
    sprintf(branch_path, "%s/.magit/branch", repo);
    DIR *dir = opendir(branch_path);
    struct dirent *fp;
    while ((fp = readdir(dir)) != NULL)
    {
        if (strcmp(fp->d_name, ".") == 0 || strcmp(fp->d_name, "..") == 0 || strcmp(fp->d_name, ".magit") == 0)
        {
            continue;
        }
        if (fp->d_type == DT_DIR)
        {
            sprintf(branch_path, "%s/.magit/branch/%s", repo, fp->d_name);
            DIR *dir2 = opendir(branch_path);
            struct dirent *fp2;
            while ((fp2 = readdir(dir2)) != NULL)
            {
                if (strcmp(fp2->d_name, ".") == 0 || strcmp(fp2->d_name, "..") == 0 || strcmp(fp2->d_name, ".magit") == 0)
                {
                    continue;
                }
                if (fp2->d_type == DT_DIR)
                {
                    int id2 = atoi(fp2->d_name);
                    if (id2 == id)
                    {
                        return fp->d_name;
                    }
                }
            }
        }
    }
}
int lastid()
{
    char *cwd;
    char buffer[PATH_MAX];
    cwd = getcwd(buffer, PATH_MAX);
    char *repo = CheckInit(cwd);
    char *branch_path = malloc(PATH_MAX);
    sprintf(branch_path, "%s/.magit/commits/last_id.txt", repo);
    FILE *lastid_file = fopen(branch_path, "r");
    char *lastid = malloc(PATH_MAX);
    fgets(lastid, PATH_MAX, lastid_file);
    return atoi(lastid);
}
int comparefiles(char *first, char *second)
{
    FILE *file1 = fopen(first, "rb");
    FILE *file2 = fopen(second, "rb");
    if (file1 == NULL || file2 == NULL)
    {
        if (file1)
            fclose(file1);
        if (file2)
            fclose(file2);
        return 0;
    }
    int ch1, ch2;
    int result = 1;
    while (1)
    {
        ch1 = fgetc(file1);
        ch2 = fgetc(file2);
        if (ch1 != ch2)
        {
            result = 0;
            break;
        }
        if (ch1 == EOF || ch2 == EOF)
        {
            break;
        }
    }
    if (ch1 != EOF || ch2 != EOF)
    {
        result = 0;
    }
    fclose(file1);
    fclose(file2);
    return result;
}
void status(int argc, char **argv)
{
    char *cwd;
    char buffer[PATH_MAX];
    cwd = getcwd(buffer, PATH_MAX);
    char *repo = CheckInit(cwd);
    DIR *dir = opendir(cwd);
    struct dirent *fp;
    while ((fp = readdir(dir)) != NULL)
    {
        if (strcmp(fp->d_name, ".magit") == 0 || strcmp(fp->d_name, ".") == 0 || strcmp(fp->d_name, "..") == 0)
        {
            continue;
        }
        if (fp->d_type == DT_DIR)
        {
            chdir(fp->d_name);
            status(argc, argv);
            chdir("..");
        }
        else if (fp->d_type == DT_REG)
        {
            char stats;
            char *path = malloc(PATH_MAX);
            sprintf(path, "%s/%s", cwd, fp->d_name);
            int stage = CheckStage(path);
            int last_id = lastid();
            char *branch = branchfinder(last_id);
            char *commit_path = malloc(PATH_MAX);
            sprintf(commit_path, "%s/.magit/branch/%s/%d", repo, branch, last_id);
            char *realpath = malloc(PATH_MAX);
            strcpy(realpath, cwd);
            realpath = realpath + strlen(repo);
            char *file_path = malloc(PATH_MAX);
            sprintf(file_path, "%s%s/%s", commit_path, realpath, fp->d_name);
            FILE *file = fopen(file_path, "r");
            if (file == NULL)
            {
                stats = 'A';
            }
            else
            {
                char *file1 = malloc(PATH_MAX);
                char *file2 = malloc(PATH_MAX);
                sprintf(file1, "%s/%s", cwd, fp->d_name);
                sprintf(file2, "%s%s/%s", commit_path, realpath, fp->d_name);
                if (comparefiles(file1, file2))
                {
                    stats = 'U';
                }
                else
                {
                    stats = 'M';
                }
            }
            if (stage)
            {
                printf("%s +%c\n", fp->d_name, stats);
            }
            else
            {
                printf("%s -%c\n", fp->d_name, stats);
            }
        }
    }
}
void commitstatus(int argc, char **argv)
{
    char *cwd;
    char buffer[PATH_MAX];
    cwd = getcwd(buffer, PATH_MAX);
    char *repo = CheckInit(cwd);
    char *commit_path = malloc(PATH_MAX);
    int last_id = lastid();
    char *branch = branchfinder(last_id);
    char *realpath = malloc(PATH_MAX);
    sprintf(commit_path, "%s/.magit/branch/%s/%d", repo, branch, last_id);
    if (!strcmp(repo, cwd))
    {
        chdir(commit_path);
    }
    char *cwd2;
    char buffer2[PATH_MAX];
    cwd2 = getcwd(buffer2, PATH_MAX);
    DIR *dir = opendir(cwd2);
    struct dirent *fp;
    while ((fp = readdir(dir)) != NULL)
    {
        if (strcmp(fp->d_name, ".") == 0 || strcmp(fp->d_name, "..") == 0 || strcmp(fp->d_name, ".magit") == 0)
        {
            continue;
        }
        if (fp->d_type == DT_DIR)
        {
            chdir(fp->d_name);
            commitstatus(argc, argv);
            chdir("..");
        }
        else if (fp->d_type == DT_REG)
        {
            char stats = 'U';
            char *path = malloc(PATH_MAX);
            sprintf(path, "%s/%s", cwd2, fp->d_name);
            char *main_path = malloc(PATH_MAX);
            char *file = malloc(PATH_MAX);
            strcpy(file, cwd2);
            file = file + strlen(commit_path);
            sprintf(main_path, "%s%s/%s", repo, file, fp->d_name);
            FILE *check = fopen(main_path, "r");
            if (check == NULL)
            {
                stats = 'D';
            }
            int stage = CheckStage(main_path);
            if (stage)
            {
                printf("%s +%c\n", fp->d_name, stats);
            }
            else
            {
                printf("%s -%c\n", fp->d_name, stats);
            }
        }
    }
    chdir(cwd);
}
void alias(int argc, char **argv, int mode)
{
    char *cwd = malloc(PATH_MAX);
    cwd = getcwd(cwd, PATH_MAX);
    if (mode == 1)
    {
        char path[] = "/home/aminkoohi/.magitconfig";
        DIR *dir = opendir(path);
        if (dir == NULL)
        {
            mkdir(path, 0777);
            char *alias = malloc(PATH_MAX);
            sscanf(argv[3], "alias.%s", alias);
            char *name = malloc(PATH_MAX);
            sprintf(name, "%s.txt", alias);
            chdir("/");
            chdir(path);
            FILE *aliastxt = fopen(name, "w");
            fprintf(aliastxt, "%s", argv[4]);
            chdir(cwd);
        }
        else
        {
            char *alias = malloc(PATH_MAX);
            sscanf(argv[3], "alias.%s", alias);
            char *name = malloc(PATH_MAX);
            sprintf(name, "%s.txt", alias);
            chdir("/");
            chdir(path);
            FILE *aliastxt = fopen(name, "w");
            fprintf(aliastxt, "%s", argv[4]);
            chdir(cwd);
        }
    }
    else
    {
        char cwd[PATH_MAX];
        getcwd(cwd, PATH_MAX);
        char *repo = CheckInit(cwd);
        if (repo == NULL)
        {
            puts("you are not in a magit repository");
            return;
        }
        char *alias = malloc(PATH_MAX);
        sscanf(argv[2], "alias.%s", alias);
        char *alias_path = malloc(PATH_MAX);
        sprintf(alias_path, "%s/.magit/%s.txt", repo, alias);
        FILE *alias_file = fopen(alias_path, "w");
        fprintf(alias_file, "%s", argv[3]);
    }
}
int aliasfind(int argc, char **argv)
{
    char cwd[PATH_MAX];
    getcwd(cwd, PATH_MAX);
    char *repo = CheckInit(cwd);
    char *alias_path = malloc(PATH_MAX);
    sprintf(alias_path, "%s/.magit/%s.txt", repo, argv[1]);
    FILE *alias_file = fopen(alias_path, "r");
    if (alias_file == NULL)
    {
        char path[] = "/home/aminkoohi/.magitconfig";
        chdir("/");
        chdir(path);
        char *name = malloc(PATH_MAX);
        sprintf(name, "%s.txt", argv[1]);
        alias_file = fopen(name, "r");
        if (alias_file == NULL)
        {
            chdir(cwd);
            return 0;
        }
        char *alias_txt = malloc(PATH_MAX);
        fgets(alias_txt, PATH_MAX, alias_file);
        char *system_command = malloc(PATH_MAX);
        chdir(cwd);
        sprintf(system_command, "magit %s", alias_txt);
        system(system_command);
        return 1;
    }
    char *alias_txt = malloc(PATH_MAX);
    fgets(alias_txt, PATH_MAX, alias_file);
    char *system_command = malloc(PATH_MAX);
    sprintf(system_command, "magit %s", alias_txt);
    system(system_command);
    return 1;
}
