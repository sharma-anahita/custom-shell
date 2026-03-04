# Shelly - A Custom Shell Implementation in C

A lightweight, cross-platform shell implementation written in C that provides basic command-line functionality with custom built-in commands and external command execution.

## 🌟 Features

- **Built-in Commands**: `cd`, `pwd`, `echo`, `which`, `env`, `set`, `unset`, `help`, `exit`
- **External Command Execution**: Run system binaries and executables via fork/exec
- **Cross-platform Support**: Works on Windows (MinGW/MSYS2) and Unix-like systems (Linux, macOS)
- **Environment Variable Management**: View, set, and unset environment variables
- **Environment Variable Expansion**: Support for `$VARIABLE` syntax in echo command
- **Case-insensitive Environment Variables**: Enhanced Windows compatibility
- **Custom String Functions**: Minimal dependency on standard string library
- **PATH Resolution**: Automatic executable discovery in system PATH (including Windows PATHEXT support)

## 📋 Prerequisites

### Windows
- MinGW or MSYS2 with GCC compiler
- Make utility (usually included with MinGW/MSYS2)

### Linux/macOS
- GCC compiler
- Make utility
- Standard POSIX development tools

## 🚀 Installation

### Clone the Repository
```bash
git clone https://github.com/sharma-anahita/custom-shell
cd custom-shell
```

### Build from Source

#### Using Make (Recommended)
```bash
make
```

This will compile all source files and generate `main.exe` (Windows) or `main` (Unix).

#### Manual Compilation
```bash
gcc -Wall -Wextra -std=c11 main.c helpers.c input_parser.c builtin.c externals.c -o main
```

#### Clean Build Artifacts
```bash
make clean
```

## 💻 Usage

### Starting the Shell
```bash
./main.exe    # Windows
./main        # Linux/macOS
```

You'll see a prompt like:
```
Welcome to this simple shell!
(My_Shelly)>/current/directory
```

### Available Commands

#### Built-in Commands

##### `cd [directory]`
Change the current working directory.
```bash
cd ~              # Go to home directory
cd /path/to/dir   # Go to specific directory
cd ..             # Go to parent directory
cd                # Print current directory
```

##### `pwd`
Print the current working directory.
```bash
pwd
```

##### `echo [options] [arguments]`
Print arguments to standard output with environment variable expansion.
```bash
echo Hello World
echo $PATH           # Print PATH environment variable
echo $HOME           # Print HOME directory (or USERPROFILE on Windows)
echo -n No newline   # Suppress trailing newline
```

**Note**: Environment variable lookup is case-insensitive for better Windows compatibility.

##### `which [command]`
Locate a command in the system PATH or identify built-in commands.
```bash
which ls
which python
which cd            # Identifies as built-in
```

##### `env`
Display all environment variables.
```bash
env
```

##### `set [variable] [value]`
Set or update an environment variable.
```bash
set MY_VAR hello        # Set MY_VAR=hello
set PATH /new/path      # Update PATH
```

**Supported formats**:
- `set VAR value` - Set variable to value
- `set VAR=value` - Alternative syntax

##### `unset [variable...]`
Remove one or more environment variables.
```bash
unset MY_VAR
unset VAR1 VAR2 VAR3    # Remove multiple variables
```

##### `help`
Display help information (placeholder).
```bash
help
```

##### `exit` or `quit`
Exit the shell.
```bash
exit
quit
```

#### External Commands

Shelly can execute any system binary or executable found in your PATH:

```bash
ls -la              # List files (Unix)
dir                 # List files (Windows)
cat file.txt        # View file contents
python script.py    # Run Python scripts
gcc main.c -o main  # Compile C programs
```

The shell will:
1. Search for the executable in your system PATH
2. Create a child process using `fork()`
3. Execute the command using `execve()`
4. Report exit status and any signals

## 🏗️ Project Structure

```
custom-shell/
├── main.c              # Main shell loop and command dispatcher
├── builtin.c           # Built-in command implementations
├── externals.c         # External command execution (fork/exec)
├── helpers.c           # Helper functions and utilities
├── input_parser.c      # Command input parsing logic
├── shelly.h            # Header file with function declarations
├── Makefile            # Build configuration
├── README.md           # This file
├── .gitignore          # Git ignore rules
└── .vscode/            # VS Code configuration
    └── tasks.json
```
 
### Cross-Platform Compatibility

The shell uses preprocessor directives to handle platform differences:

 

**Windows-specific features**:
- PATHEXT support for finding `.exe`, `.bat`, `.cmd` files
- Uses `_access()` instead of `access()`
- Backslash path separators
- USERPROFILE fallback for HOME variable
 
## 🚧 Planned Features
 
- [ ] Command piping (`|`)
- [ ] I/O redirection (`>`, `<`, `>>`, `2>`)
- [ ] Background processes (`&`) and job control
- [ ] Command history with arrow key navigation
- [ ] Tab completion for commands and paths
- [ ] Enhanced signal handling (Ctrl+C, Ctrl+Z)
- [ ] Quote and escape sequence handling
- [ ] Wildcard/glob expansion
- [ ] Command aliases
- [ ] Shell scripting support
- [ ] Configuration file (`~/.shellyrc`)
- [ ] Better cross-platform testing and compatibility

## 🤝 Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.
