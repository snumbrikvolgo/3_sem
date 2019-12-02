#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <getopt.h>
#include <assert.h>

#define BUF_SIZE 1024
#define MODE_SIZE 10

char* rights = "-rwxrwxrwx";

struct options {
    int all, list, numeric, recursive, inode, directory;
    int num;
    int not_show_hidden;
};

struct options options = {0,0,0,0,0,0,0,0};

static struct option longopts[] =
{
	   {"long",    			no_argument,	0,  'l' },
	   {"directory",		no_argument,	0,  'd' },
	   {"all",  			no_argument,	0,  'a' },
	   {"numeric-uid-gid",	no_argument,	0,  'n' },
	   {"recursive",  		no_argument,	0,	'R' },
	   {"inode",    		no_argument,	0,  'i' },
	   {0,					0,				0,  0	}
 };

 struct ls
{
    char*  path;
    size_t path_size;
    char*  rec_buf;
    size_t rec_size;
    char*  link;
    size_t len;
    char*  datestr;
    char*  modestr;
    struct stat st;
    struct stat stl;
};

struct ls* cur_node;

 void RunLs(char* path);
 void RunRecLs(char* dirName, char* path);
 void ParseOptions(int argc, char** argv);
 void CutStr(char* str);
 char* CatPath(char* buff, char* dirName, char* path);
void ShortPrint(char* dirName, char* path);
void LongPrint(char* dirName, char* path);
void NamePrint(struct stat* statBuf, char* path);

void ParseOptions(int argc, char** argv)
{
    int opt = 0;
    while ((opt = getopt_long(argc, argv, "lanRid", longopts, NULL)) != -1)
    {
        switch (opt)
        {
            case 'l':
                options.list = 1;
                break;
            case 'a':
                options.all = 1;
                break;
            case 'n':
                options.numeric = 1;
                break;
            case 'R':
                options.recursive = 1;
                break;
            case 'i':
                options.inode = 1;
                break;
            case 'd':
                options.directory = 1;
                break;
        }
    }
}

struct ls* lsInit()
{
    struct ls* ins = (struct ls*) malloc(sizeof(struct ls));

    ins->path = (char*) malloc(BUF_SIZE);
    ins->path_size = BUF_SIZE;
    ins->path[BUF_SIZE - 1] = '\0';
    ins->path[0] = '\0';

    ins->rec_buf = (char*) malloc(BUF_SIZE);
    ins->rec_size = BUF_SIZE;
    ins->rec_buf[BUF_SIZE - 1] = '\0';
    ins->rec_buf[0] = '\0';

    ins->link = (char*) malloc(BUF_SIZE);
    ins->link[BUF_SIZE - 1] = '\0';
    ins->link[0] = '\0';

    ins->datestr = (char*) malloc(BUF_SIZE);
    ins->datestr[BUF_SIZE - 1] = '\0';
    ins->modestr = (char*) malloc(MODE_SIZE);
    ins->modestr[MODE_SIZE - 1] = '\0';

    return ins;
}

void lsClear()
{
    if (cur_node->path)
        free(cur_node->path);
    if (cur_node->rec_buf)
        free(cur_node->rec_buf);
    if (cur_node->link)
        free(cur_node->link);
    if (cur_node->datestr)
        free(cur_node->datestr);
    if (cur_node->modestr)
        free(cur_node->modestr);
    free(cur_node);
    cur_node = NULL;
}


int main(int argc, char** argv)
{
    ParseOptions(argc, argv);
    printf("%d %d %d\n", options.recursive, options.list, options.all);
    cur_node = lsInit();
    if (argc == optind)
        RunLs(".");
    else{
        for (int i = optind; i < argc; i++)
            RunLs(argv[i]);
    }

    lsClear();
    return 0;
}

