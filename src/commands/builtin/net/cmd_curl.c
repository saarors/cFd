#include "cmd_curl.h"
#include "../../../utils/mem.h"
#include "../../../utils/str_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#ifdef _WIN32
#  include <windows.h>
#  include <winhttp.h>
#else
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <netdb.h>
#  include <unistd.h>
#  include <errno.h>
#endif

/* ---- URL parsing ---- */
typedef struct {
    char scheme[16];
    char host[256];
    int  port;
    char path[2048];
} parsed_url_t;

static int parse_url(const char *url, parsed_url_t *out) {
    memset(out, 0, sizeof(*out));
    const char *p = url;

    /* scheme */
    const char *colon = strstr(p, "://");
    if (!colon) {
        strncpy(out->scheme, "http", 15);
    } else {
        size_t slen = (size_t)(colon - p);
        if (slen >= sizeof(out->scheme)) slen = sizeof(out->scheme) - 1;
        strncpy(out->scheme, p, slen);
        out->scheme[slen] = '\0';
        p = colon + 3;
    }

    /* default port */
    if (strcmp(out->scheme, "https") == 0)
        out->port = 443;
    else
        out->port = 80;

    /* host[:port] */
    const char *slash = strchr(p, '/');
    const char *portcolon;
    size_t hostlen;
    if (slash) {
        hostlen = (size_t)(slash - p);
        strncpy(out->path, slash, sizeof(out->path) - 1);
    } else {
        hostlen = strlen(p);
        strncpy(out->path, "/", sizeof(out->path) - 1);
    }

    /* check for port in host */
    char hostpart[256];
    if (hostlen >= sizeof(hostpart)) hostlen = sizeof(hostpart) - 1;
    strncpy(hostpart, p, hostlen);
    hostpart[hostlen] = '\0';
    portcolon = strchr(hostpart, ':');
    if (portcolon) {
        out->port = atoi(portcolon + 1);
        size_t hl = (size_t)(portcolon - hostpart);
        strncpy(out->host, hostpart, hl);
        out->host[hl] = '\0';
    } else {
        strncpy(out->host, hostpart, sizeof(out->host) - 1);
    }

    return 0;
}

/* ---- curl options ---- */
typedef struct {
    const char *url;
    const char *output_file;
    const char *method;      /* GET, POST, etc. */
    const char *data;
    char       *headers[32];
    int         header_count;
    bool        silent;
    bool        verbose;
    bool        follow_redirects;
    bool        save_to_file;  /* used by wget mode */
} curl_opts_t;

/* ---- Windows implementation ---- */
#ifdef _WIN32

static int do_request_win(const curl_opts_t *opts, FILE *out_fp, bool quiet) {
    parsed_url_t purl;
    if (parse_url(opts->url, &purl) != 0) {
        fprintf(stderr, "curl: failed to parse URL: %s\n", opts->url);
        return 1;
    }

    HINTERNET hSession = WinHttpOpen(
        L"cFd/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0);
    if (!hSession) {
        fprintf(stderr, "curl: WinHttpOpen failed (%lu)\n", GetLastError());
        return 1;
    }

    /* Convert host to wide */
    wchar_t whost[256];
    MultiByteToWideChar(CP_UTF8, 0, purl.host, -1, whost, 256);

    HINTERNET hConnect = WinHttpConnect(hSession, whost, (INTERNET_PORT)purl.port, 0);
    if (!hConnect) {
        fprintf(stderr, "curl: WinHttpConnect failed (%lu)\n", GetLastError());
        WinHttpCloseHandle(hSession);
        return 1;
    }

    wchar_t wpath[2048];
    MultiByteToWideChar(CP_UTF8, 0, purl.path[0] ? purl.path : "/", -1, wpath, 2048);

    const char *method = opts->method ? opts->method : "GET";
    wchar_t wmethod[16];
    MultiByteToWideChar(CP_UTF8, 0, method, -1, wmethod, 16);

    DWORD flags = (purl.port == 443) ? WINHTTP_FLAG_SECURE : 0;
    if (opts->follow_redirects) flags |= WINHTTP_FLAG_REFRESH;

    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect, wmethod, wpath,
        NULL, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        flags);
    if (!hRequest) {
        fprintf(stderr, "curl: WinHttpOpenRequest failed (%lu)\n", GetLastError());
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 1;
    }

    /* Add custom headers */
    for (int i = 0; i < opts->header_count; i++) {
        wchar_t wheader[512];
        MultiByteToWideChar(CP_UTF8, 0, opts->headers[i], -1, wheader, 512);
        WinHttpAddRequestHeaders(hRequest, wheader, (DWORD)-1,
            WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE);
    }

    LPVOID send_buf = NULL;
    DWORD  send_len = 0;
    if (opts->data) {
        send_buf = (LPVOID)opts->data;
        send_len = (DWORD)strlen(opts->data);
    }

    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                            send_buf, send_len, send_len, 0)) {
        fprintf(stderr, "curl: WinHttpSendRequest failed (%lu)\n", GetLastError());
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 1;
    }

    if (!WinHttpReceiveResponse(hRequest, NULL)) {
        fprintf(stderr, "curl: WinHttpReceiveResponse failed (%lu)\n", GetLastError());
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 1;
    }

    /* Print status if verbose */
    if (opts->verbose && !quiet) {
        DWORD status = 0;
        DWORD sz = sizeof(DWORD);
        WinHttpQueryHeaders(hRequest,
            WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX,
            &status, &sz, WINHTTP_NO_HEADER_INDEX);
        fprintf(stderr, "< HTTP/1.1 %lu\n", status);
    }

    /* Read response */
    char buf[4096];
    DWORD bytes_read = 0;
    size_t total = 0;
    BOOL ok = TRUE;

    while (ok) {
        DWORD avail = 0;
        ok = WinHttpQueryDataAvailable(hRequest, &avail);
        if (!ok || avail == 0) break;
        if (avail > sizeof(buf)) avail = sizeof(buf);
        ok = WinHttpReadData(hRequest, buf, avail, &bytes_read);
        if (ok && bytes_read > 0) {
            fwrite(buf, 1, bytes_read, out_fp);
            total += bytes_read;
        }
    }

    if (!quiet && opts->verbose)
        fprintf(stderr, "\n* Downloaded %zu bytes\n", total);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return 0;
}

