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
