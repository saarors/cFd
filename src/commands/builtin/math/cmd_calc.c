#include "cmd_calc.h"
#include "../../../utils/mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <stdbool.h>

/* ---- Recursive descent parser for math expressions ---- */

typedef struct {
    const char *src;
    int         pos;
    char        errbuf[256];
    bool        error;
    double      ans; /* last result */
} calc_parser_t;

static double calc_expr(calc_parser_t *p);

static void calc_skip_ws(calc_parser_t *p) {
    while (p->src[p->pos] == ' ' || p->src[p->pos] == '\t')
        p->pos++;
}

static bool calc_match(calc_parser_t *p, char c) {
    calc_skip_ws(p);
    if (p->src[p->pos] == c) { p->pos++; return true; }
    return false;
}

/* Parse a number or function call or constant */
static double calc_primary(calc_parser_t *p) {
    calc_skip_ws(p);

    /* Unary minus / plus */
    if (p->src[p->pos] == '-') {
        p->pos++;
        return -calc_primary(p);
    }
    if (p->src[p->pos] == '+') {
        p->pos++;
        return calc_primary(p);
    }

    /* Parenthesized expression */
    if (p->src[p->pos] == '(') {
        p->pos++;
        double val = calc_expr(p);
        if (!calc_match(p, ')')) {
            snprintf(p->errbuf, sizeof(p->errbuf), "Expected ')'");
            p->error = true;
        }
        return val;
    }

    /* Number */
    if (isdigit((unsigned char)p->src[p->pos]) || p->src[p->pos] == '.') {
        char *end;
        double val = strtod(p->src + p->pos, &end);
        p->pos = (int)(end - p->src);
        return val;
    }

    /* Identifier: function or constant */
    if (isalpha((unsigned char)p->src[p->pos]) || p->src[p->pos] == '_') {
        char name[64];
        int  nlen = 0;
        while ((isalnum((unsigned char)p->src[p->pos]) || p->src[p->pos] == '_')
               && nlen < 63) {
            name[nlen++] = p->src[p->pos++];
        }
        name[nlen] = '\0';

        /* Constants */
        if (strcmp(name, "pi") == 0) return M_PI;
        if (strcmp(name, "e")  == 0) return M_E;
        if (strcmp(name, "ans") == 0) return p->ans;

        /* Functions */
        calc_skip_ws(p);
        if (p->src[p->pos] == '(') {
            p->pos++;
            double arg = calc_expr(p);
            if (!calc_match(p, ')')) {
                snprintf(p->errbuf, sizeof(p->errbuf), "Expected ')' after function arg");
                p->error = true;
                return 0.0;
            }
            if (strcmp(name, "sin")   == 0) return sin(arg);
            if (strcmp(name, "cos")   == 0) return cos(arg);
            if (strcmp(name, "tan")   == 0) return tan(arg);
            if (strcmp(name, "sqrt")  == 0) return sqrt(arg);
            if (strcmp(name, "abs")   == 0) return fabs(arg);
            if (strcmp(name, "floor") == 0) return floor(arg);
            if (strcmp(name, "ceil")  == 0) return ceil(arg);
            if (strcmp(name, "log")   == 0) return log(arg);
            if (strcmp(name, "log2")  == 0) return log2(arg);
            if (strcmp(name, "log10") == 0) return log10(arg);
            if (strcmp(name, "asin")  == 0) return asin(arg);
            if (strcmp(name, "acos")  == 0) return acos(arg);
            if (strcmp(name, "atan")  == 0) return atan(arg);
            if (strcmp(name, "exp")   == 0) return exp(arg);
            if (strcmp(name, "round") == 0) return round(arg);

            /* pow needs second arg */
            if (strcmp(name, "pow") == 0) {
                /* already consumed first arg; need comma and second */
                if (!calc_match(p, ',')) {
                    /* Try without comma, re-use same arg */
                    return pow(arg, arg);
                }
                double arg2 = calc_expr(p);
                if (!calc_match(p, ')')) {
                    snprintf(p->errbuf, sizeof(p->errbuf), "Expected ')' after pow args");
                    p->error = true;
                    return 0.0;
                }
                return pow(arg, arg2);
            }

            snprintf(p->errbuf, sizeof(p->errbuf), "Unknown function: %s", name);
            p->error = true;
            return 0.0;
        }

        snprintf(p->errbuf, sizeof(p->errbuf), "Unknown identifier: %s", name);
        p->error = true;
        return 0.0;
    }

    snprintf(p->errbuf, sizeof(p->errbuf), "Unexpected character '%c' at pos %d",
             p->src[p->pos], p->pos);
    p->error = true;
    return 0.0;
}