#else /* Unix */

static int do_request_unix(const curl_opts_t *opts, FILE *out_fp, bool quiet) {
    parsed_url_t purl;
    if (parse_url(opts->url, &purl) != 0) {
        fprintf(stderr, "curl: failed to parse URL: %s\n", opts->url);
        return 1;
    }

    if (strcmp(purl.scheme, "https") == 0) {
        fprintf(stderr, "curl: HTTPS not supported without WinHTTP (Unix plain sockets). Use system curl.\n");
        return 1;
    }

    char port_str[16];
    snprintf(port_str, sizeof(port_str), "%d", purl.port);

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int rc = getaddrinfo(purl.host, port_str, &hints, &res);
    if (rc != 0) {
        fprintf(stderr, "curl: could not resolve host '%s': %s\n", purl.host, gai_strerror(rc));
        return 1;
    }

    int sockfd = -1;
    for (struct addrinfo *ai = res; ai; ai = ai->ai_next) {
        sockfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (sockfd < 0) continue;
        if (connect(sockfd, ai->ai_addr, ai->ai_addrlen) == 0) break;
        close(sockfd);
        sockfd = -1;
    }
    freeaddrinfo(res);

    if (sockfd < 0) {
        fprintf(stderr, "curl: connection to %s:%d failed\n", purl.host, purl.port);
        return 1;
    }

    /* Build HTTP request */
    const char *method = opts->method ? opts->method : "GET";
    const char *path   = purl.path[0] ? purl.path : "/";
    char req[8192];
    int rlen = snprintf(req, sizeof(req),
        "%s %s HTTP/1.0\r\n"
        "Host: %s\r\n"
        "Connection: close\r\n"
        "User-Agent: cFd/1.0\r\n",
        method, path, purl.host);

    for (int i = 0; i < opts->header_count && rlen < (int)sizeof(req) - 4; i++) {
        rlen += snprintf(req + rlen, sizeof(req) - (size_t)rlen, "%s\r\n", opts->headers[i]);
    }

    if (opts->data) {
        rlen += snprintf(req + rlen, sizeof(req) - (size_t)rlen,
            "Content-Length: %zu\r\n\r\n%s", strlen(opts->data), opts->data);
    } else {
        strncat(req + rlen, "\r\n", sizeof(req) - (size_t)rlen - 1);
    }

    if (opts->verbose && !quiet)
        fprintf(stderr, "> %s %s HTTP/1.0\n", method, path);

    /* Send */
    size_t total_sent = 0;
    size_t req_len = strlen(req);
    while (total_sent < req_len) {
        ssize_t sent = write(sockfd, req + total_sent, req_len - total_sent);
        if (sent < 0) {
            fprintf(stderr, "curl: send error: %s\n", strerror(errno));
            close(sockfd);
            return 1;
        }
        total_sent += (size_t)sent;
    }

    /* Receive: skip headers */
    char buf[4096];
    ssize_t n;
    bool headers_done = false;
    char header_buf[8192];
    int  hbuf_pos = 0;
    size_t total_body = 0;

    while ((n = read(sockfd, buf, sizeof(buf))) > 0) {
        if (!headers_done) {
            for (ssize_t i = 0; i < n; i++) {
                if (hbuf_pos < (int)sizeof(header_buf) - 1)
                    header_buf[hbuf_pos++] = buf[i];
                header_buf[hbuf_pos] = '\0';
                char *end = strstr(header_buf, "\r\n\r\n");
                if (end) {
                    headers_done = true;
                    if (opts->verbose && !quiet)
                        fprintf(stderr, "< %.*s\n", (int)(end - header_buf), header_buf);
                    /* write rest of this chunk */
                    size_t skip = (size_t)(end - header_buf) + 4;
                    size_t rest = (size_t)n - (skip - (size_t)(hbuf_pos - i - 1));
                    /* simpler: write from end+4 */
                    char *body_start = end + 4;
                    size_t body_in_hdr = (size_t)(hbuf_pos - (int)(body_start - header_buf));
                    if (body_start >= header_buf && body_in_hdr > 0) {
                        fwrite(body_start, 1, body_in_hdr, out_fp);
                        total_body += body_in_hdr;
                    }
                    /* write rest of buf after i */
                    ssize_t remaining = n - (i + 1);
                    if (remaining > 0) {
                        fwrite(buf + i + 1, 1, (size_t)remaining, out_fp);
                        total_body += (size_t)remaining;
                    }
                    break;
                }
            }
        } else {
            fwrite(buf, 1, (size_t)n, out_fp);
            total_body += (size_t)n;
        }
    }

    close(sockfd);

    if (!quiet && opts->verbose)
        fprintf(stderr, "\n* Downloaded %zu bytes\n", total_body);

    return 0;
}

