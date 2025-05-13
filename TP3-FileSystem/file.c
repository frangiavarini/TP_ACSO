#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "file.h"
#include "inode.h"
#include "diskimg.h"


int file_getblock(struct unixfilesystem *fs, int inumber, int blockIndex, void *buffer) {
    if (!fs || !buffer || inumber <= 0 || blockIndex < 0) return -1;

    struct inode node;
    if (inode_iget(fs, inumber, &node) != 0) return -1;

    int physicalBlock = inode_indexlookup(fs, &node, blockIndex);
    if (physicalBlock < 0) return -1;
    if (physicalBlock == 0) return 0; 

    int read = diskimg_readsector(fs->dfd, physicalBlock, buffer);
    if (read < 0) return -1;

    int fileSizeBytes = inode_getsize(&node);
    int offset = blockIndex * DISKIMG_SECTOR_SIZE;

    if (fileSizeBytes <= offset) return 0;

    int remainingBytes = fileSizeBytes - offset;
    return remainingBytes >= DISKIMG_SECTOR_SIZE ? DISKIMG_SECTOR_SIZE : remainingBytes;
}


