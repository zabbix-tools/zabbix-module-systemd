/*
 * Copyright 2017 Ryan Armstrong <ryan@cavaliercoder.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * C String Builder - https://github.com/cavaliercoder/c-stringbuilder
 *
 * sb.c is a simple, non-thread safe String Builder that makes use of a
 * dynamically-allocated linked-list to enable linear time appending and
 * concatenation.
 */

#ifndef SB_H
#define SB_H

#define SB_FAILURE				-1
#define SB_MAX_FRAG_LENGTH		4096

typedef struct _StringFragment {
	struct _StringFragment	*next;
	int						length;
	char					*str;
} StringFragment;

typedef struct _StringBuilder {
	struct _StringFragment	*root;
	struct _StringFragment	*trunk;
	int						length;
} StringBuilder;

StringBuilder	*sb_create();
int				sb_empty(StringBuilder *sb);
int				sb_append(StringBuilder *sb, const char *str);
int				sb_appendf(StringBuilder *sb, const char *format, ...);
char			*sb_concat(StringBuilder *sb);
void 			sb_reset(StringBuilder *sb);
void			sb_free(StringBuilder *sb);

#endif
