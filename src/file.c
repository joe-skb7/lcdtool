// SPDX-License-Identifier: GPL-3.0-only

#include <file.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define READ_CHUNK 4096

bool file_exist(const char *path)
{
	return access(path, F_OK) != -1;
}

/**
 * Get file size (in bytes).
 *
 * @param path Path to file
 * @return File size in bytes or -1 on error
 */
off_t file_size(const char *path)
{
	struct stat st;

	if (stat(path, &st) == 0)
		return st.st_size;

	return -1;
}

/**
 * Read binary file.
 *
 * Caller must free allocated buf.
 *
 * @param path Path to file to read; if NULL, stdin will be used
 * @param[out] len If not NULL, will contain length of read data, in bytes
 * @return Read data on success or NULL on failure
 */
char *file_read(const char *path, size_t *len)
{
	FILE *f;
	size_t got, fsize = 0;
	char *buf;

	if (path) {
		f = fopen(path, "r");
		if (!f) {
			perror("Error: Unable to open file");
			return NULL;
		}
	} else {
		f = stdin;
	}

	buf = malloc(READ_CHUNK);
	if (!buf)
		goto err_malloc;

	while ((got = fread(buf + fsize, 1, READ_CHUNK, f)) == READ_CHUNK) {
		fsize += READ_CHUNK;
		buf = realloc(buf, fsize + READ_CHUNK);
		if (!buf)
			goto err_realloc;
	}
	fsize += got;

	buf = realloc(buf, fsize);
	if (!buf)
		goto err_realloc;

	if (f != stdin)
		fclose(f);
	if (len)
		*len = fsize;

	return buf;

err_realloc:
	free(buf);
err_malloc:
	fclose(f);
	return NULL;
}
