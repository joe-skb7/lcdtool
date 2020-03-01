/* SPDX-License-Identifier: GPL-3.0-only */

#ifndef FILE_H
#define FILE_H

#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

bool file_exist(const char *path);
off_t file_size(const char *path);
char *file_read(const char *path, size_t *len);

#endif /* FILE_H */
