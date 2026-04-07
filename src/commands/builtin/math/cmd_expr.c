#include "cmd_expr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#ifndef _WIN32
#  include <regex.h>
#endif

/* ---- expr: evaluates an expression like POSIX expr ---- */

static bool is_integer(const char *s) {
    if (!s || !*s) return false;
    int i = 0;
    if (s[0] == '-' || s[0] == '+') i = 1;
    if (!s[i]) return false;
    while (s[i]) {
        if (!isdigit((unsigned char)s[i])) return false;
        i++;
    }
    return true;
}

/* expr evaluates argv[1..] as an expression */
/* Returns: printed result, exit 0 if non-zero/non-null result, 1 if 0 or null, 2 on error */

typedef struct {
    char **args;
    int    argc;
    int    pos;
} expr_state_t;

static const char *expr_peek(expr_state_t *e) {
    if (e->pos < e->argc) return e->args[e->pos];
    return NULL;
}

static const char *expr_next(expr_state_t *e) {
    if (e->pos < e->argc) return e->args[e->pos++];
    return NULL;
}

/* Forward declaration */
static char *expr_eval(expr_state_t *e);

static char *expr_primary(expr_state_t *e) {
    const char *tok = expr_peek(e);
    if (!tok) return NULL;

    /* Parentheses */
    if (strcmp(tok, "(") == 0) {
        expr_next(e); /* consume '(' */
        char *val = expr_eval(e);
        tok = expr_peek(e);
        if (tok && strcmp(tok, ")") == 0)
            expr_next(e);
        return val;
    }

    /* String operations */
    if (strcmp(tok, "length") == 0) {
        expr_next(e);
        const char *s = expr_next(e);
        if (!s) return strdup("0");
        char buf[32];
        snprintf(buf, sizeof(buf), "%zu", strlen(s));
        return strdup(buf);
    }

    if (strcmp(tok, "substr") == 0) {
        expr_next(e);
        const char *s   = expr_next(e);
        const char *pos_s = expr_next(e);
        const char *len_s = expr_next(e);
        if (!s || !pos_s || !len_s) return strdup("");
        int pos_val = atoi(pos_s) - 1; /* 1-based */
        int len_val = atoi(len_s);
        int slen    = (int)strlen(s);
        if (pos_val < 0) pos_val = 0;
        if (pos_val >= slen) return strdup("");
        if (pos_val + len_val > slen) len_val = slen - pos_val;
        char *out = (char *)malloc((size_t)(len_val + 1));
        if (!out) return strdup("");
        strncpy(out, s + pos_val, (size_t)len_val);
        out[len_val] = '\0';
        return out;
    }

    if (strcmp(tok, "index") == 0) {
        expr_next(e);
        const char *s    = expr_next(e);
        const char *chars = expr_next(e);
        if (!s || !chars) return strdup("0");
        for (int i = 0; s[i]; i++) {
            if (strchr(chars, s[i])) {
                char buf[32];
                snprintf(buf, sizeof(buf), "%d", i + 1);
                return strdup(buf);
            }
        }
        return strdup("0");
    }

    if (strcmp(tok, "match") == 0) {
        expr_next(e);
        const char *s   = expr_next(e);
        const char *pat = expr_next(e);
        if (!s || !pat) return strdup("0");
#ifndef _WIN32
        regex_t re;
        if (regcomp(&re, pat, REG_EXTENDED) != 0) return strdup("0");
        regmatch_t m[1];
        int rc = regexec(&re, s, 1, m, 0);
        regfree(&re);
        if (rc == 0) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%d", (int)(m[0].rm_eo - m[0].rm_so));
            return strdup(buf);
        }
#endif
        return strdup("0");
    }

    /* Plain token */
    expr_next(e);
    return strdup(tok);
}

