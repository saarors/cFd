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
  LDFLAGS += -lshlwapi -lwinhttp
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
  src/commands/builtin/fs/cmd_du.c \
  src/commands/builtin/fs/cmd_df.c \
  src/commands/builtin/fs/cmd_ln.c \
  src/commands/builtin/fs/cmd_realpath.c \
  src/commands/builtin/fs/cmd_tree.c \
  src/commands/builtin/fs/cmd_chmod.c \
  src/commands/builtin/text/cmd_echo.c \
  src/commands/builtin/text/cmd_grep.c \
  src/commands/builtin/text/cmd_wc.c \
  src/commands/builtin/text/cmd_sort.c \
  src/commands/builtin/text/cmd_head.c \
  src/commands/builtin/text/cmd_tail.c \
  src/commands/builtin/text/cmd_cut.c \
  src/commands/builtin/text/cmd_tr.c \
  src/commands/builtin/text/cmd_tee.c \
  src/commands/builtin/text/cmd_uniq.c \
  src/commands/builtin/text/cmd_diff.c \
  src/commands/builtin/text/cmd_base64.c \
  src/commands/builtin/text/cmd_xargs.c \
  src/commands/builtin/text/cmd_column.c \
  src/commands/builtin/text/cmd_fold.c \
  src/commands/builtin/sys/cmd_env.c \
  src/commands/builtin/sys/cmd_set.c \
  src/commands/builtin/sys/cmd_unset.c \
  src/commands/builtin/sys/cmd_date.c \
  src/commands/builtin/sys/cmd_clear.c \
  src/commands/builtin/sys/cmd_exit.c \
  src/commands/builtin/sys/cmd_alias.c \
  src/commands/builtin/sys/cmd_version.c \
  src/commands/builtin/sys/cmd_help.c \
  src/commands/builtin/sys/cmd_which.c \
  src/commands/builtin/sys/cmd_type.c \
  src/commands/builtin/sys/cmd_history.c \
  src/commands/builtin/sys/cmd_source.c \
  src/commands/builtin/sys/cmd_export.c \
  src/commands/builtin/sys/cmd_read.c \
  src/commands/builtin/sys/cmd_sleep.c \
  src/commands/builtin/sys/cmd_uname.c \
  src/commands/builtin/sys/cmd_whoami.c \
  src/commands/builtin/sys/cmd_hostname.c \
  src/commands/builtin/sys/cmd_uptime.c \
  src/commands/builtin/sys/cmd_watch.c \
  src/commands/builtin/sys/cmd_test_cmd.c \
  src/commands/builtin/sys/cmd_update.c \
  src/commands/builtin/process/cmd_exec.c \
  src/commands/builtin/process/cmd_ps.c \
  src/commands/builtin/process/cmd_kill.c \
  src/commands/builtin/process/cmd_jobs.c \
  src/commands/builtin/process/cmd_bg.c \
  src/commands/builtin/process/cmd_fg.c \
  src/commands/builtin/process/cmd_wait.c \
  src/commands/builtin/net/cmd_curl.c \
  src/commands/builtin/net/cmd_wget.c \
  src/commands/builtin/net/cmd_ping.c \
  src/commands/builtin/net/cmd_netstat.c \
  src/commands/builtin/net/cmd_ipconfig.c \
  src/commands/builtin/editor/cmd_nano.c \
  src/commands/builtin/pkg/cmd_pkg.c \
  src/commands/builtin/crypto/cmd_md5.c \
  src/commands/builtin/crypto/cmd_sha256.c \
  src/commands/builtin/math/cmd_calc.c \
  src/commands/builtin/math/cmd_seq.c \
  src/commands/builtin/math/cmd_expr.c

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

.PHONY: all clean tests install installer-win installer-nsis installer-mac

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

# ── Windows MSI (requires WiX Toolset 3 — https://wixtoolset.org) ──────────
installer-win: $(TARGET)
	@mkdir -p build
	candle installer/windows/cfd.wxs -o build/cfd.wixobj
	light  build/cfd.wixobj -ext WixUIExtension -o build/cFd-1.0.0-windows.msi
	@echo "MSI: build/cFd-1.0.0-windows.msi"

# ── Windows NSIS Setup EXE (requires NSIS — https://nsis.sourceforge.io) ───
installer-nsis: $(TARGET)
	@mkdir -p build
	makensis installer/windows/cfd.nsi
	@echo "Setup: build/cFd-setup-1.0.0.exe"

# ── macOS .app + DMG ────────────────────────────────────────────────────────
installer-mac:
	@bash installer/macos/build_app.sh