void RunLs(char* path){
    printf("run\n");

    if (lstat(path, &(cur_node -> st)) < 0){
        perror("Problems with statbuf during lstat\n");
        return errno;
    }
    if(S_ISDIR(cur_node -> st.st_mode) && !options.directory)
    {
        if(strcmp(".", path))
            printf("%s:\n", path);

        DIR* d = opendir(path);
        if (!d){
            perror("Cannot open directory\n");
            return errno;
        }
        struct dirent* dir = NULL;
        while ((dir = readdir(d)))
        {
            if (dir -> d_name[0] != '.' || options.all)
            {
                if(options.list || options.numeric)
                {
                    LongPrint(path, dir -> d_name);
                    printf("\n");
                }
                else
                {
                    ShortPrint(path, dir -> d_name);
                    printf("\n");
                }
            }
        }
        closedir(d); // rewinddir(d);

        d = opendir(path);
        dir = NULL;

        while(options.recursive && (dir = readdir(d)))
        {
            if ((dir -> d_name[0] != '.' || options.all) &&
                strcmp(".", dir -> d_name) && strcmp("..", dir -> d_name))
            {
                RunRecLs(path, dir -> d_name);
            }
        }
        closedir(d);
    }
    else
    {
        if (options.list || options.numeric)
        {
            LongPrint(NULL, path);
            printf("\n");
        }
        else
        {
            if (options.inode)
            {
                printf("%ld ", cur_node -> st.st_ino);
            }
            NamePrint(&(cur_node -> st), path);
        }
    }

    if(!options.list && !options.numeric)
        printf("\n");
}

void LongPrint(char* dirName, char* path)
{
    assert(path);

    if (!dirName)
        cur_node->path = path;
    else
    {
        cur_node->len = strlen(dirName) + strlen(path) + 2;
        if (cur_node->len > cur_node->path_size)
        {
             cur_node->path = (char*) realloc(cur_node->path, cur_node->len * 2);
             cur_node->path_size = cur_node->len * 2;
             cur_node->path[cur_node->len - 1] = '\0';
        }
        CatPath(cur_node->path, dirName, path);
    }

    if (lstat(cur_node->path, &(cur_node->st)))
    {

        fprintf(stderr, "unable to access '%s': %s\n",
                cur_node->path, strerror(errno));
        goto out;
    }

    if (options.inode)
        printf("%ld ", (long) cur_node->st.st_ino);

    if (S_ISDIR(cur_node->st.st_mode))
        printf("d");
    else if (S_ISCHR(cur_node->st.st_mode))
        printf("c");
    else if (S_ISBLK(cur_node->st.st_mode))
        printf("b");
    else if (S_ISFIFO(cur_node->st.st_mode))
        printf("p");
    else if (S_ISLNK(cur_node->st.st_mode))
        printf("l");
    else if (S_ISSOCK(cur_node->st.st_mode))
        printf("s");
    else
        printf("-");

    for (int j = 8; j >= 0; j--)
    {
        if (cur_node->st.st_mode & (1 << (8 - j)))
        {
            if (j % 3 == 0)
                cur_node->modestr[j] = 'r';
            else if (j % 3 == 1)
                cur_node->modestr[j] = 'w';
            else
                cur_node->modestr[j] = 'x';
        }
        else
        {
            cur_node->modestr[j] = '-';
        }
    }
    printf("%s %3ld ", cur_node->modestr, (long) cur_node->st.st_nlink);

    if (options.numeric)
        printf("%d %d ", cur_node->st.st_uid, cur_node->st.st_gid);
    else
    {
        struct passwd* pwu = getpwuid(cur_node->st.st_uid);
        if (pwu == NULL)
            printf("%d ", cur_node->st.st_uid);
        else
            printf("%s ", pwu->pw_name);

        struct group* gr = getgrgid(cur_node->st.st_gid);
        if (gr == NULL)
            printf("%d ", cur_node->st.st_gid);
        else
            printf("%s ", gr->gr_name);
    }

    printf("%7ld ", (long) cur_node->st.st_size);

    struct tm* time = localtime(&(cur_node->st.st_mtim.tv_sec));

    strftime(cur_node->datestr, BUF_SIZE - 1, "%Y %b %d %H:%M ", time);
    printf("%s", cur_node->datestr);

    if(!S_ISLNK(cur_node->st.st_mode))
    {
        NamePrint(&(cur_node->st), path);
    }
    else
    {
        cur_node->len = readlink(cur_node->path, cur_node->link, BUF_SIZE - 1);
        if (cur_node->len == -1)
        {
            fprintf(stderr, "unable to read symbolic link '%s': %s\n",
                    cur_node->path, strerror(errno));
            goto out;
        }
        cur_node->link[cur_node->len] = '\0';

        if (stat(cur_node->path, &(cur_node->stl)))
        {
            //link is broken
            printf("\x1b[1;31m%s\x1b[0m -> \x1b[1;31m%s\x1b[0m",
                   cur_node->path, cur_node->link);
        }
        else
        {
            NamePrint(&(cur_node->st), path);
            printf(" -> ");
            NamePrint(&(cur_node->stl), cur_node->link);
        }
    }

out:
    if (!dirName)
        cur_node->path = NULL;
}

