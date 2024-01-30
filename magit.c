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
                        printf("%s is already staged\n", argv[i]);
                        continue;
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
                        printf("%s is already staged\n", argv[i]);
                        continue;
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
            if (!strcmp(argv[2], "-m"))
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
        else if (!strcmp(argv[1], "log"))
        {
            logg(argc, argv);
        }
        else if (!strcmp(argv[1], "checkout"))
        {
            checkout(argc, argv);
        }
        else
        {
            INVALID
        }
    }
    return 0;
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
    FILE *destinationFile = fopen(destinationPath, "wb");
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
    fopen(".magit/status.txt", "w");
    fopen(".magit/reset.txt", "w");
    fopen(".magit/add.txt", "w");
    FILE *branch = fopen(".magit/commits/branch.txt", "w");
    FILE *previd = fopen(".magit/commits/previd.txt", "w");
    FILE *last_id = fopen(".magit/commits/last_id.txt", "w");
    FILE *log = fopen(".magit/commits/log.txt", "w");
    FILE *prevbranch = fopen(".magit/commits/prevbranch.txt", "w");
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
        sprintf(rm_path, "rm %s/.magit/stage/%s", repo, purepath);
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
    char *user = malloc(PATH_MAX);
    char *email = malloc(PATH_MAX);
    sprintf(user, "%s/.magit/user.txt", repo);
    sprintf(email, "%s/.magit/email.txt", repo);
    FILE *user_file = fopen(user, "r");
    FILE *email_file = fopen(email, "r");
    if (user_file == NULL || email_file == NULL)
    {
        // check global config
        char *user = malloc(PATH_MAX);
        char *email = malloc(PATH_MAX);
        sprintf(user, "/home/aminkoohi/.magitconfig/user.txt");
        sprintf(email, "/home/aminkoohi/.magitconfig/email.txt");
        user_file = fopen(user, "r");
        email_file = fopen(email, "r");
        if (user_file == NULL || email_file == NULL)
        {
            puts("you must set user name and email before committing");
            return;
        }
    }
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
        // copy files from previous commit
        char *prev_comm_path = malloc(PATH_MAX);
        sprintf(prev_comm_path, "%s/.magit/branch/%s/%d", repo, comm->branch, comm->prev);
        DIR *dir = opendir(prev_comm_path);
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
        char *stage_path = malloc(PATH_MAX);
        sprintf(stage_path, "%s/.magit/stage", repo);
        dir = opendir(stage_path);
        while ((fp = readdir(dir)) != NULL)
        {
            if (strcmp(fp->d_name, ".") == 0 || strcmp(fp->d_name, "..") == 0)
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
            if (!strcmp(comm[j].branch, argv[3]))
            {
                printf("author: %s", comm[j].author);
                printf("commit id: %d\n", comm[j].id);
                printf("message: %s", comm[j].message);
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
                printf("time: %d/%d/%d %d:%d:%d\n", comm[j].time->tm_year, comm[j].time->tm_mon, comm[j].time->tm_mday, comm[j].time->tm_hour, comm[j].time->tm_min, comm[j].time->tm_sec);
                printf("file count: %d\n", comm[j].filecount);
                printf("\n");
            }
            else if (comm[j].time->tm_year == tm.tm_year && comm[j].time->tm_mon > tm.tm_mon)
            {
                printf("author: %s", comm[j].author);
                printf("commit id: %d\n", comm[j].id);
                printf("message: %s", comm[j].message);
                printf("time: %d/%d/%d %d:%d:%d\n", comm[j].time->tm_year, comm[j].time->tm_mon, comm[j].time->tm_mday, comm[j].time->tm_hour, comm[j].time->tm_min, comm[j].time->tm_sec);
                printf("file count: %d\n", comm[j].filecount);
                printf("\n");
            }
            else if (comm[j].time->tm_year == tm.tm_year && comm[j].time->tm_mon == tm.tm_mon && comm[j].time->tm_mday > tm.tm_mday)
            {
                printf("author: %s", comm[j].author);
                printf("commit id: %d\n", comm[j].id);
                printf("message: %s", comm[j].message);
                printf("time: %d/%d/%d %d:%d:%d\n", comm[j].time->tm_year, comm[j].time->tm_mon, comm[j].time->tm_mday, comm[j].time->tm_hour, comm[j].time->tm_min, comm[j].time->tm_sec);
                printf("file count: %d\n", comm[j].filecount);
                printf("\n");
            }
            else if (comm[j].time->tm_year == tm.tm_year && comm[j].time->tm_mon == tm.tm_mon && comm[j].time->tm_mday == tm.tm_mday && comm[j].time->tm_hour > tm.tm_hour)
            {
                printf("author: %s", comm[j].author);
                printf("commit id: %d\n", comm[j].id);
                printf("message: %s", comm[j].message);
                printf("time: %d/%d/%d %d:%d:%d\n", comm[j].time->tm_year, comm[j].time->tm_mon, comm[j].time->tm_mday, comm[j].time->tm_hour, comm[j].time->tm_min, comm[j].time->tm_sec);
                printf("file count: %d\n", comm[j].filecount);
                printf("\n");
            }
            else if (comm[j].time->tm_year == tm.tm_year && comm[j].time->tm_mon == tm.tm_mon && comm[j].time->tm_mday == tm.tm_mday && comm[j].time->tm_hour == tm.tm_hour && comm[j].time->tm_min > tm.tm_min)
            {
                printf("author: %s", comm[j].author);
                printf("commit id: %d\n", comm[j].id);
                printf("message: %s", comm[j].message);
                printf("time: %d/%d/%d %d:%d:%d\n", comm[j].time->tm_year, comm[j].time->tm_mon, comm[j].time->tm_mday, comm[j].time->tm_hour, comm[j].time->tm_min, comm[j].time->tm_sec);
                printf("file count: %d\n", comm[j].filecount);
                printf("\n");
            }
            else if (comm[j].time->tm_year == tm.tm_year && comm[j].time->tm_mon == tm.tm_mon && comm[j].time->tm_mday == tm.tm_mday && comm[j].time->tm_hour == tm.tm_hour && comm[j].time->tm_min == tm.tm_min && comm[j].time->tm_sec > tm.tm_sec)
            {
                printf("author: %s", comm[j].author);
                printf("commit id: %d\n", comm[j].id);
                printf("message: %s", comm[j].message);
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
                printf("time: %d/%d/%d %d:%d:%d\n", comm[j].time->tm_year, comm[j].time->tm_mon, comm[j].time->tm_mday, comm[j].time->tm_hour, comm[j].time->tm_min, comm[j].time->tm_sec);
                printf("file count: %d\n", comm[j].filecount);
                printf("\n");
            }
            else if (comm[j].time->tm_year == tm.tm_year && comm[j].time->tm_mon < tm.tm_mon)
            {
                printf("author: %s", comm[j].author);
                printf("commit id: %d\n", comm[j].id);
                printf("message: %s", comm[j].message);
                printf("time: %d/%d/%d %d:%d:%d\n", comm[j].time->tm_year, comm[j].time->tm_mon, comm[j].time->tm_mday, comm[j].time->tm_hour, comm[j].time->tm_min, comm[j].time->tm_sec);
                printf("file count: %d\n", comm[j].filecount);
                printf("\n");
            }
            else if (comm[j].time->tm_year == tm.tm_year && comm[j].time->tm_mon == tm.tm_mon && comm[j].time->tm_mday < tm.tm_mday)
            {
                printf("author: %s", comm[j].author);
                printf("commit id: %d\n", comm[j].id);
                printf("message: %s", comm[j].message);
                printf("time: %d/%d/%d %d:%d:%d\n", comm[j].time->tm_year, comm[j].time->tm_mon, comm[j].time->tm_mday, comm[j].time->tm_hour, comm[j].time->tm_min, comm[j].time->tm_sec);
                printf("file count: %d\n", comm[j].filecount);
                printf("\n");
            }
            else if (comm[j].time->tm_year == tm.tm_year && comm[j].time->tm_mon == tm.tm_mon && comm[j].time->tm_mday == tm.tm_mday && comm[j].time->tm_hour < tm.tm_hour)
            {
                printf("author: %s", comm[j].author);
                printf("commit id: %d\n", comm[j].id);
                printf("message: %s", comm[j].message);
                printf("time: %d/%d/%d %d:%d:%d\n", comm[j].time->tm_year, comm[j].time->tm_mon, comm[j].time->tm_mday, comm[j].time->tm_hour, comm[j].time->tm_min, comm[j].time->tm_sec);
                printf("file count: %d\n", comm[j].filecount);
            }
            else if (comm[j].time->tm_year == tm.tm_year && comm[j].time->tm_mon == tm.tm_mon && comm[j].time->tm_mday == tm.tm_mday && comm[j].time->tm_hour == tm.tm_hour && comm[j].time->tm_min < tm.tm_min)
            {
                printf("author: %s", comm[j].author);
                printf("commit id: %d\n", comm[j].id);
                printf("message: %s", comm[j].message);
                printf("time: %d/%d/%d %d:%d:%d\n", comm[j].time->tm_year, comm[j].time->tm_mon, comm[j].time->tm_mday, comm[j].time->tm_hour, comm[j].time->tm_min, comm[j].time->tm_sec);
                printf("file count: %d\n", comm[j].filecount);
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
                printf("time: %d/%d/%d %d:%d:%d\n", comm[j].time->tm_year, comm[j].time->tm_mon, comm[j].time->tm_mday, comm[j].time->tm_hour, comm[j].time->tm_min, comm[j].time->tm_sec);
                printf("file count: %d\n", comm[j].filecount);
                printf("\n");
            }
        }
    }
}
void checkout(int argc, char **argv)
{
}
