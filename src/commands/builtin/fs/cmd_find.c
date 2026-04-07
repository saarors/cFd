#include "cmd_find.h"
#include <stdio.h>
#include <string.h>
#include <fnmatch.h>

#ifdef _WIN32
#  include <windows.h>
#  ifndef FNM_CASEFOLD
#    define FNM_CASEFOLD 0
#  endif
static int fnmatch(const char *p, const char *s, int f) {
    (void)f;
    return PathMatchSpecA(s,p) ? 0 : 1;
}
static void find_walk(const char *dir, const char *name_pat, int maxdepth, int depth) {
    if (maxdepth >= 0 && depth > maxdepth) return;
    char pat[512]; snprintf(pat,sizeof(pat),"%s\\*",dir);
    WIN32_FIND_DATAA fd; HANDLE h=FindFirstFileA(pat,&fd);
    if(h==INVALID_HANDLE_VALUE) return;
    do {
        if(strcmp(fd.cFileName,".")==0||strcmp(fd.cFileName,"..")==0) continue;
        char full[512]; snprintf(full,sizeof(full),"%s\\%s",dir,fd.cFileName);
        if(!name_pat||fnmatch(name_pat,fd.cFileName,0)==0) puts(full);
        if(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) find_walk(full,name_pat,maxdepth,depth+1);
    } while(FindNextFileA(h,&fd));
    FindClose(h);
}
#else
#  include <dirent.h>
#  include <sys/stat.h>
static void find_walk(const char *dir, const char *name_pat, int maxdepth, int depth) {
    if (maxdepth >= 0 && depth > maxdepth) return;
    DIR *d = opendir(dir); if (!d) return;
    struct dirent *ent;
    while ((ent = readdir(d))) {
        if (strcmp(ent->d_name,".")==0||strcmp(ent->d_name,"..")==0) continue;
        char full[4096]; snprintf(full,sizeof(full),"%s/%s",dir,ent->d_name);
        if (!name_pat || fnmatch(name_pat, ent->d_name, 0)==0) puts(full);
        struct stat st;
        if (stat(full,&st)==0&&S_ISDIR(st.st_mode)) find_walk(full,name_pat,maxdepth,depth+1);
    }
    closedir(d);
}
#endif

int cmd_find(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;
    const char *dir="."; const char *name_pat=NULL; int maxdepth=-1;
    for (int i=1;i<argc;i++) {
        if(strcmp(argv[i],"-name")==0&&i+1<argc) name_pat=argv[++i];
        else if(strcmp(argv[i],"-maxdepth")==0&&i+1<argc) maxdepth=atoi(argv[++i]);
        else if(argv[i][0]!='-') dir=argv[i];
    }
    find_walk(dir, name_pat, maxdepth, 0);
    return 0;
}
const cfd_command_t builtin_find = {
    "find","find [dir] [-name pat] [-maxdepth n]","Find files","filesystem",cmd_find,0,-1
};
