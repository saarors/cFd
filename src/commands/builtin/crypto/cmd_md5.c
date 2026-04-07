#include "cmd_md5.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* ---- MD5 implementation (RFC 1321) ---- */

typedef struct {
    uint32_t state[4];
    uint32_t count[2];
    uint8_t  buf[64];
} md5_ctx_t;

/* Per-round shift amounts */
static const uint32_t md5_s[64] = {
    7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
    5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,
    4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
    6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21
};

/* Precomputed T table (floor(abs(sin(i+1)) * 2^32)) */
static const uint32_t md5_T[64] = {
    0xd76aa478,0xe8c7b756,0x242070db,0xc1bdceee,
    0xf57c0faf,0x4787c62a,0xa8304613,0xfd469501,
    0x698098d8,0x8b44f7af,0xffff5bb1,0x895cd7be,
    0x6b901122,0xfd987193,0xa679438e,0x49b40821,
    0xf61e2562,0xc040b340,0x265e5a51,0xe9b6c7aa,
    0xd62f105d,0x02441453,0xd8a1e681,0xe7d3fbc8,
    0x21e1cde6,0xc33707d6,0xf4d50d87,0x455a14ed,
    0xa9e3e905,0xfcefa3f8,0x676f02d9,0x8d2a4c8a,
    0xfffa3942,0x8771f681,0x6d9d6122,0xfde5380c,
    0xa4beea44,0x4bdecfa9,0xf6bb4b60,0xbebfbc70,
    0x289b7ec6,0xeaa127fa,0xd4ef3085,0x04881d05,
    0xd9d4d039,0xe6db99e5,0x1fa27cf8,0xc4ac5665,
    0xf4292244,0x432aff97,0xab9423a7,0xfc93a039,
    0x655b59c3,0x8f0ccc92,0xffeff47d,0x85845dd1,
    0x6fa87e4f,0xfe2ce6e0,0xa3014314,0x4e0811a1,
    0xf7537e82,0xbd3af235,0x2ad7d2bb,0xeb86d391
};

#define MD5_LEFTROTATE(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

static void md5_transform(uint32_t state[4], const uint8_t block[64]) {
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint32_t M[16];

    for (int i = 0; i < 16; i++) {
        M[i] = ((uint32_t)block[i*4])        |
               ((uint32_t)block[i*4+1] << 8)  |
               ((uint32_t)block[i*4+2] << 16) |
               ((uint32_t)block[i*4+3] << 24);
    }

    for (int i = 0; i < 64; i++) {
        uint32_t F, g;
        if (i < 16) {
            F = (b & c) | (~b & d);
            g = (uint32_t)i;
        } else if (i < 32) {
            F = (d & b) | (~d & c);
            g = (5u * (uint32_t)i + 1u) % 16u;
        } else if (i < 48) {
            F = b ^ c ^ d;
            g = (3u * (uint32_t)i + 5u) % 16u;
        } else {
            F = c ^ (b | ~d);
            g = (7u * (uint32_t)i) % 16u;
        }
        F = F + a + md5_T[i] + M[g];
        a = d;
        d = c;
        c = b;
        b = b + MD5_LEFTROTATE(F, md5_s[i]);
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
}

static void md5_init(md5_ctx_t *ctx) {
    ctx->count[0] = ctx->count[1] = 0;
    ctx->state[0] = 0x67452301u;
    ctx->state[1] = 0xefcdab89u;
    ctx->state[2] = 0x98badcfeu;
    ctx->state[3] = 0x10325476u;
}

static void md5_update(md5_ctx_t *ctx, const uint8_t *data, size_t len) {
    uint32_t index = (ctx->count[0] >> 3) & 0x3f;

    ctx->count[0] += (uint32_t)(len << 3);
    if (ctx->count[0] < (uint32_t)(len << 3)) ctx->count[1]++;
    ctx->count[1] += (uint32_t)(len >> 29);

    uint32_t part_len = 64 - index;
    size_t i;

    if (len >= part_len) {
        memcpy(&ctx->buf[index], data, part_len);
        md5_transform(ctx->state, ctx->buf);
        for (i = part_len; i + 63 < len; i += 64)
            md5_transform(ctx->state, data + i);
        index = 0;
    } else {
        i = 0;
    }

    memcpy(&ctx->buf[index], data + i, len - i);
}

static void md5_final(md5_ctx_t *ctx, uint8_t digest[16]) {
    static const uint8_t padding[64] = {
        0x80, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    };

    uint8_t bits[8];
    for (int i = 0; i < 8; i++)
        bits[i] = (uint8_t)((ctx->count[i/4] >> ((i % 4) * 8)) & 0xff);

    uint32_t index = (ctx->count[0] >> 3) & 0x3f;
    uint32_t pad_len = (index < 56) ? (56 - index) : (120 - index);
    md5_update(ctx, padding, pad_len);
    md5_update(ctx, bits, 8);

    for (int i = 0; i < 4; i++) {
        digest[i*4]   = (uint8_t)(ctx->state[i]);
        digest[i*4+1] = (uint8_t)(ctx->state[i] >> 8);
        digest[i*4+2] = (uint8_t)(ctx->state[i] >> 16);
        digest[i*4+3] = (uint8_t)(ctx->state[i] >> 24);
    }
}

/* ---- Compute and print ---- */
static void md5_print(const char *label, const uint8_t *data, size_t len) {
    md5_ctx_t ctx;
    uint8_t   digest[16];

    md5_init(&ctx);
    md5_update(&ctx, data, len);
    md5_final(&ctx, digest);

    printf("MD5 (%s) = ", label);
    for (int i = 0; i < 16; i++)
        printf("%02x", digest[i]);
    putchar('\n');
}

static int md5_file(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "md5: %s: No such file or directory\n", filename);
        return 1;
    }

    md5_ctx_t ctx;
    md5_init(&ctx);

    uint8_t buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0)
        md5_update(&ctx, buf, n);
    fclose(fp);

    uint8_t digest[16];
    md5_final(&ctx, digest);

    printf("MD5 (%s) = ", filename);
    for (int i = 0; i < 16; i++) printf("%02x", digest[i]);
    putchar('\n');
    return 0;
}

int cmd_md5(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;

    if (argc < 2) {
        /* Read from stdin */
        uint8_t buf[4096];
        md5_ctx_t ctx;
        md5_init(&ctx);
        size_t n;
        while ((n = fread(buf, 1, sizeof(buf), stdin)) > 0)
            md5_update(&ctx, buf, n);
        uint8_t digest[16];
        md5_final(&ctx, digest);
        printf("MD5 (stdin) = ");
        for (int i = 0; i < 16; i++) printf("%02x", digest[i]);
        putchar('\n');
        return 0;
    }

    int ret = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            const char *s = argv[++i];
            md5_print(s, (const uint8_t *)s, strlen(s));
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "md5: unknown option: %s\n", argv[i]);
            ret = 1;
        } else {
            ret |= md5_file(argv[i]);
        }
    }
    return ret;
}

const cfd_command_t builtin_md5 = {
    "md5",
    "md5 [-s string] [file...]",
    "Compute MD5 message digest",
    "crypto",
    cmd_md5,
    0, -1
};
