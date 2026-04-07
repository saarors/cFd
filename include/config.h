#ifndef CFD_CONFIG_H
#define CFD_CONFIG_H

/* Terminal limits */
#define CFD_MAX_INPUT       4096
#define CFD_MAX_ARGS        256
#define CFD_MAX_PATH        4096
#define CFD_MAX_ENV_VARS    1024
#define CFD_MAX_ALIASES     256
#define CFD_MAX_HISTORY     1000
#define CFD_MAX_FUNCTIONS   256
#define CFD_MAX_PIPE_DEPTH  16
#define CFD_MAX_REDIRECTS   16
#define CFD_MAX_LINE        1024
#define CFD_MAX_NAME        256
#define CFD_MAX_ERROR       512
#define CFD_MAX_PROMPT      256
#define CFD_MAX_TOKENS      512
#define CFD_MAX_JOBS        64
#define CFD_MAX_COMPLETION  512

/* Buffer sizes */
#define CFD_BUF_SMALL       256
#define CFD_BUF_MEDIUM      1024
#define CFD_BUF_LARGE       4096
#define CFD_BUF_HUGE        65536

/* Hash table sizes */
#define CFD_HASH_ENV        128
#define CFD_HASH_ALIAS      64
#define CFD_HASH_CMD        128
#define CFD_HASH_FUNC       64

/* Default values */
#define CFD_DEFAULT_PROMPT  "cFd> "
#define CFD_HISTFILE        ".cfd_history"
#define CFD_RCFILE          ".cfdrc"
#define CFD_CONFIG_DIR      ".cfd"
#define CFD_THEME_DEFAULT   "default"

/* Feature flags */
#define CFD_FEATURE_COLOR       1
#define CFD_FEATURE_HISTORY     1
#define CFD_FEATURE_COMPLETION  1
#define CFD_FEATURE_SCRIPTING   1
#define CFD_FEATURE_PIPES       1
#define CFD_FEATURE_REDIRECT    1
#define CFD_FEATURE_JOBS        1
#define CFD_FEATURE_ALIASES     1

/* Exit codes */
#define CFD_EXIT_OK         0
#define CFD_EXIT_ERROR      1
#define CFD_EXIT_USAGE      2
#define CFD_EXIT_NOTFOUND   127

#endif /* CFD_CONFIG_H */
