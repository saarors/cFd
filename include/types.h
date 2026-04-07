#ifndef CFD_TYPES_H
#define CFD_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Basic integer types */
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

/* String type */
typedef char *   cfd_str_t;

/* Error code */
typedef int cfd_err_t;

/* Status codes */
#define CFD_OK          0
#define CFD_ERR        -1
#define CFD_ERR_MEM    -2
#define CFD_ERR_IO     -3
#define CFD_ERR_PARSE  -4
#define CFD_ERR_NOTFOUND -5
#define CFD_ERR_PERM   -6
#define CFD_ERR_EXIST  -7
#define CFD_ERR_NOTEMPTY -8
#define CFD_ERR_INVAL  -9

/* Forward declarations */
typedef struct cfd_terminal   cfd_terminal_t;
typedef struct cfd_session    cfd_session_t;
typedef struct cfd_command    cfd_command_t;
typedef struct cfd_token      cfd_token_t;
typedef struct cfd_ast_node   cfd_ast_node_t;
typedef struct cfd_env        cfd_env_t;
typedef struct cfd_alias      cfd_alias_t;
typedef struct cfd_job        cfd_job_t;
typedef struct cfd_list       cfd_list_t;
typedef struct cfd_list_node  cfd_list_node_t;
typedef struct cfd_hash       cfd_hash_t;
typedef struct cfd_hash_entry cfd_hash_entry_t;
typedef struct cfd_stream     cfd_stream_t;
typedef struct cfd_redirect   cfd_redirect_t;
typedef struct cfd_pipe       cfd_pipe_t;
typedef struct cfd_theme      cfd_theme_t;
typedef struct cfd_config     cfd_config_t;
typedef struct cfd_history    cfd_history_t;
typedef struct cfd_function   cfd_function_t;
typedef struct cfd_variable   cfd_variable_t;

/* Command handler function pointer */
typedef int (*cfd_cmd_fn_t)(cfd_session_t *sess, int argc, char **argv);

/* Completion handler */
typedef char **(*cfd_complete_fn_t)(const char *text, int start, int end);

/* Variable scope */
typedef enum {
    CFD_SCOPE_LOCAL  = 0,
    CFD_SCOPE_GLOBAL = 1,
    CFD_SCOPE_EXPORT = 2
} cfd_scope_t;

/* Redirect type */
typedef enum {
    CFD_REDIR_NONE    = 0,
    CFD_REDIR_IN      = 1,
    CFD_REDIR_OUT     = 2,
    CFD_REDIR_APPEND  = 3,
    CFD_REDIR_ERR     = 4,
    CFD_REDIR_ERR_OUT = 5,
    CFD_REDIR_HEREDOC = 6
} cfd_redir_type_t;

/* Job status */
typedef enum {
    CFD_JOB_RUNNING  = 0,
    CFD_JOB_STOPPED  = 1,
    CFD_JOB_DONE     = 2,
    CFD_JOB_KILLED   = 3
} cfd_job_status_t;

/* Log level */
typedef enum {
    CFD_LOG_DEBUG = 0,
    CFD_LOG_INFO  = 1,
    CFD_LOG_WARN  = 2,
    CFD_LOG_ERROR = 3,
    CFD_LOG_FATAL = 4
} cfd_log_level_t;

#endif /* CFD_TYPES_H */
