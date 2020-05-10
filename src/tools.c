// SPDX-License-Identifier: GPL-3.0-only

#include <tools.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

#define ESTR2INTOVERFLOW	1
#define ESTR2INTUNDERFLOW	2
#define ESTR2INTINCONVERTIBLE	3

/**
 * Convert string to int.
 *
 * @param[out] out The converted int; cannot be NULL
 * @param[in] s Input string to be converted; cannot be NULL
 *              The format is the same as in strtol(), except that the
 *              following are inconvertible:
 *                - empty string
 *                - leading whitespace
 *                - any trailing characters that are not part of the number
 * @param[in] base Base to interpret string in. Same range as strtol (2 to 36)
 * @return 0 on success or negative value on error
 */
int str2int(int *out, char *s, int base)
{
	char *end;
	long l;

	assert(out);
	assert(s);
	assert(base >= 2 && base <= 36);

	if (s[0] == '\0' || isspace(s[0]))
		return -ESTR2INTINCONVERTIBLE;

	errno = 0;
	l = strtol(s, &end, base);

	/* Both checks are needed because INT_MAX == LONG_MAX is possible */
	if (l > INT_MAX || (errno == ERANGE && l == LONG_MAX))
		return -ESTR2INTOVERFLOW;
	if (l < INT_MIN || (errno == ERANGE && l == LONG_MIN))
		return -ESTR2INTUNDERFLOW;
	if (*end != '\0')
		return -ESTR2INTINCONVERTIBLE;

	*out = l;
	return 0;
}
