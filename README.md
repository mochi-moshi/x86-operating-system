# Operating System

## Pathway

- [ ] Basic
  - [x] Bootsector
    - [x] Relocate self to 0x0060:0
    - [x] Load first bootable partition to 0x7c00 and run it
  - [ ] Bootloader
    - [x] Find kernel entry file
    - [x] Enter 32 bit protected mode
    - [x] Query Memory
    - [x] Configure video mode
      - [ ] Allow to choose different compatable video modes
    - [x] Setup Virtual Memory
      - [ ] Create Simple Malloc
    - [ ] Load Kernel into Upper Memory
  - [ ] Filesystem
    - [x] Can search root directory
    - [x] Can traverse subdirectories
    - [x] Can load files from disk
    - [ ] Can create files
    - [ ] Can delete files
    - [ ] Can edit files
  - [ ] Simple linux-like terminal
    - [ ] Command lexing
    - [ ] PATH searching
- [ ] Expanded
  - [ ] Break up Kernel into Modules
  - [ ] Seperate Kernel and User space

## Creating the disk image

```sh
make clean
make bin/disk.img
```
