# Operating System

## Pathway

- [ ] Basic
  - [x] Bootsector
    - [x] Relocate self to 0x0060:0
    - [x] Load first bootable partition to 0x7c00 and run it
  - [ ] Bootloader
    - [ ] Enter 32 bit protected mode
    - [ ] Load first bootable partition at 1Mb and jump to it
      - [ ] Display Error if not found
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
mkfs.fat -f 2 -M 0xF8 /dev/loop1
```

Mount image:

```sh
mkdir -p bin/fs 
sudo mount /dev/loop1 bin/fs
```
