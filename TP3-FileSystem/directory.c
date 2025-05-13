#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "file.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>


int directory_findname(struct unixfilesystem *fs, const char *targetName, int dirInumber, struct direntv6 *result) {
    if (!fs || !targetName || !result) return -1;

    struct inode inodeBuffer;
    if (inode_iget(fs, dirInumber, &inodeBuffer) != 0) return -1;

    int fileSize = inode_getsize(&inodeBuffer);
    int totalBlocks = (fileSize + DISKIMG_SECTOR_SIZE - 1) / DISKIMG_SECTOR_SIZE;
    char data[DISKIMG_SECTOR_SIZE];

    for (int blk = 0; blk < totalBlocks; blk++) {
        int bytesRead = file_getblock(fs, dirInumber, blk, data);
        if (bytesRead < 0) return -1;

        struct direntv6 *dirEntry = (struct direntv6 *) data;
        int entryCount = bytesRead / sizeof(struct direntv6);

        for (int e = 0; e < entryCount; e++) {
            if (strncmp(dirEntry[e].d_name, targetName, FILENAME_MAX) == 0) {
                memcpy(result, &dirEntry[e], sizeof(struct direntv6));
                return 0;
            }
        }
    }

    return -1;
}


