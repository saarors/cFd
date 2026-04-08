#include "registry.h"
/* --- existing --- */
#include "builtin/fs/cmd_ls.h"
#include "builtin/fs/cmd_cd.h"
#include "builtin/fs/cmd_pwd.h"
#include "builtin/fs/cmd_mkdir.h"
#include "builtin/fs/cmd_rmdir.h"
#include "builtin/fs/cmd_rm.h"
#include "builtin/fs/cmd_cp.h"
#include "builtin/fs/cmd_mv.h"
#include "builtin/fs/cmd_cat.h"
#include "builtin/fs/cmd_touch.h"
#include "builtin/fs/cmd_stat.h"
#include "builtin/fs/cmd_find.h"
#include "builtin/text/cmd_echo.h"
#include "builtin/text/cmd_grep.h"
#include "builtin/text/cmd_wc.h"
#include "builtin/text/cmd_sort.h"
#include "builtin/text/cmd_head.h"
#include "builtin/text/cmd_tail.h"
#include "builtin/text/cmd_cut.h"
#include "builtin/text/cmd_tr.h"
#include "builtin/sys/cmd_env.h"
#include "builtin/sys/cmd_set.h"
#include "builtin/sys/cmd_unset.h"
#include "builtin/sys/cmd_date.h"
#include "builtin/sys/cmd_clear.h"
#include "builtin/sys/cmd_exit.h"
#include "builtin/sys/cmd_alias.h"
#include "builtin/sys/cmd_version.h"
#include "builtin/sys/cmd_help.h"
#include "builtin/process/cmd_exec.h"
#include "builtin/process/cmd_ps.h"
#include "builtin/process/cmd_kill.h"
#include "builtin/process/cmd_jobs.h"
#include "builtin/process/cmd_bg.h"
#include "builtin/process/cmd_fg.h"
/* --- new net --- */
#include "builtin/net/cmd_curl.h"
#include "builtin/net/cmd_wget.h"
#include "builtin/net/cmd_ping.h"
#include "builtin/net/cmd_netstat.h"
#include "builtin/net/cmd_ipconfig.h"
/* --- new editor --- */
#include "builtin/editor/cmd_nano.h"
/* --- new pkg --- */
#include "builtin/pkg/cmd_pkg.h"
/* --- new crypto --- */
#include "builtin/crypto/cmd_md5.h"
#include "builtin/crypto/cmd_sha256.h"
/* --- new math --- */
#include "builtin/math/cmd_calc.h"
#include "builtin/math/cmd_seq.h"
#include "builtin/math/cmd_expr.h"
/* --- new sys --- */
#include "builtin/sys/cmd_which.h"
#include "builtin/sys/cmd_type.h"
#include "builtin/sys/cmd_history.h"
#include "builtin/sys/cmd_source.h"
#include "builtin/sys/cmd_export.h"
#include "builtin/sys/cmd_read.h"
#include "builtin/sys/cmd_sleep.h"
#include "builtin/sys/cmd_uname.h"
#include "builtin/sys/cmd_whoami.h"
#include "builtin/sys/cmd_hostname.h"
#include "builtin/sys/cmd_uptime.h"
#include "builtin/sys/cmd_watch.h"
#include "builtin/sys/cmd_test_cmd.h"
/* --- new text --- */
#include "builtin/text/cmd_tee.h"
#include "builtin/text/cmd_uniq.h"
#include "builtin/text/cmd_diff.h"
#include "builtin/text/cmd_base64.h"
#include "builtin/text/cmd_xargs.h"
#include "builtin/text/cmd_column.h"
#include "builtin/text/cmd_fold.h"
/* --- new fs --- */
#include "builtin/fs/cmd_du.h"
#include "builtin/fs/cmd_df.h"
#include "builtin/fs/cmd_ln.h"
#include "builtin/fs/cmd_realpath.h"
#include "builtin/fs/cmd_tree.h"
#include "builtin/fs/cmd_chmod.h"
/* --- new process --- */
#include "builtin/process/cmd_wait.h"
/* --- update --- */
#include "builtin/sys/cmd_update.h"