/* Power (right-assoc) */
static double calc_power(calc_parser_t *p) {
    double base = calc_primary(p);
    calc_skip_ws(p);
    if (p->src[p->pos] == '^') {
        p->pos++;
        double exp_val = calc_power(p); /* right-associative */
        return pow(base, exp_val);
    }
    return base;
}

/* Multiply / divide / modulo */
static double calc_term(calc_parser_t *p) {
    double left = calc_power(p);
    for (;;) {
        calc_skip_ws(p);
        char op = p->src[p->pos];
        if (op == '*') {
            p->pos++;
            left *= calc_power(p);
        } else if (op == '/') {
            p->pos++;
            double right = calc_power(p);
            if (right == 0.0) {
                snprintf(p->errbuf, sizeof(p->errbuf), "Division by zero");
                p->error = true;
                return 0.0;
            }
            left /= right;
        } else if (op == '%') {
            p->pos++;
            double right = calc_power(p);
            if (right == 0.0) {
                snprintf(p->errbuf, sizeof(p->errbuf), "Modulo by zero");
                p->error = true;
                return 0.0;
            }
            left = fmod(left, right);
        } else {
            break;
        }
    }
    return left;
}

/* Add / subtract */
static double calc_expr(calc_parser_t *p) {
    double left = calc_term(p);
    for (;;) {
        calc_skip_ws(p);
        char op = p->src[p->pos];
        if (op == '+') {
            p->pos++;
            left += calc_term(p);
        } else if (op == '-') {
            p->pos++;
            left -= calc_term(p);
        } else {
            break;
        }
    }
    return left;
}

/* ---- Last answer persistence (simple static) ---- */
static double g_last_ans = 0.0;

/* ---- Format result (trim trailing zeros) ---- */
static void calc_format_result(double val, char *buf, size_t sz) {
    /* Special cases */
    if (val != val) { snprintf(buf, sz, "nan"); return; }
    if (val == (double)INFINITY)  { snprintf(buf, sz, "inf"); return; }
    if (val == -(double)INFINITY) { snprintf(buf, sz, "-inf"); return; }

    /* Integer check */
    if (val == floor(val) && fabs(val) < 1e15) {
        snprintf(buf, sz, "%.0f", val);
        return;
    }

    /* Use %g with up to 10 significant digits */
    snprintf(buf, sz, "%.10g", val);
}

int cmd_calc(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;

    if (argc < 2) {
        fprintf(stderr, "Usage: calc <expression>\n");
        fprintf(stderr, "  Operators: + - * / %% ^ ( )\n");
        fprintf(stderr, "  Functions: sin cos tan sqrt abs floor ceil log log2 log10 pow exp round asin acos atan\n");
        fprintf(stderr, "  Constants: pi e\n");
        fprintf(stderr, "  Use 'ans' for previous result\n");
        return 1;
    }

    /* Join all arguments into one expression string */
    char expr[4096];
    expr[0] = '\0';
    for (int i = 1; i < argc; i++) {
        if (i > 1) strncat(expr, " ", sizeof(expr) - strlen(expr) - 1);
        strncat(expr, argv[i], sizeof(expr) - strlen(expr) - 1);
    }

    calc_parser_t p;
    memset(&p, 0, sizeof(p));
    p.src = expr;
    p.ans = g_last_ans;

    double result = calc_expr(&p);

    if (p.error) {
        fprintf(stderr, "calc: %s\n", p.errbuf);
        return 1;
    }

    calc_skip_ws(&p);
    if (p.src[p.pos] != '\0') {
        fprintf(stderr, "calc: unexpected token at '%s'\n", p.src + p.pos);
        return 1;
    }

    g_last_ans = result;

    char resbuf[64];
    calc_format_result(result, resbuf, sizeof(resbuf));
    printf("%s\n", resbuf);
    return 0;
}

const cfd_command_t builtin_calc = {
    "calc",
    "calc <expression>",
    "Evaluate a mathematical expression",
    "math",
    cmd_calc,
    1, -1
};
