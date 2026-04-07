#include "control.h"
#include "../core/session.h"
#include "../utils/mem.h"
#include <string.h>

/* Forward: execute an AST node via the session's executor */
extern int cfd_session_exec_node(cfd_session_t *sess, cfd_ast_node_t *node);

int cfd_ctrl_eval_if(cfd_session_t *sess, cfd_ast_node_t *node, cfd_ctrl_state_t *ctrl) {
    if (!node) return 0;
    int ret = 0;
    if (node->condition) {
        ret = cfd_session_exec_node(sess, node->condition);
        if (ctrl->signal != CFD_CTRL_NORMAL) return ret;
    }
    if (ret == 0 && node->body) {
        ret = cfd_session_exec_node(sess, node->body);
    } else if (ret != 0 && node->else_body) {
        ret = cfd_session_exec_node(sess, node->else_body);
    }
    return ret;
}

int cfd_ctrl_eval_while(cfd_session_t *sess, cfd_ast_node_t *node, cfd_ctrl_state_t *ctrl) {
    if (!node || !node->condition || !node->body) return 0;
    int ret = 0;
    while (true) {
        ret = cfd_session_exec_node(sess, node->condition);
        if (ctrl->signal != CFD_CTRL_NORMAL) break;
        if (ret != 0) break;
        ret = cfd_session_exec_node(sess, node->body);
        if (ctrl->signal == CFD_CTRL_BREAK) { ctrl->signal = CFD_CTRL_NORMAL; break; }
        if (ctrl->signal == CFD_CTRL_CONTINUE) { ctrl->signal = CFD_CTRL_NORMAL; continue; }
        if (ctrl->signal != CFD_CTRL_NORMAL) break;
    }
    return ret;
}

int cfd_ctrl_eval_for(cfd_session_t *sess, cfd_ast_node_t *node, cfd_ctrl_state_t *ctrl) {
    if (!node || !node->var_name || !node->body) return 0;
    int ret = 0;
    for (int i = 0; i < node->var_list_len; i++) {
        /* set variable */
        cfd_session_set_var(sess, node->var_name, node->var_list[i]);
        ret = cfd_session_exec_node(sess, node->body);
        if (ctrl->signal == CFD_CTRL_BREAK) { ctrl->signal = CFD_CTRL_NORMAL; break; }
        if (ctrl->signal == CFD_CTRL_CONTINUE) { ctrl->signal = CFD_CTRL_NORMAL; continue; }
        if (ctrl->signal != CFD_CTRL_NORMAL) break;
    }
    return ret;
}