static char *expr_eval(expr_state_t *e) {
    char *left = expr_primary(e);

    while (true) {
        const char *op = expr_peek(e);
        if (!op) break;

        /* Arithmetic */
        if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0 ||
            strcmp(op, "*") == 0 || strcmp(op, "/") == 0 ||
            strcmp(op, "%") == 0) {

            if (!is_integer(left)) break;
            expr_next(e);
            char *right = expr_primary(e);
            if (!right || !is_integer(right)) { free(right); break; }

            long lv = atol(left);
            long rv = atol(right);
            long res = 0;

            free(left); free(right);

            if      (op[0] == '+') res = lv + rv;
            else if (op[0] == '-') res = lv - rv;
            else if (op[0] == '*') res = lv * rv;
            else if (op[0] == '/') { if (rv == 0) { fprintf(stderr, "expr: division by zero\n"); return strdup("0"); } res = lv / rv; }
            else if (op[0] == '%') { if (rv == 0) { fprintf(stderr, "expr: modulo by zero\n"); return strdup("0"); }  res = lv % rv; }

            char buf[32];
            snprintf(buf, sizeof(buf), "%ld", res);
            left = strdup(buf);
        }
        /* Comparison */
        else if (strcmp(op, "=")  == 0 || strcmp(op, "!=") == 0 ||
                 strcmp(op, "<")  == 0 || strcmp(op, ">")  == 0 ||
                 strcmp(op, "<=") == 0 || strcmp(op, ">=") == 0) {

            expr_next(e);
            char *right = expr_primary(e);
            if (!right) break;

            int cmp;
            if (is_integer(left) && is_integer(right))
                cmp = (atol(left) > atol(right)) - (atol(left) < atol(right));
            else
                cmp = strcmp(left, right);

            bool result = false;
            if      (strcmp(op, "=")  == 0) result = (cmp == 0);
            else if (strcmp(op, "!=") == 0) result = (cmp != 0);
            else if (strcmp(op, "<")  == 0) result = (cmp <  0);
            else if (strcmp(op, ">")  == 0) result = (cmp >  0);
            else if (strcmp(op, "<=") == 0) result = (cmp <= 0);
            else if (strcmp(op, ">=") == 0) result = (cmp >= 0);

            free(left); free(right);
            left = strdup(result ? "1" : "0");
        }
        /* Logical */
        else if (strcmp(op, "&") == 0) {
            expr_next(e);
            char *right = expr_eval(e);
            bool lv = left  && left[0]  != '\0' && strcmp(left,  "0") != 0;
            bool rv = right && right[0] != '\0' && strcmp(right, "0") != 0;
            char *res = strdup((lv && rv) ? left : "0");
            free(left); free(right);
            left = res;
        }
        else if (strcmp(op, "|") == 0) {
            expr_next(e);
            char *right = expr_eval(e);
            bool lv = left  && left[0]  != '\0' && strcmp(left,  "0") != 0;
            char *res = lv ? strdup(left) : strdup(right ? right : "0");
            free(left); free(right);
            left = res;
        }
        else {
            break;
        }
    }

    return left;
}

int cmd_expr(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;

    if (argc < 2) {
        fprintf(stderr, "Usage: expr arg [op arg ...]\n");
        fprintf(stderr, "  Integer ops: + - * / %%\n");
        fprintf(stderr, "  Comparison:  = != < > <= >=\n");
        fprintf(stderr, "  Logical:     & |\n");
        fprintf(stderr, "  String ops:  length str, substr str pos len, index str chars, match str regex\n");
        return 2;
    }

    expr_state_t e;
    e.args = argv + 1;
    e.argc = argc - 1;
    e.pos  = 0;

    char *result = expr_eval(&e);

    if (!result) {
        fprintf(stderr, "expr: syntax error\n");
        return 2;
    }

    printf("%s\n", result);

    int ret = 0;
    if (result[0] == '\0' || strcmp(result, "0") == 0) ret = 1;

    free(result);
    return ret;
}

const cfd_command_t builtin_expr = {
    "expr",
    "expr arg [op arg ...]",
    "Evaluate expressions (arithmetic, string, logical)",
    "math",
    cmd_expr,
    1, -1
};