void cfd_register_all_builtins(cfd_registry_t *reg) {
    /* filesystem */
    cfd_registry_register(reg, &builtin_ls);
    cfd_registry_register(reg, &builtin_cd);
    cfd_registry_register(reg, &builtin_pwd);
    cfd_registry_register(reg, &builtin_mkdir);
    cfd_registry_register(reg, &builtin_rmdir);
    cfd_registry_register(reg, &builtin_rm);
    cfd_registry_register(reg, &builtin_cp);
    cfd_registry_register(reg, &builtin_mv);
    cfd_registry_register(reg, &builtin_cat);
    cfd_registry_register(reg, &builtin_touch);
    cfd_registry_register(reg, &builtin_stat);
    cfd_registry_register(reg, &builtin_find);
    cfd_registry_register(reg, &builtin_du);
    cfd_registry_register(reg, &builtin_df);
    cfd_registry_register(reg, &builtin_ln);
    cfd_registry_register(reg, &builtin_realpath);
    cfd_registry_register(reg, &builtin_tree);
    cfd_registry_register(reg, &builtin_chmod);
    /* text */
    cfd_registry_register(reg, &builtin_echo);
    cfd_registry_register(reg, &builtin_grep);
    cfd_registry_register(reg, &builtin_wc);
    cfd_registry_register(reg, &builtin_sort);
    cfd_registry_register(reg, &builtin_head);
    cfd_registry_register(reg, &builtin_tail);
    cfd_registry_register(reg, &builtin_cut);
    cfd_registry_register(reg, &builtin_tr);
    cfd_registry_register(reg, &builtin_tee);
    cfd_registry_register(reg, &builtin_uniq);
    cfd_registry_register(reg, &builtin_diff);
    cfd_registry_register(reg, &builtin_base64);
    cfd_registry_register(reg, &builtin_xargs);
    cfd_registry_register(reg, &builtin_column);
    cfd_registry_register(reg, &builtin_fold);
    /* system */
    cfd_registry_register(reg, &builtin_env);
    cfd_registry_register(reg, &builtin_set);
    cfd_registry_register(reg, &builtin_unset);
    cfd_registry_register(reg, &builtin_date);
    cfd_registry_register(reg, &builtin_clear);
    cfd_registry_register(reg, &builtin_exit);
    cfd_registry_register(reg, &builtin_alias);
    cfd_registry_register(reg, &builtin_unalias);
    cfd_registry_register(reg, &builtin_version);
    cfd_registry_register(reg, &builtin_help);
    cfd_registry_register(reg, &builtin_which);
    cfd_registry_register(reg, &builtin_type);
    cfd_registry_register(reg, &builtin_history);
    cfd_registry_register(reg, &builtin_source);
    cfd_registry_register(reg, &builtin_export);
    cfd_registry_register(reg, &builtin_read);
    cfd_registry_register(reg, &builtin_sleep);
    cfd_registry_register(reg, &builtin_uname);
    cfd_registry_register(reg, &builtin_whoami);
    cfd_registry_register(reg, &builtin_hostname);
    cfd_registry_register(reg, &builtin_uptime);
    cfd_registry_register(reg, &builtin_watch);
    cfd_registry_register(reg, &builtin_test);
    /* process */
    cfd_registry_register(reg, &builtin_exec);
    cfd_registry_register(reg, &builtin_ps);
    cfd_registry_register(reg, &builtin_kill);
    cfd_registry_register(reg, &builtin_jobs);
    cfd_registry_register(reg, &builtin_bg);
    cfd_registry_register(reg, &builtin_fg);
    cfd_registry_register(reg, &builtin_wait);
    /* net */
    cfd_registry_register(reg, &builtin_curl);
    cfd_registry_register(reg, &builtin_wget);
    cfd_registry_register(reg, &builtin_ping);
    cfd_registry_register(reg, &builtin_netstat);
    cfd_registry_register(reg, &builtin_ipconfig);
    /* editor */
    cfd_registry_register(reg, &builtin_nano);
    /* pkg */
    cfd_registry_register(reg, &builtin_pkg);
    /* crypto */
    cfd_registry_register(reg, &builtin_md5);
    cfd_registry_register(reg, &builtin_sha256);
    /* math */
    cfd_registry_register(reg, &builtin_calc);
    cfd_registry_register(reg, &builtin_seq);
    cfd_registry_register(reg, &builtin_expr);
    /* update */
    cfd_registry_register(reg, &builtin_update);
}
