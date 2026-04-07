#include "cmd_alias.h"
#include "../../../utils/mem.h"
#include <stdio.h>
#include <string.h>
int cmd_alias(cfd_session_t *sess, int argc, char **argv) {
    if (argc < 2) {
        /* list all */
        size_t n; char **keys = cfd_hash_keys(sess->aliases, &n);
        for (size_t i=0;i<n;i++){
            const char *v=(const char*)cfd_hash_get(sess->aliases,keys[i]);
            printf("alias %s='%s'\n",keys[i],v?v:"");
            cfd_free(keys[i]);
        }
        cfd_free(keys);
        return 0;
    }
    for (int i=1;i<argc;i++){
        char *eq=strchr(argv[i],'=');
        if(eq){
            *eq='\0';
            char *old=(char*)cfd_hash_get(sess->aliases,argv[i]);
            cfd_free(old);
            cfd_hash_set(sess->aliases,argv[i],cfd_strdup(eq+1));
            *eq='=';
        } else {
            const char *v=(const char*)cfd_hash_get(sess->aliases,argv[i]);
            if(v) printf("alias %s='%s'\n",argv[i],v);
        }
    }
    return 0;
}
int cmd_unalias(cfd_session_t *sess, int argc, char **argv) {
    for(int i=1;i<argc;i++){
        char *old=(char*)cfd_hash_get(sess->aliases,argv[i]);
        cfd_free(old);
        cfd_hash_del(sess->aliases,argv[i]);
    }
    return 0;
}
const cfd_command_t builtin_alias = {
    "alias","alias [name[=val]...]","Define or list aliases","system",cmd_alias,0,-1
};
const cfd_command_t builtin_unalias = {
    "unalias","unalias <name...>","Remove aliases","system",cmd_unalias,1,-1
};
