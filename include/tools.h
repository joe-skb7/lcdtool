/* SPDX-License-Identifier: GPL-3.0-only */

#ifndef TOOLS_H
#define TOOLS_H

#define UNUSED(v)	((void)v)
#define BIT(n)		(1 << (n))
#define ARRAY_SIZE(a)	(sizeof(a) / sizeof(a[0]))

int str2int(int *out, char *s, int base);

#endif /* TOOLS_H */
