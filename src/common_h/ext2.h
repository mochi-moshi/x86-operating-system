#ifndef EXT2_H
#define EXT2_H
#include "stddef.h"
#include "drive.h"

#define EXT2_CLEAN 1
#define EXT2_ERROR 2

#define EXT2_INGORE 1
#define EXT2_REMOUNTRO 2
#define EXT2_PANIC

#define EXT2_PREALLOC 0x1
#define EXT2_AFS_INODES 0x2
#define EXT2_JOURNAL 0x4
#define EXT2_EXT_INODES 0x8
#define EXT2_RESIZABLE 0x10
#define EXT2_HASH_DIR 0x20

#define EXT2_COMPRESSION 0x1
#define EXT2_TYPED_DIR 0x2
#define EXT2_REPLAY_JOURNAL 0x4
#define EXT2_HAS_JOURNAL 0x8

#define EXT2_SPARSE_SUPER 0x1
#define EXT2_64BIT_SIZE 0x2
#define EXT2_BTREE_DIRECTORY 0x4

typedef struct __attribute__((packed)) {
    uint32_t total_inodes;
    uint32_t total_blocks;
    uint32_t superuser_blocks_reserved;
    uint32_t unallocated_blocks;
    uint32_t unallocated_inodes;
    uint32_t superblock_block;
    uint32_t block_size_shift; // log2(block size) - 10
    uint32_t fragment_size_shift;
    uint32_t blocks_per_group;
    uint32_t fragments_per_group;
    uint32_t inodes_per_group;
    uint32_t last_mount_time;
    uint32_t last_written_time;
    uint16_t mounts_since_last_check;
    uint16_t mounts_before_check;
    uint16_t ext2_sig; // 0xEF53
    uint16_t state;
    uint16_t error_strategy;
    uint16_t minor_version;
    uint32_t last_check_time;
    uint32_t forced_check_interval;
    uint32_t operating_system_id;
    uint32_t major_version;
    uint16_t superuser_id;
    uint16_t supergroup_id;
    uint32_t first_unreserved_inode;
    uint16_t inode_size;
    uint16_t superblock_group;
    uint32_t optional_features;
    uint32_t required_features;
    uint32_t write_features;
    uint32_t id[4];
    char     volume_name[16];
    char     last_mount_path[64];
    uint32_t compression_algorithm;
    uint8_t  file_prealloc;
    uint8_t  dir_prealloc;
    uint16_t _unused;
    uint32_t journal_id[4];
    uint32_t journal_inode;
    uint32_t journal_device;
    uint32_t orphan_inode_head;
    uint8_t  _unused2[19];
} superblock_t; // 256 bytes

typedef struct __attribute__((packed)) {
    uint32_t block_bitmap_block;
    uint32_t inode_bitmap_block;
    uint32_t inode_table_block;
    uint16_t unallocated_blocks;
    uint16_t unallocated_inodes;
    uint16_t number_of_directories;
    uint8_t  _unused[14];
} block_group_desc_t;

#define EXT2_FIFO 0x1000
#define EXT2_CHARDEV 0x2000
#define EXT2_DIR 0x4000
#define EXT2_BLOCKDEV 0x6000
#define EXT2_FILE 0x8000
#define EXT2_SYMLINK 0xA000
#define EXT2_UNIXSOCK 0xC000

#define EXT2_PERM_OX 0x001
#define EXT2_PERM_OW 0x002
#define EXT2_PERM_OR 0x004
#define EXT2_PERM_GX 0x008
#define EXT2_PERM_GW 0x010
#define EXT2_PERM_GR 0x020
#define EXT2_PERM_UX 0x040
#define EXT2_PERM_UW 0x080
#define EXT2_PERM_UR 0x100
#define EXT2_PERM_STICKY 0x200
#define EXT2_PERM_SGID 0x400
#define EXT2_PERM_SUID 0x800


#define EXT2_SEC_DEL 0x00000001
#define EXT2_COPY_ON_DEL 0x00000002
#define EXT2_FILE_COMP 0x00000004
#define EXT2_SYNC_UPDATE 0x00000008
#define EXT2_IMMUTABLE 0x00000010
#define EXT2_APPEND_ONLY 0x00000020
#define EXT2_NO_DUMP 0x00000040
#define EXT2_NO_ACCESS_UPDATE 0x00000080
#define EXT2_HASHED_DIR 0x00010000
#define EXT2_AFS_DIR 0x00020000
#define EXT2_JOURNAL_FILE 0x00040000

typedef struct __attribute__((packed)) {
    uint16_t type;
    uint16_t userid;
    uint32_t size;
    uint32_t last_access_time;
    uint32_t creation_time;
    uint32_t last_mode_time;
    uint32_t deletion_time;
    uint32_t groupid;
    uint32_t number_of_hard_links;
    uint32_t sector_count;
    uint32_t flags;
    uint32_t opspec; // operating system specific (pertains to fragments)
    uint32_t direct_blocks[12];
    uint32_t indirect1_block;
    uint32_t indirect2_block;
    uint32_t indirect3_block;
    uint32_t generation_number;
    uint32_t ext_attr; // if enabled
    union {
        uint32_t size_upper; // if enabled
        uint32_t directory_acl;
    };
    uint32_t fragment_block;
    uint8_t  opspec2[12];
} inode_t;

#define EXT2_ENTRY_FILE
#define EXT2_ENTRY_DIR
#define EXT2_ENTRY_CHARDEV
#define EXT2_ENTRY_BLOCKDEV
#define EXT2_ENTRY_FIFO
#define EXT2_ENTRY_SOCKET
#define EXT2_ENTRY_SYMLINK

typedef struct __attribute__((packed, aligned (4))) {
    uint32_t inode;
    uint16_t length;
    union {
        uint16_t _name_length;
        struct {
            uint8_t name_length;
            uint8_t type;
        };
    };
    char name[0];
} directory_entry_t; 

static superblock_t __ext2_superblock;

static void ext2_init(void) {
    memset(&__ext2_superblock, 0, sizeof(superblock_t));
    
    drive_read_to(&__ext2_superblock, 2, 256);//sizeof(superblock_t));
}

#endif
