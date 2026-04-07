#ifndef CFD_H
#define CFD_H

#include "types.h"
#include "config.h"
#include "version.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* Core subsystem headers */
#include "../src/utils/mem.h"
#include "../src/utils/str_utils.h"
#include "../src/utils/error.h"
#include "../src/utils/list.h"
#include "../src/utils/hash.h"
#include "../src/utils/path.h"
#include "../src/utils/log.h"

#include "../src/platform/platform.h"

#include "../src/ui/color.h"
#include "../src/ui/theme.h"
#include "../src/ui/display.h"
#include "../src/ui/prompt.h"
#include "../src/ui/ui.h"

#include "../src/parser/token.h"
#include "../src/parser/lexer.h"
#include "../src/parser/ast.h"
#include "../src/parser/parser.h"

#include "../src/io/stream.h"
#include "../src/io/pipe.h"
#include "../src/io/redirect.h"
#include "../src/io/io.h"

#include "../src/input/keybind.h"
#include "../src/input/history.h"
#include "../src/input/completion.h"
#include "../src/input/readline.h"
#include "../src/input/input.h"

#include "../src/scripting/variable.h"
#include "../src/scripting/control.h"
#include "../src/scripting/function.h"
#include "../src/scripting/script.h"

#include "../src/commands/command.h"
#include "../src/commands/registry.h"
#include "../src/commands/dispatch.h"

#include "../src/core/config.h"
#include "../src/core/session.h"
#include "../src/core/repl.h"
#include "../src/core/terminal.h"

#endif /* CFD_H */
