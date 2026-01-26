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
git clone <your-repository-url>
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

## 🔧 Architecture

### Core Components

1. **Main Loop** (`main.c`)
   - Displays prompt with current directory
   - Reads user input
   - Parses commands
   - Executes built-in or external commands

2. **Input Parser** (`input_parser.c`)
   - Tokenizes input into command and arguments
   - Handles whitespace separation
   - Returns NULL-terminated array of strings

3. **Built-in Commands** (`builtin.c`)
   - Implements shell built-in commands
   - Handles environment variable operations
   - Manages directory navigation

4. **Helper Functions** (`helpers.c`)
   - Custom string manipulation functions
   - PATH searching and command resolution
   - Cross-platform compatibility utilities

## 🔍 Technical Details

### Custom String Functions
To avoid standard library dependencies, the shell implements its own string functions:
- `my_strcmp()` - String comparison
- `my_strncmp()` - String comparison with length limit and case-sensitivity option
- `my_strLen()` - String length calculation
- `my_strdup()` - String duplication
- `my_strconcat()` - String concatenation
- `my_strtok()` - String tokenization
- `my_getenv()` - Case-insensitive environment variable lookup

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

## 🤝 Contributing

Contributions are welcome! Here's how you can help:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Development Guidelines
- Follow the existing code style
- Add comments for complex logic
- Test on both Windows and Unix systems when possible
- Update documentation for new features

## 📝 License

This project is open source and available under the [MIT License](LICENSE).

## 👨‍💻 Author

Created as a learning project to understand shell implementation and system programming in C.

## 🙏 Acknowledgments

- Inspired by Unix shells (bash, sh, zsh)
- Built as an educational project to learn:
  - System calls and process management
  - String manipulation in C
  - Cross-platform development
  - Build systems (Make)

## 📚 Resources

- [GNU C Library Documentation](https://www.gnu.org/software/libc/manual/)
- [POSIX Standards](https://pubs.opengroup.org/onlinepubs/9699919799/)
- [Advanced Programming in the UNIX Environment](https://www.apuebook.com/)

## 🔗 Links

- Repository: `<your-repository-url>`
- Issue Tracker: `<your-repository-url>/issues`
- Documentation: `<your-repository-url>/wiki`

---

**Note**: This is an educational project and not intended for production use. For a full-featured shell, please use established shells like bash, zsh, or fish.
