#ifndef _VELA_UNISTD_H
#define _VELA_UNISTD_H

#include <stddef.h>

typedef int ssize_t;
typedef int off_t;
typedef int pid_t;
typedef unsigned int mode_t;
typedef unsigned int uid_t;
typedef unsigned int gid_t;

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define F_OK 0
#define R_OK 4
#define W_OK 2
#define X_OK 1
int access(const char *path, int mode);

char *getcwd(char *buf, size_t size);

#endif
