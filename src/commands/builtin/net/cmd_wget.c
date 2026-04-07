#include "cmd_wget.h"
#include "../../../utils/mem.h"
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

/* ---- URL parsing (duplicated for self-contained file) ---- */
typedef struct {
    char scheme[16];
    char host[256];
    int  port;
    char path[2048];
} wget_url_t;

static int wget_parse_url(const char *url, wget_url_t *out) {
    memset(out, 0, sizeof(*out));
    const char *p = url;
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
    out->port = (strcmp(out->scheme, "https") == 0) ? 443 : 80;

    const char *slash = strchr(p, '/');
    size_t hostlen;
    if (slash) {
        hostlen = (size_t)(slash - p);
        strncpy(out->path, slash, sizeof(out->path) - 1);
    } else {
        hostlen = strlen(p);
        strncpy(out->path, "/", sizeof(out->path) - 1);
    }
    char hostpart[256];
    if (hostlen >= sizeof(hostpart)) hostlen = sizeof(hostpart) - 1;
    strncpy(hostpart, p, hostlen);
    hostpart[hostlen] = '\0';
    const char *portc = strchr(hostpart, ':');
    if (portc) {
        out->port = atoi(portc + 1);
        size_t hl = (size_t)(portc - hostpart);
        strncpy(out->host, hostpart, hl);
        out->host[hl] = '\0';
    } else {
        strncpy(out->host, hostpart, sizeof(out->host) - 1);
    }
    return 0;
}

/* Extract filename from URL path */
static void url_basename(const char *url, char *out, size_t outsz) {
    const char *last = strrchr(url, '/');
    if (last && *(last + 1)) {
        /* strip query string */
        const char *q = strchr(last + 1, '?');
        size_t len = q ? (size_t)(q - last - 1) : strlen(last + 1);
        if (len >= outsz) len = outsz - 1;
        strncpy(out, last + 1, len);
        out[len] = '\0';
    } else {
        strncpy(out, "index.html", outsz - 1);
        out[outsz - 1] = '\0';
    }
}

#ifdef _WIN32

static int wget_download_win(const char *url, const char *outfile, bool quiet) {
    wget_url_t purl;
    if (wget_parse_url(url, &purl) != 0) {
        fprintf(stderr, "wget: failed to parse URL\n");
        return 1;
    }

    HINTERNET hSession = WinHttpOpen(
        L"cFd-wget/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0);
    if (!hSession) { fprintf(stderr, "wget: WinHttpOpen failed\n"); return 1; }

    wchar_t whost[256];
    MultiByteToWideChar(CP_UTF8, 0, purl.host, -1, whost, 256);
    HINTERNET hConnect = WinHttpConnect(hSession, whost, (INTERNET_PORT)purl.port, 0);
    if (!hConnect) {
        fprintf(stderr, "wget: WinHttpConnect failed\n");
        WinHttpCloseHandle(hSession);
        return 1;
    }

    wchar_t wpath[2048];
    MultiByteToWideChar(CP_UTF8, 0, purl.path[0] ? purl.path : "/", -1, wpath, 2048);
    DWORD flags = (purl.port == 443) ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", wpath,
        NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (!hRequest) {
        fprintf(stderr, "wget: WinHttpOpenRequest failed\n");
        WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
        return 1;
    }

    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                            WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        fprintf(stderr, "wget: WinHttpSendRequest failed\n");
        WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 1;
    }

    if (!WinHttpReceiveResponse(hRequest, NULL)) {
        fprintf(stderr, "wget: WinHttpReceiveResponse failed\n");
        WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 1;
    }

    /* Get content-length if available */
    wchar_t cl_buf[64];
    DWORD cl_sz = sizeof(cl_buf);
    long long content_length = -1;
    if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CONTENT_LENGTH,
                            WINHTTP_HEADER_NAME_BY_INDEX,
                            cl_buf, &cl_sz, WINHTTP_NO_HEADER_INDEX)) {
        char tmp[64];
        WideCharToMultiByte(CP_UTF8, 0, cl_buf, -1, tmp, 64, NULL, NULL);
        content_length = atoll(tmp);
    }

    FILE *fp = fopen(outfile, "wb");
    if (!fp) {
        fprintf(stderr, "wget: cannot open output file: %s\n", outfile);
        WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 1;
    }

    if (!quiet)
        fprintf(stderr, "Saving to: '%s'\n", outfile);

    char buf[8192];
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
            fwrite(buf, 1, bytes_read, fp);
            total += bytes_read;
            if (!quiet) {
                if (content_length > 0)
                    fprintf(stderr, "\r%.1f KB / %.1f KB (%.0f%%)",
                        (double)total / 1024.0,
                        (double)content_length / 1024.0,
                        (double)total * 100.0 / (double)content_length);
                else
                    fprintf(stderr, "\r%.1f KB downloaded", (double)total / 1024.0);
            }
        }
    }

    fclose(fp);
    WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    if (!quiet)
        fprintf(stderr, "\n'%s' saved [%zu bytes]\n", outfile, total);

    return 0;
}

