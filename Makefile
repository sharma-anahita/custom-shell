# ════════════════════════════════════════════════════════════════
#  Cross-platform Makefile for Shelly
#  Targets: Windows (MinGW/MSYS2) and POSIX (Linux/macOS)
# ════════════════════════════════════════════════════════════════

CC     = gcc
CFLAGS = -Wall -Wextra -std=c11

SRCS = main.c helpers.c input_parser.c builtin.c externals.c
OBJS = $(SRCS:.c=.o)

# ── detect platform ─────────────────────────────────────────────
ifeq ($(OS), Windows_NT)
    TARGET  = main.exe
    # MinGW needs these to pull in WinSock / Win32 libs
    LDFLAGS = -lkernel32
    RM      = del /Q
else
    TARGET  = main
    LDFLAGS =
    RM      = rm -f
endif

# ── default target ──────────────────────────────────────────────
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

# ── pattern rule for object files ───────────────────────────────
%.o: %.c shelly.h
	$(CC) $(CFLAGS) -c $< -o $@

# ── clean ───────────────────────────────────────────────────────
clean:
	$(RM) $(OBJS) $(TARGET)

.PHONY: all clean