CC=clang
CWARN=
CFLAGS= $(CWARN)
LINKER_FLAGS=-m elf_i386 --oformat binary -M

SRC=src
BIN=bin
BUILD=build
BBIN=build/bin

.PHONY: build-clean build clean
#.SILENT:

build-clean: clean build
build: $(BIN)/disk.img $(BIN)/bootsector.bin
	dd if=bin/bootsector.bin of=bin/disk.img bs=440 count=1 conv=notrunc
$(BIN)/disk.img:
	dd if=/dev/zero of=$(BIN)/disk.img bs=512 count=131072
	echo "n\np\n1\n2048\n131071\na\nw" | fdisk $(BIN)/disk.img
	sudo losetup /dev/loop0 $(BIN)/disk.img
	sudo losetup /dev/loop1 $(BIN)/disk.img -o 1048576
	sudo mkfs.fat -f 2 /dev/loop1
	mkdir -p bin/fs
	sudo mount /dev/loop1 bin/fs

$(BIN)/bootsector.bin: $(SRC)/bootloader/bootsector.s
	nasm $< -o $@

RUNFLAGS=-mem-path tmp.bin
run: $(BIN)/disk.img
	export DISPLAY=:0;\
	qemu-system-x86_64 \
	-display gtk \
	-name "Operating System" \
	-machine pc \
	-m 1G \
	-drive file=$(BIN)/disk.img,media=disk,index=0,format=raw \
	-rtc base=localtime,clock=host,driftfix=slew

clean:
	-sudo umount bin/fs
	-rm -rf bin/fs
	-sudo losetup -d /dev/loop0
	-sudo losetup -d /dev/loop1
	-rm -rf build/*.o bin/disk.img