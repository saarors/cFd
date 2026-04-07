# cFd

> A fully custom terminal shell written in C — no bash, no PowerShell, no wrappers.

[![Build](https://img.shields.io/badge/build-passing-brightgreen)](#building)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-blue)](#building)
[![Language](https://img.shields.io/badge/language-C11-lightgrey)](#)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)

cFd is a from-scratch terminal shell built entirely in C. It has its own lexer, parser, AST, command dispatcher, line editor, history, tab completion, syntax highlighting, scripting engine, and 70+ built-in commands — none of it delegated to an existing shell.

---

## Download

| Platform | File | Notes |
|----------|------|-------|
| Windows  | [cFd-1.0.0-windows.msi](https://github.com/saarors/cFd/releases/latest) | MSI installer — adds `cfd` to PATH |
| macOS    | [cFd-1.0.0-macos.dmg](https://github.com/saarors/cFd/releases/latest) | App bundle |
| Linux    | [cFd-1.0.0-linux-x86_64.tar.gz](https://github.com/saarors/cFd/releases/latest) | Standalone binary |

Or build from source — see [Building](#building).

---

## Features

- **Custom line editor** — arrow keys, Ctrl+A/E/K/U/W/Y, kill-ring
- **Syntax highlighting** — commands, strings, pipes, variables colored as you type
- **Auto-suggest** — ghost-text completion from history (→ to accept)
- **Ctrl+R** — incremental reverse history search
- **Tab completion** — commands and file paths with `~` expansion
- **Multiline input** — auto-detects incomplete `if`/`while`/pipeline and prompts for continuation
- **Real pipes** — `cmd1 | cmd2 | cmd3` via `dup`/`dup2` (no temp files)
- **I/O redirection** — `>`, `>>`, `<`, `2>`, `2>&1`
- **Scripting** — variables, `if/elif/else`, `while`, `for`, functions, `source`
- **Variable expansion** — `$VAR`, `${VAR}`, `$?`, `$#`, `$1`..`$N`
- **Aliases** — `alias`/`unalias`
- **Job control** — `&`, `jobs`, `bg`, `fg`
- **Ctrl+C forwarding** — kills the running child process, not the shell
- **Execution timer** — shows elapsed time for commands taking >5s
- **Smart prompt** — exit code indicator, git branch, user, host, path
- **Package manager** — `pkg install git` via winget/scoop/apt/brew
- **Built-in text editor** — `nano` with search, cut/paste, save
- **HTTP client** — `curl`, `wget` (WinHTTP on Windows, sockets on Unix)
- **Crypto** — `md5`, `sha256` implemented from scratch (RFC 1321, FIPS 180-4)
- **Calculator** — `calc` with trig, log, sqrt, `pi`, `e`, `ans`
- **Themes** — `default`, `dark`, `minimal`
- **Cross-platform** — Windows (Win32 API) and Unix (POSIX)

---

## Building

**Requirements:** GCC (MinGW on Windows), GNU Make

```sh
git clone https://github.com/saarors/cFd.git
cd cFd
make
./cfd          # Linux/macOS
cfd.exe        # Windows
```

```sh
make clean     # remove build artifacts
make tests     # build and run tests
make install   # install to /usr/local/bin (Unix)
```

---

## Built-in Commands

### Filesystem
`ls` `cd` `pwd` `mkdir` `rmdir` `rm` `cp` `mv` `cat` `touch` `stat` `find` `du` `df` `ln` `tree` `realpath` `chmod`

### Text Processing
`echo` `grep` `wc` `sort` `head` `tail` `cut` `tr` `tee` `uniq` `diff` `base64` `xargs` `column` `fold`

### System
`help` `version` `exit` `clear` `date` `env` `export` `set` `unset` `alias` `source` `history` `which` `type` `read` `sleep` `uname` `whoami` `hostname` `uptime` `watch` `test`

### Process
`ps` `kill` `jobs` `bg` `fg` `exec` `wait`

### Network
`curl` `wget` `ping` `netstat` `ipconfig`

### Math
`calc` `expr` `seq` `md5` `sha256`

### Editor
`nano` — full terminal text editor (`^X` exit, `^S` save, `^W` search, `^K/^U` cut/paste)

### Package Manager
`pkg install <name>` `pkg remove` `pkg search` `pkg list` `pkg info`

---

## Scripting

```sh
# Variables
name="world"
echo "Hello, $name"

# If / elif / else
if test -f config.json
  echo "found"
else
  echo "not found"
fi

# While / For
for f in *.c
  echo "$f"
done

# Functions
function greet
  echo "Hello, $1!"
end
greet cFd

# Pipelines & redirection
ls -la | grep ".c" | sort | head -10 > sources.txt
```

### RC file — `~/.cfdrc`
```sh
alias ll="ls -la"
alias gs="git status"
export EDITOR=nano
theme=dark
```

---

## Prompt Customization

| Escape | Meaning |
|--------|---------|
| `\u`   | username |
| `\h`   | hostname |
| `\w`   | working directory |
| `\g`   | git branch |
| `\t`   | time HH:MM:SS |
| `\e`   | exit code (red if non-zero) |
| `\$`   | `>` green/red based on last exit |
| `\n`   | newline |

Default: `\e\u@\h \w\$ `

---

## Architecture

```
source → lexer → tokens → parser → AST → dispatcher → builtin / external
```

```
cFd/
├── include/          # types, config, version
├── src/
│   ├── utils/        # mem, str, hash, list, path, log
│   ├── platform/     # Win32 + POSIX abstraction
│   ├── ui/           # color, themes, display, prompt
│   ├── parser/       # lexer, tokens, AST, parser
│   ├── input/        # readline, history, completion, keybinds
│   ├── io/           # streams, pipes, redirection
│   ├── scripting/    # variables, control flow, functions
│   ├── commands/     # registry, dispatcher
│   │   └── builtin/  # fs/ text/ sys/ process/ net/ math/ crypto/ editor/ pkg/
│   └── core/         # session, REPL, config, terminal
├── installer/
│   ├── windows/      # cfd.wxs (WiX MSI), cfd.nsi (NSIS)
│   └── macos/        # build_app.sh, Info.plist
├── .github/workflows/release.yml   # auto-build releases on git tag
├── docs/             # GitHub Pages website
└── tests/
```

---

## Releases

Installers are built automatically by GitHub Actions on every tagged release:

```sh
git tag v1.0.0
git push --tags
```

GitHub Actions builds `cFd-*.msi` (Windows), `cFd-*.dmg` (macOS), and `cFd-*-linux.tar.gz` in parallel and attaches them to the GitHub Release.

---

## License

MIT — see [LICENSE](LICENSE)

---

<p align="center">Built from scratch in C · No bash · No PowerShell · No wrappers</p>