void ShortPrint(char* dirName, char* path)
{
    cur_node->len = strlen(dirName) + strlen(path) + 2;
    if (cur_node->len > cur_node->path_size)
    {
        cur_node->path = (char*) realloc(cur_node->path, cur_node->len * 2);
        cur_node->path[cur_node->len - 1] = '\0';
        cur_node->path_size = cur_node->len * 2;
    }
    CatPath(cur_node->path, dirName, path);

    if (lstat(cur_node->path, &(cur_node->st)))
    {
        fprintf(stderr, "unable to access '%s': %s\n", cur_node->path, strerror(errno));
        return;
    }
    if (options.inode)
        printf("%ld ", (long) cur_node->st.st_ino);

    NamePrint(&(cur_node->st), path);
}

void NamePrint(struct stat* statBuf, char* path)
{
    assert(statBuf);
    assert(path);

    if (S_ISDIR(statBuf->st_mode))
        printf("\x1b[1;34m%s\x1b[0m", path);

    else if (S_ISREG(statBuf->st_mode) && (statBuf->st_mode & (1 << 6)))   //executable file
        printf("\x1b[1;32m%s\x1b[0m", path);

    else if (S_ISLNK(statBuf->st_mode))
        printf("\x1b[1;36m%s\x1b[0m", path);

    else if (S_ISFIFO(statBuf->st_mode))
        printf("\x1b[0;33;40m%s\x1b[0m", path);

    else if (S_ISCHR(statBuf->st_mode) || S_ISBLK(statBuf->st_mode))
        printf("\x1b[1;33;40m%s\x1b[0m", path);

    else
        printf("%s", path);
}


void RunRecLs(char* dirName, char* path)
{
    assert(dirName);
    assert(path);

    cur_node -> len = strlen(dirName) + strlen(path) + 2;
    if (cur_node -> len > cur_node->rec_size)
    {
        cur_node -> rec_buf = (char*) realloc(cur_node -> rec_buf, cur_node -> len*2);
        cur_node -> rec_buf[cur_node -> len - 1] = '\0';
        cur_node -> rec_size = cur_node -> len *2;
    }

    CatPath(cur_node -> rec_buf, dirName, path);

    if(lstat(cur_node -> rec_buf, &(cur_node -> st)))
    {
        perror("Cannot lstat on rec\n");
        return errno;
    }

    if(S_ISDIR(cur_node -> st.st_mode) && options.recursive)
    {
        printf("\n");
        RunLs(cur_node -> rec_buf);
    }
    CutStr(cur_node -> rec_buf);
}


char* CatPath(char* buff, char* dirName, char* path)
{
    assert(buff);
    assert(dirName);
    assert(path);

    if (strcmp(buff, dirName))
    {
        buff[0] = '\0';
        strcat(buff, dirName);
    }
    strcat(buff, "/");
    strcat(buff, path);

    return buff;
}

void CutStr(char* str)
{
    assert(str);

    cur_node->len = strlen(str);
    char* tmp = str + cur_node->len - 1;
    while(*tmp != '/' && tmp != str)
        tmp--;

    if (*tmp == '/')
        *tmp = '\0';
}
