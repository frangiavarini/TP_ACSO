#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "inode.h"
#include "diskimg.h"
#include <string.h>


int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inode_out) {
    if (!fs || !inode_out || inumber <= 0) return -1;

    const int inodesPerSector = DISKIMG_SECTOR_SIZE / sizeof(struct inode);
    const int inodeIndex = inumber - 1;
    const int sectorNum = INODE_START_SECTOR + (inodeIndex / inodesPerSector);
    const int positionInSector = inodeIndex % inodesPerSector;

    char sectorData[DISKIMG_SECTOR_SIZE];
    int readBytes = diskimg_readsector(fs->dfd, sectorNum, sectorData);
    if (readBytes < 0) {
        fprintf(stderr, "inode_iget: Failed to read sector %d\n", sectorNum);
        return -1;
    }

    struct inode *inodeArray = (struct inode *) sectorData;
    memcpy(inode_out, &inodeArray[positionInSector], sizeof(struct inode));

    return 0;
}


int inode_indexlookup(struct unixfilesystem *fs, struct inode *inode, int blockNum) {
    if (!fs || !inode || blockNum < 0) return -1;

    
    if ((inode->i_mode & ILARG) == 0) {
        if (blockNum >= 8) return -1;
        return inode->i_addr[blockNum];
    }

    char data[DISKIMG_SECTOR_SIZE];

    if (blockNum < 7 * 256) {
        int indirectSlot = blockNum / 256;
        int blockOffset = blockNum % 256;

        int indirectBlockAddr = inode->i_addr[indirectSlot];
        if (diskimg_readsector(fs->dfd, indirectBlockAddr, data) < 0) return -1;

        unsigned short *entries = (unsigned short *) data;
        return entries[blockOffset];
    }

   
    int logicalIndex = blockNum - 7 * 256;
    int outerIndex = logicalIndex / 256;
    int innerIndex = logicalIndex % 256;

    int doubleIndirectAddr = inode->i_addr[7];
    if (diskimg_readsector(fs->dfd, doubleIndirectAddr, data) < 0) return -1;

    unsigned short *firstLevel = (unsigned short *) data;
    int secondLevelBlock = firstLevel[outerIndex];

    if (diskimg_readsector(fs->dfd, secondLevelBlock, data) < 0) return -1;

    unsigned short *secondLevel = (unsigned short *) data;
    return secondLevel[innerIndex];
}


int inode_getsize(struct inode *inp) {
  return ((inp->i_size0 << 16) | inp->i_size1); 
}