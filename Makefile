arch ?= x86_64
kernel := build/kernel-$(arch).bin
iso := build/os-$(arch).iso
ext2_img := build/kernel_disk.img
linker_script := src/arch/$(arch)/linker.ld
grub_cfg := src/arch/$(arch)/isofiles/boot/grub/grub.cfg

# Assembly source files
assembly_source_files := $(wildcard src/arch/$(arch)/*.asm)
assembly_object_files := $(patsubst src/arch/$(arch)/%.asm, \
 build/arch/$(arch)/%.o, $(assembly_source_files))

# C source files
c_source_files := $(wildcard src/kernel/*.c)
c_object_files := $(patsubst src/kernel/%.c, \
 build/kernel/%.o, $(c_source_files))

# C compiler and flags
CC = x86_64-elf-gcc
CFLAGS = -ffreestanding -O2 -Wall -Wextra -Werror -mno-red-zone -c -g

.PHONY: all clean run run_ext2 iso ext2_disk

all: $(kernel)

clean:
	@rm -rf build
	@rm -f kernel_disk.img

# Run with ISO image (CDROM)
run: $(iso)
	@qemu-system-x86_64 -s -cdrom $(iso) -serial stdio

# Run with ext2 disk image
run_ext2: $(ext2_img)
	@qemu-system-x86_64 -s -drive format=raw,file=$(ext2_img) -serial stdio

# Create ISO image
iso: $(iso)
$(iso): $(kernel) $(grub_cfg)
	@mkdir -p build/isofiles/boot/grub
	@cp $(kernel) build/isofiles/boot/kernel.bin
	@cp $(grub_cfg) build/isofiles/boot/grub
	@grub-mkrescue -o $(iso) build/isofiles 2> /dev/null
	@rm -r build/isofiles

# Create ext2 disk image
ext2_disk: $(kernel) $(grub_cfg)
	@chmod +x create_disk.sh
	@./create_disk.sh
	@mkdir -p build
	@mv kernel_disk.img $(ext2_img)

# Link kernel from assembly and C objects
$(kernel): $(assembly_object_files) $(c_object_files) $(linker_script)
	@mkdir -p $(shell dirname $@)
	@ld -n -T $(linker_script) -o $(kernel) $(assembly_object_files) $(c_object_files)

# Compile assembly files
build/arch/$(arch)/%.o: src/arch/$(arch)/%.asm
	@mkdir -p $(shell dirname $@)
	@nasm -felf64 $< -o $@

# Compile C files
build/kernel/%.o: src/kernel/%.c
	@mkdir -p $(shell dirname $@)
	@$(CC) $(CFLAGS) $< -o $@