# cFd Makefile
# Targets: all, clean, tests, install

CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -Wpedantic -g \
          -Iinclude -Isrc \
          -D_POSIX_C_SOURCE=200809L
LDFLAGS =

# Windows detection
ifeq ($(OS),Windows_NT)
  EXE     = .exe
  CFLAGS += -D_WIN32
  LDFLAGS +=
else
  EXE     =
  LDFLAGS += -lm
endif

TARGET  = cfd$(EXE)
TEST_TARGET = cfd_tests$(EXE)

# -------------------------------------------------------
# Source files
# -------------------------------------------------------

SRC_UTILS = \
  src/utils/mem.c \
  src/utils/str_utils.c \
  src/utils/error.c \
  src/utils/list.c \
  src/utils/hash.c \
  src/utils/path.c \
  src/utils/log.c

SRC_PLATFORM = \
  src/platform/common.c \
  src/platform/win32.c \
  src/platform/unix.c

SRC_UI = \
  src/ui/color.c \
  src/ui/theme.c \
  src/ui/display.c \
  src/ui/prompt.c \
  src/ui/ui.c

SRC_PARSER = \
  src/parser/token.c \
  src/parser/lexer.c \
  src/parser/ast.c \
  src/parser/parser.c

SRC_INPUT = \
  src/input/keybind.c \
  src/input/history.c \
  src/input/completion.c \
  src/input/readline.c \
  src/input/input.c

SRC_IO = \
  src/io/stream.c \
  src/io/pipe.c \
  src/io/redirect.c \
  src/io/io.c

SRC_SCRIPTING = \
  src/scripting/variable.c \
  src/scripting/control.c \
  src/scripting/function.c \
  src/scripting/script.c

SRC_COMMANDS = \
  src/commands/command.c \
  src/commands/registry.c \
  src/commands/dispatch.c \
  src/commands/builtins_register.c \
  src/commands/builtin/fs/cmd_ls.c \
  src/commands/builtin/fs/cmd_cd.c \
  src/commands/builtin/fs/cmd_pwd.c \
  src/commands/builtin/fs/cmd_mkdir.c \
  src/commands/builtin/fs/cmd_rmdir.c \
  src/commands/builtin/fs/cmd_rm.c \
  src/commands/builtin/fs/cmd_cp.c \
  src/commands/builtin/fs/cmd_mv.c \
  src/commands/builtin/fs/cmd_cat.c \
  src/commands/builtin/fs/cmd_touch.c \
  src/commands/builtin/fs/cmd_stat.c \
  src/commands/builtin/fs/cmd_find.c \
  src/commands/builtin/text/cmd_echo.c \
  src/commands/builtin/text/cmd_grep.c \
  src/commands/builtin/text/cmd_wc.c \
  src/commands/builtin/text/cmd_sort.c \
  src/commands/builtin/text/cmd_head.c \
  src/commands/builtin/text/cmd_tail.c \
  src/commands/builtin/text/cmd_cut.c \
  src/commands/builtin/text/cmd_tr.c \
  src/commands/builtin/sys/cmd_env.c \
  src/commands/builtin/sys/cmd_set.c \
  src/commands/builtin/sys/cmd_unset.c \
  src/commands/builtin/sys/cmd_date.c \
  src/commands/builtin/sys/cmd_clear.c \
  src/commands/builtin/sys/cmd_exit.c \
  src/commands/builtin/sys/cmd_alias.c \
  src/commands/builtin/sys/cmd_version.c \
  src/commands/builtin/sys/cmd_help.c \
  src/commands/builtin/process/cmd_exec.c \
  src/commands/builtin/process/cmd_ps.c \
  src/commands/builtin/process/cmd_kill.c \
  src/commands/builtin/process/cmd_jobs.c \
  src/commands/builtin/process/cmd_bg.c \
  src/commands/builtin/process/cmd_fg.c

SRC_CORE = \
  src/core/session.c \
  src/core/config.c \
  src/core/repl.c \
  src/core/terminal.c

SRC_MAIN = src/main.c

ALL_LIB_SRC = \
  $(SRC_UTILS) \
  $(SRC_PLATFORM) \
  $(SRC_UI) \
  $(SRC_PARSER) \
  $(SRC_INPUT) \
  $(SRC_IO) \
  $(SRC_SCRIPTING) \
  $(SRC_COMMANDS) \
  $(SRC_CORE)

SRC_TESTS = \
  tests/test_main.c \
  tests/test_lexer.c \
  tests/test_parser.c \
  tests/test_utils.c \
  tests/test_io.c

# Object files
OBJ = $(ALL_LIB_SRC:.c=.o) $(SRC_MAIN:.c=.o)
OBJ_TESTS = $(ALL_LIB_SRC:.c=.o) $(SRC_TESTS:.c=.o)

# -------------------------------------------------------
# Targets
# -------------------------------------------------------

.PHONY: all clean tests install

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Built: $@"

$(TEST_TARGET): $(OBJ_TESTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Built: $@"

tests: $(TEST_TARGET)
	./$(TEST_TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	find src tests -name '*.o' -delete 2>/dev/null || true
	rm -f $(TARGET) $(TEST_TARGET)
	@echo "Cleaned."

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/cfd
	@echo "Installed to /usr/local/bin/cfd"