#else /* Unix */

static int wget_download_unix(const char *url, const char *outfile, bool quiet) {
    wget_url_t purl;
    if (wget_parse_url(url, &purl) != 0) {
        fprintf(stderr, "wget: failed to parse URL\n");
        return 1;
    }

    if (strcmp(purl.scheme, "https") == 0) {
        fprintf(stderr, "wget: HTTPS not supported without SSL library. Use system wget.\n");
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
        fprintf(stderr, "wget: could not resolve host '%s': %s\n", purl.host, gai_strerror(rc));
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
        fprintf(stderr, "wget: connection to %s:%d failed\n", purl.host, purl.port);
        return 1;
    }

    const char *path = purl.path[0] ? purl.path : "/";
    char req[4096];
    snprintf(req, sizeof(req),
        "GET %s HTTP/1.0\r\n"
        "Host: %s\r\n"
        "Connection: close\r\n"
        "User-Agent: cFd-wget/1.0\r\n\r\n",
        path, purl.host);

    size_t req_len = strlen(req);
    size_t sent = 0;
    while (sent < req_len) {
        ssize_t s = write(sockfd, req + sent, req_len - sent);
        if (s < 0) { close(sockfd); return 1; }
        sent += (size_t)s;
    }

    FILE *fp = fopen(outfile, "wb");
    if (!fp) {
        fprintf(stderr, "wget: cannot open output file: %s\n", outfile);
        close(sockfd);
        return 1;
    }

    if (!quiet)
        fprintf(stderr, "Saving to: '%s'\n", outfile);

    char buf[4096];
    ssize_t n;
    bool headers_done = false;
    char hbuf[8192];
    int hpos = 0;
    size_t total = 0;

    while ((n = read(sockfd, buf, sizeof(buf))) > 0) {
        if (!headers_done) {
            for (ssize_t i = 0; i < n; i++) {
                if (hpos < (int)sizeof(hbuf) - 1)
                    hbuf[hpos++] = buf[i];
                hbuf[hpos] = '\0';
                char *end = strstr(hbuf, "\r\n\r\n");
                if (end) {
                    headers_done = true;
                    char *body_start = end + 4;
                    size_t body_in_hdr = (size_t)(hbuf + hpos - body_start);
                    if (body_in_hdr > 0) {
                        fwrite(body_start, 1, body_in_hdr, fp);
                        total += body_in_hdr;
                    }
                    ssize_t remaining = n - (i + 1);
                    if (remaining > 0) {
                        fwrite(buf + i + 1, 1, (size_t)remaining, fp);
                        total += (size_t)remaining;
                    }
                    break;
                }
            }
        } else {
            fwrite(buf, 1, (size_t)n, fp);
            total += (size_t)n;
            if (!quiet)
                fprintf(stderr, "\r%.1f KB downloaded", (double)total / 1024.0);
        }
    }

    fclose(fp);
    close(sockfd);

    if (!quiet)
        fprintf(stderr, "\n'%s' saved [%zu bytes]\n", outfile, total);

    return 0;
}

#endif /* _WIN32 */

int cmd_wget(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;

    if (argc < 2) {
        fprintf(stderr, "Usage: wget [options] <url>\n");
        fprintf(stderr, "  -O <file>   Save to file (default: from URL)\n");
        fprintf(stderr, "  -q          Quiet mode\n");
        return 1;
    }

    const char *url = NULL;
    const char *outfile = NULL;
    bool quiet = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-O") == 0 && i + 1 < argc) {
            outfile = argv[++i];
        } else if (strcmp(argv[i], "-q") == 0) {
            quiet = true;
        } else if (argv[i][0] != '-') {
            url = argv[i];
        }
    }

    if (!url) {
        fprintf(stderr, "wget: no URL specified\n");
        return 1;
    }

    char auto_name[512];
    if (!outfile) {
        url_basename(url, auto_name, sizeof(auto_name));
        outfile = auto_name;
    }

#ifdef _WIN32
    return wget_download_win(url, outfile, quiet);
#else
    return wget_download_unix(url, outfile, quiet);
#endif
}

const cfd_command_t builtin_wget = {
    "wget",
    "wget [options] <url>",
    "Download files from the web",
    "net",
    cmd_wget,
    1, -1
};
