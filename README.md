# Operating System

## Pathway

- [ ] Basic
  - [x] Bootsector
    - [x] Relocate self to 0x0060:0
    - [x] Load first bootable partition to 0x7c00 and run it
  - [ ] Bootloader
    - [ ] Find kernel entry file
    - [ ] Enter 32 bit protected mode
    - [ ] Query Memory
    - [ ] Configure video mode
      - [ ] Allow to choose differet compatable video modes
  - [ ] Filesystem
    - [ ] Can search root directory
    - [ ] Can traverse subdirectories
    - [ ] Can load files from disk
    - [ ] Can create files
    - [ ] Can delete files
    - [ ] Can edit files
  - [ ] Simple linux-like terminal
    - [ ] Command lexing
    - [ ] PATH searching

## Creating the disk image

Create raw file:

```sh
dd if=/dev/zero of=disk.img bs=512 count=131072
```

Partition and make bootable:

```sh
echo "n\np\n1\n2048\n131071\na\nw" | fdisk disk.img
```

Map image to device loops:

```sh
sudo losetup /dev/loop0 disk.img
sudo losetup /dev/loop1 disk.img -o 1048576
```

Format image

```sh
mkdir -p bin/fs 
mkfs.ext2 -d bin/fs /dev/loop1
```
