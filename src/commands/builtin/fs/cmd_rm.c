#include "cmd_rm.h"
#include "../../../utils/path.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#  include <windows.h>
#else
#  include <unistd.h>
#  include <dirent.h>
#  include <sys/stat.h>
#endif

static int rm_recursive(const char *path);

#ifndef _WIN32
static int rm_recursive(const char *path) {
    struct stat st;
    if (lstat(path, &st) != 0) { perror(path); return 1; }
    if (!S_ISDIR(st.st_mode)) return unlink(path) == 0 ? 0 : (perror(path), 1);
    DIR *d = opendir(path); if (!d) { perror(path); return 1; }
    struct dirent *ent; int ret = 0;
    while ((ent = readdir(d))) {
        if (strcmp(ent->d_name,".")==0||strcmp(ent->d_name,"..")==0) continue;
        char full[4096]; snprintf(full,sizeof(full),"%s/%s",path,ent->d_name);
        ret |= rm_recursive(full);
    }
    closedir(d);
    return rmdir(path)==0 ? ret : (perror(path), 1);
}
#else
static int rm_recursive(const char *path) {
    if (cfd_path_is_dir(path)) {
        char pattern[512]; snprintf(pattern,sizeof(pattern),"%s\\*",path);
        WIN32_FIND_DATAA fd; HANDLE h=FindFirstFileA(pattern,&fd);
        if (h!=INVALID_HANDLE_VALUE) {
            do {
                if (strcmp(fd.cFileName,".")==0||strcmp(fd.cFileName,"..")==0) continue;
                char full[512]; snprintf(full,sizeof(full),"%s\\%s",path,fd.cFileName);
                rm_recursive(full);
            } while(FindNextFileA(h,&fd));
            FindClose(h);
        }
        return RemoveDirectoryA(path)?0:1;
    }
    return DeleteFileA(path)?0:1;
}
#endif

int cmd_rm(cfd_session_t *sess, int argc, char **argv) {
    (void)sess; bool recursive=false, force=false; int ret=0;
    for (int i=1;i<argc;i++){
        if(strcmp(argv[i],"-r")==0||strcmp(argv[i],"-rf")==0||
           strcmp(argv[i],"-fr")==0){recursive=true;if(strchr(argv[i],'f'))force=true;continue;}
        if(strcmp(argv[i],"-f")==0){force=true;continue;}
        if(recursive){ ret|=rm_recursive(argv[i]); }
        else {
#ifdef _WIN32
            if(!DeleteFileA(argv[i])){if(!force){perror(argv[i]);}ret=1;}
#else
            if(unlink(argv[i])!=0){if(!force){perror(argv[i]);}ret=1;}
#endif
        }
    }
    return ret;
}
const cfd_command_t builtin_rm={
    "rm","rm [-rf] <file...>","Remove files or directories","filesystem",cmd_rm,1,-1
};
