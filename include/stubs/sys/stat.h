#ifndef _VELA_SYS_STAT_H
#define _VELA_SYS_STAT_H

#include <stddef.h>

typedef unsigned int mode_t;
typedef unsigned int dev_t;
typedef unsigned int ino_t;
typedef unsigned int nlink_t;
typedef int          off_t;
typedef unsigned int time_t;

struct stat {
    dev_t   st_dev;
    ino_t   st_ino;
    mode_t  st_mode;
    nlink_t st_nlink;
    unsigned int st_uid;
    unsigned int st_gid;
    dev_t   st_rdev;
    off_t   st_size;
    time_t  st_atime;
    time_t  st_mtime;
    time_t  st_ctime;
};

#define S_ISDIR(m)  (((m) & 0170000) == 0040000)
#define S_ISREG(m)  (((m) & 0170000) == 0100000)
#define S_IRWXU     0700
#define S_IRUSR     0400
#define S_IWUSR     0200
#define S_IXUSR     0100

#endif
