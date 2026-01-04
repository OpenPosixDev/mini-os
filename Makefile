# Simple Makefile for building a freestanding i386 kernel
# Adjust CC/LD/OBJCOPY to match your cross-toolchain (e.g., i686-elf-gcc)

CC ?= i686-elf-gcc
LD ?= i686-elf-ld
OBJCOPY ?= i686-elf-objcopy

CFLAGS ?= -std=gnu11 -ffreestanding -O2 -Wall -Wextra -fno-builtin -fno-pic -m32
LDFLAGS ?= -n -T linker.ld

SRCDIR := .
SRCS := $(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/drivers/*.c)
OBJS := $(SRCS:.c=.o)

KERNEL_ELF := kernel.elf
KERNEL_BIN := kernel.bin

.PHONY: all clean run
all: $(KERNEL_BIN)

# Compile C sources
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Link and produce a flat binary
$(KERNEL_ELF): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	# produce a raw binary suitable for qemu -kernel
	$(OBJCOPY) -O binary $@ $(KERNEL_BIN)

$(KERNEL_BIN): $(KERNEL_ELF)
	@echo "Built $@"

clean:
	rm -f $(OBJS) $(KERNEL_ELF) $(KERNEL_BIN)

# Run with QEMU (if installed). Assumes kernel.bin is a multiboot kernel or
# a flat binary appropriate for -kernel. Adjust as needed for your setup.
run: all
	qemu-system-i386 -kernel $(KERNEL_BIN) -serial stdio

# Notes:
# - This Makefile expects a linker script named 'linker.ld' in the repo root.
# - If you don't have a cross compiler installed, set CC/LD/OBJCOPY to your
#   native toolchain (but building a freestanding kernel with a native gcc on
#   x86_64 may require additional flags).
# - You can customize CFLAGS and toolchain variables as needed.
