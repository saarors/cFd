#include "cmd_base64.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

/* ---- Base64 alphabet ---- */
static const char b64_enc[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static int b64_dec_char(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

/* ---- Encode ---- */
static int base64_encode(FILE *in, int wrap_cols) {
    unsigned char buf[3];
    int    pos  = 0; /* current line position */
    size_t n;

    while ((n = fread(buf, 1, 3, in)) > 0) {
        /* Pad if needed */
        if (n < 3) {
            if (n == 1) buf[1] = 0;
            buf[2] = 0;
        }

        char out[4];
        out[0] = b64_enc[(buf[0] >> 2) & 0x3f];
        out[1] = b64_enc[((buf[0] << 4) | (buf[1] >> 4)) & 0x3f];
        out[2] = (n > 1) ? b64_enc[((buf[1] << 2) | (buf[2] >> 6)) & 0x3f] : '=';
        out[3] = (n > 2) ? b64_enc[buf[2] & 0x3f] : '=';

        for (int i = 0; i < 4; i++) {
            if (wrap_cols > 0 && pos >= wrap_cols) {
                putchar('\n');
                pos = 0;
            }
            putchar(out[i]);
            pos++;
        }
    }
    if (pos > 0) putchar('\n');
    return 0;
}

/* ---- Decode ---- */
static int base64_decode(FILE *in) {
    char   quad[4];
    int    q = 0;
    int    c;

    while ((c = fgetc(in)) != EOF) {
        if (c == '\n' || c == '\r' || c == ' ') continue;
        if (c == '=') {
            /* Padding - fill with 'A' equivalent */
            quad[q++] = 'A';
        } else {
            if (b64_dec_char((char)c) < 0) {
                fprintf(stderr, "base64: invalid character '%c'\n", c);
                return 1;
            }
            quad[q++] = (char)c;
        }

        if (q == 4) {
            int v0 = b64_dec_char(quad[0]);
            int v1 = b64_dec_char(quad[1]);
            int v2 = b64_dec_char(quad[2]);
            int v3 = b64_dec_char(quad[3]);

            putchar((char)((v0 << 2) | (v1 >> 4)));
            if (quad[2] != 'A' || quad[3] != 'A') /* not pure padding */
                putchar((char)(((v1 & 0xf) << 4) | (v2 >> 2)));
            if (quad[3] != 'A')
                putchar((char)(((v2 & 0x3) << 6) | v3));

            q = 0;
        }
    }
    return 0;
}

int cmd_base64(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;

    bool decode    = false;
    int  wrap_cols = 76;
    const char *infile = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--decode") == 0) {
            decode = true;
        } else if (strcmp(argv[i], "-w") == 0 && i + 1 < argc) {
            wrap_cols = atoi(argv[++i]);
        } else if (argv[i][0] != '-') {
            infile = argv[i];
        }
    }

    FILE *fp = infile ? fopen(infile, decode ? "r" : "rb") : stdin;
    if (!fp) {
        fprintf(stderr, "base64: %s: No such file or directory\n", infile);
        return 1;
    }

    int ret;
    if (decode)
        ret = base64_decode(fp);
    else
        ret = base64_encode(fp, wrap_cols);

    if (infile) fclose(fp);
    return ret;
}

const cfd_command_t builtin_base64 = {
    "base64",
    "base64 [-d] [-w cols] [file]",
    "Encode or decode data in base64 format",
    "text",
    cmd_base64,
    0, -1
};
