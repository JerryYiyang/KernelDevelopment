arch ?= x86_64
kernel := build/kernel-$(arch).bin
iso := build/os-$(arch).iso
ext2_img := build/kernel_disk.img
linker_script := src/arch/$(arch)/linker.ld
grub_cfg := src/arch/$(arch)/isofiles/boot/grub/grub.cfg
assembly_source_files := $(wildcard src/arch/$(arch)/*.asm)
assembly_object_files := $(patsubst src/arch/$(arch)/%.asm, \
 build/arch/$(arch)/%.o, $(assembly_source_files))

.PHONY: all clean run run_ext2 iso ext2_disk

all: $(kernel)

clean:
	@rm -rf build
	@rm -f kernel_disk.img

# Run with ISO image (CDROM)
run: $(iso)
	@qemu-system-x86_64 -cdrom $(iso)

# Run with ext2 disk image
run_ext2: $(ext2_img)
	@qemu-system-x86_64 -drive format=raw,file=$(ext2_img)

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

$(kernel): $(assembly_object_files) $(linker_script)
	@ld -n -T $(linker_script) -o $(kernel) $(assembly_object_files)

# Compile assembly files
build/arch/$(arch)/%.o: src/arch/$(arch)/%.asm
	@mkdir -p $(shell dirname $@)
	@nasm -felf64 $< -o $@