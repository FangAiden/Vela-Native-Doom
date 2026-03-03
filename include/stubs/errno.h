#ifndef _VELA_ERRNO_H
#define _VELA_ERRNO_H

extern int errno;

#define EPERM    1
#define ENOENT   2
#define ESRCH    3
#define EINTR    4
#define EIO      5
#define ENXIO    6
#define EBADF    9
#define ENOMEM  12
#define EACCES  13
#define EFAULT  14
#define EEXIST  17
#define ENOTDIR 20
#define EISDIR  21
#define EINVAL  22
#define EMFILE  24
#define ENFILE  23
#define ENOSPC  28
#define ERANGE  34
#define ENOSYS  88

#endif
