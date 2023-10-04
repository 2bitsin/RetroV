# Variable Definitions
CC := ia16-elf-gcc
AS := ia16-elf-as
LD := ia16-elf-gcc
OBJCOPY := ia16-elf-objcopy

CFLAGS := -std=gnu11 -mcmodel=small -march=i186 -O2 -Wall -nodefaultlibs 
ASFLAGS := 
LDFLAGS := -T src/.link -nostdlib -nodefaultlibs -nostartfiles

# Directory Definitions
SRCDIR := ./src
BUILDDIR := ../build/system

# Files
C_SRC := $(wildcard $(SRCDIR)/*.c)
S_SRC := $(wildcard $(SRCDIR)/*.s)
OBJ := $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(C_SRC)) $(patsubst $(SRCDIR)/%.s,$(BUILDDIR)/%.o,$(S_SRC))
TARGET := $(BUILDDIR)/bios.bin

# Build Rules
all: $(TARGET)

$(TARGET): $(OBJ)
	$(LD) $(LDFLAGS) $^ -o $(@)
	

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.s
	mkdir -p $(BUILDDIR)
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -rf $(BUILDDIR)

.PHONY: all clean