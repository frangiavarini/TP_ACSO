#include "pathname.h"
#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>


int pathname_lookup(struct unixfilesystem *fs, const char *path) {
    if (!fs || !path || path[0] != '/') {
        
        return -1;
    }

    int inumber = 1; 

    l
    size_t len = strlen(path);
    char tempPath[len + 1];
    strncpy(tempPath, path, len + 1);

    char *segment = strtok(tempPath, "/");
    struct direntv6 dirEntry;

    while (segment != NULL) {
        if (directory_findname(fs, segment, inumber, &dirEntry) != 0) {
           
            return -1;
        }

        inumber = dirEntry.d_inumber;
        segment = strtok(NULL, "/");
    }

    return inumber;
}