#endif /* _WIN32 */

int cmd_curl(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;

    if (argc < 2) {
        fprintf(stderr, "Usage: curl [options] <url>\n");
        fprintf(stderr, "  -o <file>       Write output to file\n");
        fprintf(stderr, "  -X <method>     HTTP method (GET, POST, etc.)\n");
        fprintf(stderr, "  -d <data>       POST data\n");
        fprintf(stderr, "  -H \"Hdr: val\"  Add header\n");
        fprintf(stderr, "  -s              Silent\n");
        fprintf(stderr, "  -v              Verbose\n");
        fprintf(stderr, "  -L              Follow redirects\n");
        return 1;
    }

    curl_opts_t opts;
    memset(&opts, 0, sizeof(opts));

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            opts.output_file = argv[++i];
        } else if (strcmp(argv[i], "-X") == 0 && i + 1 < argc) {
            opts.method = argv[++i];
        } else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            opts.data = argv[++i];
            if (!opts.method) opts.method = "POST";
        } else if (strcmp(argv[i], "-H") == 0 && i + 1 < argc) {
            if (opts.header_count < 32)
                opts.headers[opts.header_count++] = argv[++i];
        } else if (strcmp(argv[i], "-s") == 0) {
            opts.silent = true;
        } else if (strcmp(argv[i], "-v") == 0) {
            opts.verbose = true;
        } else if (strcmp(argv[i], "-L") == 0) {
            opts.follow_redirects = true;
        } else if (argv[i][0] != '-') {
            opts.url = argv[i];
        }
    }

    if (!opts.url) {
        fprintf(stderr, "curl: no URL specified\n");
        return 1;
    }

    FILE *out_fp = stdout;
    if (opts.output_file) {
        out_fp = fopen(opts.output_file, "wb");
        if (!out_fp) {
            fprintf(stderr, "curl: cannot open output file: %s\n", opts.output_file);
            return 1;
        }
    }

    int ret;
#ifdef _WIN32
    ret = do_request_win(&opts, out_fp, opts.silent);
#else
    ret = do_request_unix(&opts, out_fp, opts.silent);
#endif

    if (opts.output_file && out_fp) {
        fclose(out_fp);
        if (!opts.silent)
            fprintf(stderr, "Saved to: %s\n", opts.output_file);
    }

    return ret;
}

const cfd_command_t builtin_curl = {
    "curl",
    "curl [options] <url>",
    "HTTP client - transfer data from URLs",
    "net",
    cmd_curl,
    1, -1
};
