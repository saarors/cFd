#include <stdio.h>
#include <string.h>
#include "../src/utils/str_utils.h"
#include "../src/utils/hash.h"
#include "../src/utils/list.h"
#include "../src/utils/mem.h"

static int g_pass = 0, g_fail = 0;

static void check(const char *label, int cond) {
    if (cond) { g_pass++; printf("  PASS: %s\n", label); }
    else       { g_fail++; printf("  FAIL: %s\n", label); }
}

void run_utils_tests(void) {
    /* String utils */
    {
        char s[] = "  hello  ";
        check("trim", strcmp(cfd_strtrim(s),"hello")==0);
    }
    {
        check("starts_with", cfd_starts_with("hello world","hello"));
        check("ends_with",   cfd_ends_with("hello world","world"));
        check("not starts",  !cfd_starts_with("hello","world"));
    }
    {
        int n;
        char **parts = cfd_strsplit("a:b:c",":",  &n);
        check("split count", n==3);
        check("split[0]",    strcmp(parts[0],"a")==0);
        check("split[2]",    strcmp(parts[2],"c")==0);
        cfd_strfreev(parts);
    }
    {
        char *j = cfd_strjoin((char*[]){"x","y","z"}, 3, "-");
        check("join", strcmp(j,"x-y-z")==0);
        cfd_free(j);
    }
    /* Hash table */
    {
        cfd_hash_t *h = cfd_hash_new(8, NULL);
        cfd_hash_set(h,"key","val");
        check("hash set/get", strcmp((char*)cfd_hash_get(h,"key"),"val")==0);
        check("hash has",     cfd_hash_has(h,"key"));
        check("hash no key",  !cfd_hash_has(h,"nope"));
        cfd_hash_del(h,"key");
        check("hash del",     !cfd_hash_has(h,"key"));
        cfd_hash_free(h);
    }
    /* Linked list */
    {
        cfd_list_t *l = cfd_list_new(NULL);
        cfd_list_push_back(l,(void*)"a");
        cfd_list_push_back(l,(void*)"b");
        cfd_list_push_back(l,(void*)"c");
        check("list size",  cfd_list_size(l)==3);
        check("list get 1", strcmp((char*)cfd_list_get(l,1),"b")==0);
        void *popped = cfd_list_pop_front(l);
        check("pop front",  strcmp((char*)popped,"a")==0);
        check("size after pop", cfd_list_size(l)==2);
        cfd_list_free(l);
    }
}
