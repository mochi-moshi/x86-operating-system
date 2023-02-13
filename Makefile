CC=clang
CWARN=
CFLAGS= $(CWARN)
LINKER_FLAGS=-m elf_i386 --oformat binary -M

SRC=src
BIN=bin
BUILD=build
BBIN=build/bin

.PHONY: build-clean build partial-clean clean
.SILENT:

build-partial: partial-clean build
build-all: clean build
build: $(BIN)/disk.img $(BIN)/bootsector.bin $(BIN)/bootloader.bin
	mkdir -p $(BIN)/fs
	mkdir -p $(BIN)/fs/boot
	cp -R $(SRC)/root/* $(BIN)/fs
	cp $(SRC)/bootloader/bootloader.s $(BIN)/fs/boot/entry
	sudo mkfs.ext2 -F -q -d bin/fs /dev/loop1 > /dev/null
	dd if=$(BIN)/bootsector.bin of=$(BIN)/disk.img bs=440 count=1 conv=notrunc status=none
	dd if=$(BIN)/bootloader.bin of=$(BIN)/disk.img seek=2048 bs=512 conv=notrunc status=none

$(BIN)/disk.img:
	dd if=/dev/zero of=$(BIN)/disk.img bs=512 count=131072
	echo "n\np\n1\n2048\n131071\na\nw" | fdisk $(BIN)/disk.img
	sudo losetup /dev/loop0 $(BIN)/disk.img
	sudo losetup /dev/loop1 $(BIN)/disk.img -o 1048576

$(BUILD)/bootsector.o: $(SRC)/bootsector/bootsector.s
	nasm -f elf32 -g3 -F dwarf $< -o $@

$(BUILD)/bootloader.o: $(SRC)/bootloader/bootloader.s
	nasm -f elf32 -g3 -F dwarf $< -o $@

$(BUILD)/debug/bootsector.elf: $(BUILD)/bootsector.o
	ld -T$(BUILD)/linkers/bootsector.ld -melf_i386 $< -o $@

$(BUILD)/debug/bootloader.elf: $(BUILD)/bootloader.o
	ld -T$(BUILD)/linkers/bootloader.ld -melf_i386 $< -o $@

$(BIN)/bootsector.bin: $(BUILD)/bootsector.o
	ld -T$(BUILD)/linkers/bootsector.ld -melf_i386 --oformat=binary $< -o $@

$(BIN)/bootloader.bin: $(BUILD)/bootloader.o
	ld -T$(BUILD)/linkers/bootloader.ld -melf_i386 --oformat=binary $< -o $@

run: $(BIN)/disk.img
	-export DISPLAY=:0;\
	qemu-system-i386 \
	-display gtk \
	-name "Operating System" \
	-m 1G \
	-drive format=raw,file=$(BIN)/disk.img,index=0,media=disk \
	-rtc base=localtime,clock=host,driftfix=slew

dbgfile=bootloader
debug: $(BIN)/disk.img $(BUILD)/debug/$(dbgfile).elf
	-export DISPLAY=:0;\
	qemu-system-i386 \
	-s -S \
	-display gtk \
	-name "Operating System" \
	-m 1G \
	-drive format=raw,file=$(BIN)/disk.img,index=0,media=disk \
	-rtc base=localtime,clock=host,driftfix=slew & \
	gdb -ix gdbinit_real_mode.txt $(BUILD)/debug/$(dbgfile).elf \
        -ex 'target remote localhost:1234' \
        -ex 'set architecture i8086' \
	-ex 'set tdesc filename target.xml' \
        -ex 'layout src' \
        -ex 'layout regs' \
        -ex 'break _start' \
	-ex 'break *0x7c00' \
	-ex 'continue'
partial-clean:
	-rm -rf build/*.o bin/*.bin bin/fs/*

clean:
	-rm -rf bin/fs
	-sudo losetup -d /dev/loop0
	-sudo losetup -d /dev/loop1
	-rm -rf build/*.o bin/*.bin bin/fs/* bin/disk.img
