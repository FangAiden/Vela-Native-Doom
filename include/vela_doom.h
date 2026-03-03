#ifndef VELA_DOOM_H
#define VELA_DOOM_H

#include "vela_types.h"
#include "generated/vela_miwear_watch_5_0.h"

#undef stat
#undef fstat
#undef lstat
#define stat(path, buf)   ((stat_t)(0x00604B45))((path), (buf))
#define fstat(fd, buf)    ((fstat_t)(0x00604795))((fd), (buf))
#define lstat(path, buf)  ((lstat_t)(0x00604ACD))((path), (buf))

#undef free
#define free(ptr) ((free_t)(0x00608A21))((ptr))

#undef signal
#undef kill
#define signal(sig, handler) ((signal_t)(0x008C8609))((sig), (handler))
#define kill(pid, sig)       ((kill_t)(0x00661F65))((pid), (sig))

#define VELA_OPEN    ((open_t)(0x006486E9))
#define VELA_CLOSE   ((close_t)(0x00603731))
#define VELA_READ    ((read_t)(0x006487F9))
#define VELA_WRITE   ((write_t)(0x00648C01))
#define VELA_IOCTL   ((ioctl_t)(0x00603B4D))
#define VELA_MMAP    ((mmap_t)(0x00641DCD))
#define VELA_STAT    ((stat_t)(0x00604B45))
#define VELA_FREE    ((free_t)(0x00608A21))

#endif
