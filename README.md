# Shelly - A Custom Shell Implementation in C

A lightweight, cross-platform shell implementation written in C that provides basic command-line functionality with custom built-in commands.

## 🌟 Features

- **Built-in Commands**: `cd`, `pwd`, `echo`, `which`, `env`, `help`, `exit`
- **Cross-platform Support**: Works on Windows and Unix-like systems (Linux, macOS)
- **Environment Variable Expansion**: Support for `$VARIABLE` syntax in echo command
- **Case-insensitive Environment Variables**: Better Windows compatibility
- **Custom String Functions**: No dependency on standard string library for core operations
- **PATH Resolution**: Automatic executable discovery in system PATH

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
gcc -Wall -Wextra -std=c11 main.c helpers.c input_parser.c builtin.c -o main
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

#### `cd [directory]`
Change the current working directory.
```bash
cd ~              # Go to home directory
cd /path/to/dir   # Go to specific directory
cd ..             # Go to parent directory
cd                # Print current directory
```

#### `pwd`
Print the current working directory.
```bash
pwd
```

#### `echo [options] [arguments]`
Print arguments to standard output with environment variable expansion.
```bash
echo Hello World
echo $PATH           # Print PATH environment variable
echo $HOME           # Print HOME directory
echo -n No newline   # Suppress trailing newline
```

#### `which [command]`
Locate a command in the system PATH.
```bash
which ls
which python
```

#### `env`
Display all environment variables.
```bash
env
```

#### `help`
Display help information (currently placeholder).
```bash
help
```

#### `exit` or `quit`
Exit the shell.
```bash
exit
```

## 🏗️ Project Structure

```
custom-shell/
├── main.c              # Main shell loop and entry point
├── builtin.c           # Built-in command implementations
├── helpers.c           # Helper functions and utilities
├── input_parser.c      # Command input parsing logic
├── shelly.h            # Header file with function declarations
├── Makefile            # Build configuration
├── README.md           # This file
└── .gitignore          # Git ignore rules
```
 
### Cross-Platform Compatibility
The shell uses preprocessor directives to handle platform differences:
```c
#ifdef _WIN32
    // Windows-specific code
#else
    // Unix-specific code
#endif
```

## 🐛 Known Issues & Limitations

- **External Commands**: Not fully implemented yet
- **Command History**: Not supported
- **Piping & Redirection**: Not supported
- **Job Control**: Not supported (no background processes)
- **Signal Handling**: Basic or missing
- **Quoting**: Limited quote handling in input parser
- **cd -**: Previous directory navigation not implemented
- **Tab Completion**: Not available

## 🚧 Planned Features

- [ ] Full external command execution
- [ ] Command piping (`|`)
- [ ] I/O redirection (`>`, `<`, `>>`)
- [ ] Background processes (`&`)
- [ ] Command history with arrow keys
- [ ] Tab completion
- [ ] Improved error handling
- [ ] Signal handling (Ctrl+C, Ctrl+Z)
- [ ] Better cross-platform testing
- [ ] Configuration file support

 
 
 
 
