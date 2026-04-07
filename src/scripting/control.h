#ifndef CFD_CONTROL_H
#define CFD_CONTROL_H

#include "../../include/types.h"
#include "../parser/ast.h"

/* Control flow signals */
typedef enum {
    CFD_CTRL_NORMAL  = 0,
    CFD_CTRL_BREAK   = 1,
    CFD_CTRL_CONTINUE= 2,
    CFD_CTRL_RETURN  = 3,
    CFD_CTRL_EXIT    = 4
} cfd_ctrl_signal_t;

typedef struct cfd_ctrl_state {
    cfd_ctrl_signal_t signal;
    int               return_val;
} cfd_ctrl_state_t;

/* Evaluate if-then-elif-else */
int cfd_ctrl_eval_if(cfd_session_t *sess, cfd_ast_node_t *node, cfd_ctrl_state_t *ctrl);

/* Evaluate while loop */
int cfd_ctrl_eval_while(cfd_session_t *sess, cfd_ast_node_t *node, cfd_ctrl_state_t *ctrl);

/* Evaluate for loop */
int cfd_ctrl_eval_for(cfd_session_t *sess, cfd_ast_node_t *node, cfd_ctrl_state_t *ctrl);

#endif /* CFD_CONTROL_H */
