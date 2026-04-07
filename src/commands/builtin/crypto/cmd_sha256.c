#include "cmd_sha256.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* ---- SHA-256 implementation (FIPS 180-4) ---- */

typedef struct {
    uint32_t state[8];
    uint32_t count[2]; /* bit count, low and high */
    uint8_t  buf[64];
} sha256_ctx_t;

static const uint32_t sha256_K[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,
    0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,
    0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,
    0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,
    0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,
    0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,
    0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,
    0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,
    0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

#define SHA256_ROTR(x,n)  (((x) >> (n)) | ((x) << (32 - (n))))
#define SHA256_CH(x,y,z)  (((x) & (y)) ^ (~(x) & (z)))
#define SHA256_MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define SHA256_EP0(x)     (SHA256_ROTR(x,2)  ^ SHA256_ROTR(x,13) ^ SHA256_ROTR(x,22))
#define SHA256_EP1(x)     (SHA256_ROTR(x,6)  ^ SHA256_ROTR(x,11) ^ SHA256_ROTR(x,25))
#define SHA256_SIG0(x)    (SHA256_ROTR(x,7)  ^ SHA256_ROTR(x,18) ^ ((x) >> 3))
#define SHA256_SIG1(x)    (SHA256_ROTR(x,17) ^ SHA256_ROTR(x,19) ^ ((x) >> 10))

static void sha256_transform(uint32_t state[8], const uint8_t data[64]) {
    uint32_t a, b, c, d, e, f, g, h, t1, t2, w[64];

    for (int i = 0; i < 16; i++) {
        w[i] = ((uint32_t)data[i*4]     << 24) |
               ((uint32_t)data[i*4 + 1] << 16) |
               ((uint32_t)data[i*4 + 2] <<  8) |
               ((uint32_t)data[i*4 + 3]);
    }
    for (int i = 16; i < 64; i++) {
        w[i] = SHA256_SIG1(w[i-2]) + w[i-7] + SHA256_SIG0(w[i-15]) + w[i-16];
    }

    a = state[0]; b = state[1]; c = state[2]; d = state[3];
    e = state[4]; f = state[5]; g = state[6]; h = state[7];

    for (int i = 0; i < 64; i++) {
        t1 = h + SHA256_EP1(e) + SHA256_CH(e,f,g) + sha256_K[i] + w[i];
        t2 = SHA256_EP0(a) + SHA256_MAJ(a,b,c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }

    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

static void sha256_init(sha256_ctx_t *ctx) {
    ctx->count[0] = ctx->count[1] = 0;
    ctx->state[0] = 0x6a09e667u;
    ctx->state[1] = 0xbb67ae85u;
    ctx->state[2] = 0x3c6ef372u;
    ctx->state[3] = 0xa54ff53au;
    ctx->state[4] = 0x510e527fu;
    ctx->state[5] = 0x9b05688cu;
    ctx->state[6] = 0x1f83d9abu;
    ctx->state[7] = 0x5be0cd19u;
}

static void sha256_update(sha256_ctx_t *ctx, const uint8_t *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        ctx->buf[ctx->count[0] / 8 % 64] = data[i];
        ctx->count[0] += 8;
        if (ctx->count[0] == 0) ctx->count[1]++;
        if ((ctx->count[0] / 8 % 64) == 0)
            sha256_transform(ctx->state, ctx->buf);
    }
}

static void sha256_final(sha256_ctx_t *ctx, uint8_t digest[32]) {
    uint32_t i = ctx->count[0] / 8 % 64;

    /* Append 0x80 */
    ctx->buf[i++] = 0x80;

    /* Pad to 56 bytes */
    if (i > 56) {
        while (i < 64) ctx->buf[i++] = 0x00;
        sha256_transform(ctx->state, ctx->buf);
        i = 0;
    }
    while (i < 56) ctx->buf[i++] = 0x00;

    /* Append bit length (big-endian 64-bit) */
    uint64_t bit_len = (uint64_t)ctx->count[1] * 0x100000000LL + ctx->count[0];
    for (int j = 7; j >= 0; j--) {
        ctx->buf[56 + (7 - j)] = (uint8_t)(bit_len >> (j * 8));
    }
    sha256_transform(ctx->state, ctx->buf);

    /* Output */
    for (int j = 0; j < 8; j++) {
        digest[j*4]   = (uint8_t)(ctx->state[j] >> 24);
        digest[j*4+1] = (uint8_t)(ctx->state[j] >> 16);
        digest[j*4+2] = (uint8_t)(ctx->state[j] >>  8);
        digest[j*4+3] = (uint8_t)(ctx->state[j]);
    }
}

/* ---- File / string helpers ---- */
static int sha256_file(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "sha256: %s: No such file or directory\n", filename);
        return 1;
    }

    sha256_ctx_t ctx;
    sha256_init(&ctx);

    uint8_t buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0)
        sha256_update(&ctx, buf, n);
    fclose(fp);

    uint8_t digest[32];
    sha256_final(&ctx, digest);

    printf("SHA256 (%s) = ", filename);
    for (int i = 0; i < 32; i++) printf("%02x", digest[i]);
    putchar('\n');
    return 0;
}

static void sha256_string(const char *label, const char *s) {
    sha256_ctx_t ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, (const uint8_t *)s, strlen(s));
    uint8_t digest[32];
    sha256_final(&ctx, digest);
    printf("SHA256 (%s) = ", label);
    for (int i = 0; i < 32; i++) printf("%02x", digest[i]);
    putchar('\n');
}

int cmd_sha256(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;

    if (argc < 2) {
        uint8_t buf[4096];
        sha256_ctx_t ctx;
        sha256_init(&ctx);
        size_t n;
        while ((n = fread(buf, 1, sizeof(buf), stdin)) > 0)
            sha256_update(&ctx, buf, n);
        uint8_t digest[32];
        sha256_final(&ctx, digest);
        printf("SHA256 (stdin) = ");
        for (int i = 0; i < 32; i++) printf("%02x", digest[i]);
        putchar('\n');
        return 0;
    }

    int ret = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            const char *s = argv[++i];
            sha256_string(s, s);
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "sha256: unknown option: %s\n", argv[i]);
            ret = 1;
        } else {
            ret |= sha256_file(argv[i]);
        }
    }
    return ret;
}

const cfd_command_t builtin_sha256 = {
    "sha256",
    "sha256 [-s string] [file...]",
    "Compute SHA-256 message digest",
    "crypto",
    cmd_sha256,
    0, -1
};
