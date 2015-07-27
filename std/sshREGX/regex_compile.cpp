
#include "stdafx.h"

#define NLBLOCK cd             
#define PSSTART start_pattern  
#define PSEND   end_pattern    

#define SETBIT(a,b) a[(b)/8] |= (1 << ((b)&7))
#define OFLOW_MAX (INT_MAX - 20)
static ssh_l add_list_to_class(ssh_b *, ssh_ws **, ssh_l, compile_data *, const ssh_u *, ssh_u);
static BOOL compile_regex(ssh_l, ssh_ws **, const ssh_ws **, ssh_l *, BOOL, BOOL, ssh_l, ssh_l, ssh_u *, ssh_l *, ssh_u *, ssh_l *, branch_chain *, compile_data *, ssh_l *);

#define COMPILE_WORK_SIZE (2048*LINK_SIZE)
#define COMPILE_WORK_SIZE_MAX (100*COMPILE_WORK_SIZE)
#define NAMED_GROUP_LIST_SIZE  20
#define WORK_SIZE_SAFETY_MARGIN (100)
#define REQ_CASELESS    (1 << 0)        
#define REQ_VARY        (1 << 1)        
#define REQ_UNSET       (-2)
#define REQ_NONE        (-1)
#define UTF_LENGTH     0x10000000l      

static const short escapes[] = {
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	CHAR_COLON, CHAR_SEMICOLON,
	CHAR_LESS_THAN_SIGN, CHAR_EQUALS_SIGN,
	CHAR_GREATER_THAN_SIGN, CHAR_QUESTION_MARK,
	CHAR_COMMERCIAL_AT, -ESC_A,
	-ESC_B, -ESC_C,
	-ESC_D, -ESC_E,
	0, -ESC_G,
	-ESC_H, 0,
	0, -ESC_K,
	0, 0,
	-ESC_N, 0,
	-ESC_P, -ESC_Q,
	-ESC_R, -ESC_S,
	0, 0,
	-ESC_V, -ESC_W,
	-ESC_X, 0,
	-ESC_Z, CHAR_LEFT_SQUARE_BRACKET,
	CHAR_BACKSLASH, CHAR_RIGHT_SQUARE_BRACKET,
	CHAR_CIRCUMFLEX_ACCENT, CHAR_UNDERSCORE,
	CHAR_GRAVE_ACCENT, 7,
	-ESC_b, 0,
	-ESC_d, ESC_e,
	ESC_f, 0,
	-ESC_h, 0,
	0, -ESC_k,
	0, 0,
	ESC_n, 0,
	-ESC_p, 0,
	ESC_r, -ESC_s,
	ESC_tee, 0,
	-ESC_v, -ESC_w,
	0, 0,
	-ESC_z
};

typedef struct verbitem
{
	ssh_l   len;
	ssh_l   op;
	ssh_l   op_arg;
} verbitem;

static const char verbnames[] =
"\0"
STRING_MARK0
STRING_ACCEPT0
STRING_COMMIT0
STRING_F0
STRING_FAIL0
STRING_PRUNE0
STRING_SKIP0
STRING_THEN;

static const verbitem verbs[] = {
	{0, -1, OP_MARK},
	{4, -1, OP_MARK},
	{6, OP_ACCEPT, -1},
	{6, OP_COMMIT, -1},
	{1, OP_FAIL, -1},
	{4, OP_FAIL, -1},
	{5, OP_PRUNE, OP_PRUNE_ARG},
	{4, OP_SKIP, OP_SKIP_ARG},
	{4, OP_THEN, OP_THEN_ARG}
};

static const ssh_l verbcount = sizeof(verbs) / sizeof(verbitem);
static const ssh_ws sub_start_of_word[] = {CHAR_BACKSLASH, CHAR_b, CHAR_LEFT_PARENTHESIS, CHAR_QUESTION_MARK, CHAR_EQUALS_SIGN, CHAR_BACKSLASH, CHAR_w, CHAR_RIGHT_PARENTHESIS, '\0'};
static const ssh_ws sub_end_of_word[] = {CHAR_BACKSLASH, CHAR_b, CHAR_LEFT_PARENTHESIS, CHAR_QUESTION_MARK, CHAR_LESS_THAN_SIGN, CHAR_EQUALS_SIGN, CHAR_BACKSLASH, CHAR_w, CHAR_RIGHT_PARENTHESIS, '\0'};
static const char posix_names[] = STRING_alpha0 STRING_lower0 STRING_upper0 STRING_alnum0 STRING_ascii0 STRING_blank0 STRING_cntrl0 STRING_digit0 STRING_graph0 STRING_print0 STRING_punct0 STRING_space0 STRING_word0  STRING_xdigit;
static const ssh_b posix_name_lengths[] = {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 6, 0};

#define PC_GRAPH  8
#define PC_PRINT  9
#define PC_PUNCT 10

static const ssh_l posix_class_maps[] = {
	cbit_word, cbit_digit, -2,
	cbit_lower, -1, 0,
	cbit_upper, -1, 0,
	cbit_word, -1, 2,
	cbit_print, cbit_cntrl, 0,
	cbit_space, -1, 1,
	cbit_cntrl, -1, 0,
	cbit_digit, -1, 0,
	cbit_graph, -1, 0,
	cbit_print, -1, 0,
	cbit_punct, -1, 0,
	cbit_space, -1, 0,
	cbit_word, -1, 0,
	cbit_xdigit, -1, 0
};

#define IS_DIGIT(x) ((x) >= CHAR_0 && (x) <= CHAR_9)

static const ssh_b digitab[] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,
	0x0c, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#define APTROWS (LAST_AUTOTAB_LEFT_OP - FIRST_AUTOTAB_OP + 1)
#define APTCOLS (LAST_AUTOTAB_RIGHT_OP - FIRST_AUTOTAB_OP + 1)

static const ssh_b autoposstab[APTROWS][APTCOLS] = {

	{0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
	{1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1},
	{0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1},
	{0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
	{0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
	{0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0},
	{0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0},
	{0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0}
};

static const ssh_b propposstab[PT_TABSIZE][PT_TABSIZE] = {

	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 3, 0, 0, 0, 3, 1, 1, 0, 0, 0},
	{0, 0, 2, 4, 0, 9, 10, 10, 11, 0, 0},
	{0, 0, 5, 2, 0, 15, 16, 16, 17, 0, 0},
	{0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0},
	{0, 3, 6, 12, 0, 3, 1, 1, 0, 0, 0},
	{0, 1, 7, 13, 0, 1, 3, 3, 1, 0, 0},
	{0, 1, 7, 13, 0, 1, 3, 3, 1, 0, 0},
	{0, 0, 8, 14, 0, 0, 1, 1, 3, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3}
};

static const ssh_b catposstab[7][30] = {

	{0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0}
};

static const ssh_b opcode_possessify[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0,
	OP_POSSTAR, 0,
	OP_POSPLUS, 0,
	OP_POSQUERY, 0,
	OP_POSUPTO, 0,
	0,
	0, 0, 0, 0,

	OP_POSSTARI, 0,
	OP_POSPLUSI, 0,
	OP_POSQUERYI, 0,
	OP_POSUPTOI, 0,
	0,
	0, 0, 0, 0,

	OP_NOTPOSSTAR, 0,
	OP_NOTPOSPLUS, 0,
	OP_NOTPOSQUERY, 0,
	OP_NOTPOSUPTO, 0,
	0,
	0, 0, 0, 0,

	OP_NOTPOSSTARI, 0,
	OP_NOTPOSPLUSI, 0,
	OP_NOTPOSQUERYI, 0,
	OP_NOTPOSUPTOI, 0,
	0,
	0, 0, 0, 0,

	OP_TYPEPOSSTAR, 0,
	OP_TYPEPOSPLUS, 0,
	OP_TYPEPOSQUERY, 0,
	OP_TYPEPOSUPTO, 0,
	0,
	0, 0, 0, 0,

	OP_CRPOSSTAR, 0,
	OP_CRPOSPLUS, 0,
	OP_CRPOSQUERY, 0,
	OP_CRPOSRANGE, 0,
	0, 0, 0, 0,

	0, 0, 0,
	0, 0,
	0, 0,
	0, 0
};

static ssh_l expand_workspace(compile_data *cd)
{
	ssh_ws *newspace;
	ssh_l newsize = cd->workspace_size * 2;
	if(newsize > COMPILE_WORK_SIZE_MAX) newsize = COMPILE_WORK_SIZE_MAX;
	if(cd->workspace_size >= COMPILE_WORK_SIZE_MAX || newsize - cd->workspace_size < WORK_SIZE_SAFETY_MARGIN)
		return ERR72;
	newspace = (ssh_ws*)malloc(IN_UCHARS(newsize));
	if(newspace == NULL) return ERR21;
	memcpy(newspace, cd->start_workspace, cd->workspace_size * sizeof(ssh_ws));
	cd->hwm = (ssh_ws *)newspace + (cd->hwm - cd->start_workspace);
	if(cd->workspace_size > COMPILE_WORK_SIZE) free((void *)cd->start_workspace);
	cd->start_workspace = newspace;
	cd->workspace_size = newsize;
	return 0;
}

static BOOL is_counted_repeat(const ssh_ws *p)
{
	if(!IS_DIGIT(*p)) return FALSE;
	p++;
	while(IS_DIGIT(*p)) p++;
	if(*p == CHAR_RIGHT_CURLY_BRACKET) return TRUE;
	if(*p++ != CHAR_COMMA) return FALSE;
	if(*p == CHAR_RIGHT_CURLY_BRACKET) return TRUE;
	if(!IS_DIGIT(*p)) return FALSE;
	p++;
	while(IS_DIGIT(*p)) p++;
	return (*p == CHAR_RIGHT_CURLY_BRACKET);
}

static ssh_l check_escape(const ssh_ws **ptrptr, ssh_u *chptr, ssh_l *errorcodeptr, ssh_l bracount, ssh_l options, BOOL isclass)
{
	BOOL utf = (options & REGEX_UTF8) != 0;
	const ssh_ws *ptr = *ptrptr + 1;
	ssh_u c;
	ssh_l escape = 0;
	ssh_l i;
	GETCHARINCTEST(c, ptr);
	ptr--;
	if(c == CHAR_NULL) *errorcodeptr = ERR1;
	else if(c < CHAR_0 || c > CHAR_z) {}
	else if((i = escapes[c - CHAR_0]) != 0)
	{ if(i > 0) c = (ssh_u)i; else escape = -i; }
	else
	{
		const ssh_ws *oldptr;
		BOOL braced, negated, overflow;
		ssh_l s;
		switch(c)
		{
			case CHAR_l:
			case CHAR_L:
				*errorcodeptr = ERR37;
				break;
			case CHAR_u:
				if((options & REGEX_JAVASCRIPT_COMPAT) != 0)
				{
					if(MAX_255(ptr[1]) && (digitab[ptr[1]] & ctype_xdigit) != 0 && MAX_255(ptr[2]) && (digitab[ptr[2]] & ctype_xdigit) != 0 && MAX_255(ptr[3]) && (digitab[ptr[3]] & ctype_xdigit) != 0 && MAX_255(ptr[4]) && (digitab[ptr[4]] & ctype_xdigit) != 0)
					{
						c = 0;
						for(i = 0; i < 4; ++i)
						{
							register ssh_u cc = *(++ptr);
							if(cc >= CHAR_a) cc -= 32;
							c = (c << 4) + cc - ((cc < CHAR_A) ? CHAR_0 : (CHAR_A - 10));
						}
						if(c > (utf ? 0x10ffffU : 0xffffU))
						{
							*errorcodeptr = ERR76;
						}
						else if(utf && c >= 0xd800 && c <= 0xdfff) *errorcodeptr = ERR73;
					}
				}
				else
					*errorcodeptr = ERR37;
				break;
			case CHAR_U:
				if((options & REGEX_JAVASCRIPT_COMPAT) == 0) *errorcodeptr = ERR37;
				break;
			case CHAR_g:
				if(isclass) break;
				if(ptr[1] == CHAR_LESS_THAN_SIGN || ptr[1] == CHAR_APOSTROPHE)
				{
					escape = ESC_g;
					break;
				}
				if(ptr[1] == CHAR_LEFT_CURLY_BRACKET)
				{
					const ssh_ws *p;
					for(p = ptr + 2; *p != CHAR_NULL && *p != CHAR_RIGHT_CURLY_BRACKET; p++)
						if(*p != CHAR_MINUS && !IS_DIGIT(*p)) break;
					if(*p != CHAR_NULL && *p != CHAR_RIGHT_CURLY_BRACKET)
					{
						escape = ESC_k;
						break;
					}
					braced = TRUE;
					ptr++;
				}
				else braced = FALSE;

				if(ptr[1] == CHAR_MINUS)
				{
					negated = TRUE;
					ptr++;
				}
				else negated = FALSE;
				s = 0;
				overflow = FALSE;
				while(IS_DIGIT(ptr[1]))
				{
					if(s > INT_MAX / 10 - 1)
					{
						overflow = TRUE;
						break;
					}
					s = s * 10 + (ssh_l)(*(++ptr) - CHAR_0);
				}
				if(overflow)
				{
					while(IS_DIGIT(ptr[1]))
						ptr++;
					*errorcodeptr = ERR61;
					break;
				}
				if(braced && *(++ptr) != CHAR_RIGHT_CURLY_BRACKET)
				{
					*errorcodeptr = ERR57;
					break;
				}
				if(s == 0)
				{
					*errorcodeptr = ERR58;
					break;
				}
				if(negated)
				{
					if(s > bracount)
					{
						*errorcodeptr = ERR15;
						break;
					}
					s = bracount - (s - 1);
				}
				escape = -s;
				break;
			case CHAR_1: case CHAR_2: case CHAR_3: case CHAR_4: case CHAR_5:
			case CHAR_6: case CHAR_7: case CHAR_8: case CHAR_9:
				if(!isclass)
				{
					oldptr = ptr;
					s = (ssh_l)(c - CHAR_0);
					overflow = FALSE;
					while(IS_DIGIT(ptr[1]))
					{
						if(s > INT_MAX / 10 - 1)
						{
							overflow = TRUE;
							break;
						}
						s = s * 10 + (ssh_l)(*(++ptr) - CHAR_0);
					}
					if(overflow)
					{
						while(IS_DIGIT(ptr[1]))
							ptr++;
						*errorcodeptr = ERR61;
						break;
					}
					if(s < 8 || s <= bracount)
					{
						escape = -s;
						break;
					}
					ptr = oldptr;
				}
				if((c = *ptr) >= CHAR_8) break;
			case CHAR_0:
				c -= CHAR_0;
				while(i++ < 2 && ptr[1] >= CHAR_0 && ptr[1] <= CHAR_7)
					c = c * 8 + *(++ptr) - CHAR_0;
				break;
			case CHAR_o:
				if(ptr[1] != CHAR_LEFT_CURLY_BRACKET) *errorcodeptr = ERR81; else
					if(ptr[2] == CHAR_RIGHT_CURLY_BRACKET) *errorcodeptr = ERR86; else
					{
						ptr += 2;
						c = 0;
						overflow = FALSE;
						while(*ptr >= CHAR_0 && *ptr <= CHAR_7)
						{
							register ssh_u cc = *ptr++;
							if(c == 0 && cc == CHAR_0) continue;
							c = (c << 3) + cc - CHAR_0;
							if(c > (utf ? 0x10ffffU : 0xffffU)) { overflow = TRUE; break; }
						}
						if(overflow)
						{
							while(*ptr >= CHAR_0 && *ptr <= CHAR_7) ptr++;
							*errorcodeptr = ERR34;
						}
						else if(*ptr == CHAR_RIGHT_CURLY_BRACKET)
						{
							if(utf && c >= 0xd800 && c <= 0xdfff) *errorcodeptr = ERR73;
						}
						else *errorcodeptr = ERR80;
					}
				break;
			case CHAR_x:
				if((options & REGEX_JAVASCRIPT_COMPAT) != 0)
				{
					if(MAX_255(ptr[1]) && (digitab[ptr[1]] & ctype_xdigit) != 0 && MAX_255(ptr[2]) && (digitab[ptr[2]] & ctype_xdigit) != 0)
					{
						c = 0;
						for(i = 0; i < 2; ++i)
						{
							register ssh_u cc = *(++ptr);
							if(cc >= CHAR_a) cc -= 32;
							c = (c << 4) + cc - ((cc < CHAR_A) ? CHAR_0 : (CHAR_A - 10));
						}
					}
				}
				else
				{
					if(ptr[1] == CHAR_LEFT_CURLY_BRACKET)
					{
						ptr += 2;
						if(*ptr == CHAR_RIGHT_CURLY_BRACKET)
						{
							*errorcodeptr = ERR86;
							break;
						}
						c = 0;
						overflow = FALSE;
						while(MAX_255(*ptr) && (digitab[*ptr] & ctype_xdigit) != 0)
						{
							register ssh_u cc = *ptr++;
							if(c == 0 && cc == CHAR_0) continue;
							if(cc >= CHAR_a) cc -= 32;
							c = (c << 4) + cc - ((cc < CHAR_A) ? CHAR_0 : (CHAR_A - 10));
							if(c > (utf ? 0x10ffffU : 0xffffU)) { overflow = TRUE; break; }
						}

						if(overflow)
						{
							while(MAX_255(*ptr) && (digitab[*ptr] & ctype_xdigit) != 0) ptr++;
							*errorcodeptr = ERR34;
						}
						else if(*ptr == CHAR_RIGHT_CURLY_BRACKET)
						{
							if(utf && c >= 0xd800 && c <= 0xdfff) *errorcodeptr = ERR73;
						}
						else *errorcodeptr = ERR79;
					}
					else
					{
						c = 0;
						while(i++ < 2 && MAX_255(ptr[1]) && (digitab[ptr[1]] & ctype_xdigit) != 0)
						{
							ssh_u cc;
							cc = *(++ptr);
							if(cc >= CHAR_a) cc -= 32;
							c = c * 16 + cc - ((cc < CHAR_A) ? CHAR_0 : (CHAR_A - 10));
						}
					}
				}
				break;
			case CHAR_c:
				c = *(++ptr);
				if(c == CHAR_NULL)
				{
					*errorcodeptr = ERR2;
					break;
				}
				if(c > 127)
				{
					*errorcodeptr = ERR68;
					break;
				}
				if(c >= CHAR_a && c <= CHAR_z) c -= 32;
				c ^= 0x40;
				break;
			default:
				if((options & REGEX_EXTRA) != 0) *errorcodeptr = ERR3;
				break;
		}
	}
	if(escape == ESC_N && ptr[1] == CHAR_LEFT_CURLY_BRACKET && !is_counted_repeat(ptr + 2))
		*errorcodeptr = ERR37;
	if((options & REGEX_UCP) != 0 && escape >= ESC_D && escape <= ESC_w)
		escape += (ESC_DU - ESC_D);
	*ptrptr = ptr;
	*chptr = c;
	return escape;
}

static ssh_wcs read_repeat_counts(ssh_wcs p, ssh_l *minp, ssh_l *maxp, ssh_l *errorcodeptr)
{
	ssh_l min = 0;
	ssh_l max = -1;

	while(IS_DIGIT(*p))
	{
		min = min * 10 + (ssh_l)(*p++ - CHAR_0);
		if(min > 65535)
		{
			*errorcodeptr = ERR5;
			return p;
		}
	}

	if(*p == CHAR_RIGHT_CURLY_BRACKET) max = min; else
	{
		if(*(++p) != CHAR_RIGHT_CURLY_BRACKET)
		{
			max = 0;
			while(IS_DIGIT(*p))
			{
				max = max * 10 + (ssh_l)(*p++ - CHAR_0);
				if(max > 65535)
				{
					*errorcodeptr = ERR5;
					return p;
				}
			}
			if(max < min)
			{
				*errorcodeptr = ERR4;
				return p;
			}
		}
	}

	*minp = min;
	*maxp = max;
	return p;
}

static ssh_wcs first_significant_code(ssh_wcs code, BOOL skipassert)
{
	for(;;)
	{
		switch((ssh_l)*code)
		{
			case OP_ASSERT_NOT:
			case OP_ASSERTBACK:
			case OP_ASSERTBACK_NOT:
				if(!skipassert) return code;
				do code += GET(code, 1); while(*code == OP_ALT);
				code += PRIV(OP_lengths)[*code];
				break;
			case OP_WORD_BOUNDARY:
			case OP_NOT_WORD_BOUNDARY:
				if(!skipassert) return code;
			case OP_CALLOUT:
			case OP_CREF:
			case OP_DNCREF:
			case OP_RREF:
			case OP_DNRREF:
			case OP_DEF:
				code += PRIV(OP_lengths)[*code];
				break;
			default:
				return code;
		}
	}
}

static ssh_l find_fixedlength(ssh_ws *code, BOOL utf, BOOL atend, compile_data *cd, recurse_check *recurses)
{
	ssh_l length = -1;
	recurse_check this_recurse;
	register ssh_l branchlength = 0;
	register ssh_ws *cc = code + 1 + LINK_SIZE;
	for(;;)
	{
		ssh_l d;
		ssh_ws *ce, *cs;
		register ssh_ws op = *cc;
		switch(op)
		{
			case OP_CBRA:
			case OP_BRA:
			case OP_ONCE:
			case OP_ONCE_NC:
			case OP_COND:
				d = find_fixedlength(cc + ((op == OP_CBRA) ? IMM2_SIZE : 0), utf, atend, cd, recurses);
				if(d < 0) return d;
				branchlength += d;
				do cc += GET(cc, 1); while(*cc == OP_ALT);
				cc += 1 + LINK_SIZE;
				break;
			case OP_ALT:
			case OP_KET:
			case OP_END:
			case OP_ACCEPT:
			case OP_ASSERT_ACCEPT:
				if(length < 0) length = branchlength;
				else if(length != branchlength) return -1;
				if(*cc != OP_ALT) return length;
				cc += 1 + LINK_SIZE;
				branchlength = 0;
				break;
			case OP_RECURSE:
				if(!atend) return -3;
				cs = ce = (ssh_ws *)cd->start_code + GET(cc, 1);
				do ce += GET(ce, 1); while(*ce == OP_ALT);
				if(cc > cs && cc < ce) return -1;
				else
				{
					recurse_check *r = recurses;
					for(r = recurses; r != NULL; r = r->prev) if(r->group == cs) break;
					if(r != NULL) return -1;
				}
				this_recurse.prev = recurses;
				this_recurse.group = cs;
				d = find_fixedlength(cs + IMM2_SIZE, utf, atend, cd, &this_recurse);
				if(d < 0) return d;
				branchlength += d;
				cc += 1 + LINK_SIZE;
				break;
			case OP_ASSERT:
			case OP_ASSERT_NOT:
			case OP_ASSERTBACK:
			case OP_ASSERTBACK_NOT:
				do cc += GET(cc, 1); while(*cc == OP_ALT);
				cc += PRIV(OP_lengths)[*cc];
				break;
			case OP_MARK:
			case OP_PRUNE_ARG:
			case OP_SKIP_ARG:
			case OP_THEN_ARG:
				cc += cc[1] + PRIV(OP_lengths)[*cc];
				break;
			case OP_CALLOUT:
			case OP_CIRC:
			case OP_CIRCM:
			case OP_CLOSE:
			case OP_COMMIT:
			case OP_CREF:
			case OP_DEF:
			case OP_DNCREF:
			case OP_DNRREF:
			case OP_DOLL:
			case OP_DOLLM:
			case OP_EOD:
			case OP_EODN:
			case OP_FAIL:
			case OP_NOT_WORD_BOUNDARY:
			case OP_PRUNE:
			case OP_REVERSE:
			case OP_RREF:
			case OP_SET_SOM:
			case OP_SKIP:
			case OP_SOD:
			case OP_SOM:
			case OP_THEN:
			case OP_WORD_BOUNDARY:
				cc += PRIV(OP_lengths)[*cc];
				break;
			case OP_CHAR:
			case OP_CHARI:
			case OP_NOT:
			case OP_NOTI:
				branchlength++;
				cc += 2;
				break;
			case OP_EXACT:
			case OP_EXACTI:
			case OP_NOTEXACT:
			case OP_NOTEXACTI:
				branchlength += (ssh_l)GET2(cc, 1);
				cc += 2 + IMM2_SIZE;
				break;
			case OP_TYPEEXACT:
				branchlength += GET2(cc, 1);
				if(cc[1 + IMM2_SIZE] == OP_PROP || cc[1 + IMM2_SIZE] == OP_NOTPROP) cc += 2;
				cc += 1 + IMM2_SIZE + 1;
				break;
			case OP_PROP:
			case OP_NOTPROP:
				cc += 2;
			case OP_HSPACE:
			case OP_VSPACE:
			case OP_NOT_HSPACE:
			case OP_NOT_VSPACE:
			case OP_NOT_DIGIT:
			case OP_DIGIT:
			case OP_NOT_WHITESPACE:
			case OP_WHITESPACE:
			case OP_NOT_WORDCHAR:
			case OP_WORDCHAR:
			case OP_ANY:
			case OP_ALLANY:
				branchlength++;
				cc++;
				break;
			case OP_ANYBYTE:
				return -2;
			case OP_CLASS:
			case OP_NCLASS:
			case OP_XCLASS:
				if(op == OP_XCLASS)
					cc += GET(cc, 1);
				else
					cc += PRIV(OP_lengths)[OP_CLASS];
				switch(*cc)
				{
					case OP_CRSTAR:
					case OP_CRMINSTAR:
					case OP_CRPLUS:
					case OP_CRMINPLUS:
					case OP_CRQUERY:
					case OP_CRMINQUERY:
					case OP_CRPOSSTAR:
					case OP_CRPOSPLUS:
					case OP_CRPOSQUERY:
						return -1;
					case OP_CRRANGE:
					case OP_CRMINRANGE:
					case OP_CRPOSRANGE:
						if(GET2(cc, 1) != GET2(cc, 1 + IMM2_SIZE)) return -1;
						branchlength += (ssh_l)GET2(cc, 1);
						cc += 1 + 2 * IMM2_SIZE;
						break;

					default:
						branchlength++;
				}
				break;
			case OP_ANYNL:
			case OP_BRAMINZERO:
			case OP_BRAPOS:
			case OP_BRAPOSZERO:
			case OP_BRAZERO:
			case OP_CBRAPOS:
			case OP_EXTUNI:
			case OP_KETRMAX:
			case OP_KETRMIN:
			case OP_KETRPOS:
			case OP_MINPLUS:
			case OP_MINPLUSI:
			case OP_MINQUERY:
			case OP_MINQUERYI:
			case OP_MINSTAR:
			case OP_MINSTARI:
			case OP_MINUPTO:
			case OP_MINUPTOI:
			case OP_NOTMINPLUS:
			case OP_NOTMINPLUSI:
			case OP_NOTMINQUERY:
			case OP_NOTMINQUERYI:
			case OP_NOTMINSTAR:
			case OP_NOTMINSTARI:
			case OP_NOTMINUPTO:
			case OP_NOTMINUPTOI:
			case OP_NOTPLUS:
			case OP_NOTPLUSI:
			case OP_NOTPOSPLUS:
			case OP_NOTPOSPLUSI:
			case OP_NOTPOSQUERY:
			case OP_NOTPOSQUERYI:
			case OP_NOTPOSSTAR:
			case OP_NOTPOSSTARI:
			case OP_NOTPOSUPTO:
			case OP_NOTPOSUPTOI:
			case OP_NOTQUERY:
			case OP_NOTQUERYI:
			case OP_NOTSTAR:
			case OP_NOTSTARI:
			case OP_NOTUPTO:
			case OP_NOTUPTOI:
			case OP_PLUS:
			case OP_PLUSI:
			case OP_POSPLUS:
			case OP_POSPLUSI:
			case OP_POSQUERY:
			case OP_POSQUERYI:
			case OP_POSSTAR:
			case OP_POSSTARI:
			case OP_POSUPTO:
			case OP_POSUPTOI:
			case OP_QUERY:
			case OP_QUERYI:
			case OP_REF:
			case OP_REFI:
			case OP_DNREF:
			case OP_DNREFI:
			case OP_SBRA:
			case OP_SBRAPOS:
			case OP_SCBRA:
			case OP_SCBRAPOS:
			case OP_SCOND:
			case OP_SKIPZERO:
			case OP_STAR:
			case OP_STARI:
			case OP_TYPEMINPLUS:
			case OP_TYPEMINQUERY:
			case OP_TYPEMINSTAR:
			case OP_TYPEMINUPTO:
			case OP_TYPEPLUS:
			case OP_TYPEPOSPLUS:
			case OP_TYPEPOSQUERY:
			case OP_TYPEPOSSTAR:
			case OP_TYPEPOSUPTO:
			case OP_TYPEQUERY:
			case OP_TYPESTAR:
			case OP_TYPEUPTO:
			case OP_UPTO:
			case OP_UPTOI:
				return -1;
			default:
				return -4;
		}
	}
}

ssh_wcs PRIV(find_bracket)(ssh_wcs code, BOOL utf, ssh_l number)
{
	for(;;)
	{
		register ssh_ws c = *code;
		if(c == OP_END) return NULL;
		if(c == OP_XCLASS) code += GET(code, 1);
		else if(c == OP_REVERSE)
		{
			if(number < 0) return (ssh_ws *)code;
			code += PRIV(OP_lengths)[c];
		}
		else if(c == OP_CBRA || c == OP_SCBRA || c == OP_CBRAPOS || c == OP_SCBRAPOS)
		{
			ssh_l n = (ssh_l)GET2(code, 1 + LINK_SIZE);
			if(n == number) return (ssh_ws *)code;
			code += PRIV(OP_lengths)[c];
		}
		else
		{
			switch(c)
			{
				case OP_TYPESTAR:
				case OP_TYPEMINSTAR:
				case OP_TYPEPLUS:
				case OP_TYPEMINPLUS:
				case OP_TYPEQUERY:
				case OP_TYPEMINQUERY:
				case OP_TYPEPOSSTAR:
				case OP_TYPEPOSPLUS:
				case OP_TYPEPOSQUERY:
					if(code[1] == OP_PROP || code[1] == OP_NOTPROP) code += 2;
					break;
				case OP_TYPEUPTO:
				case OP_TYPEMINUPTO:
				case OP_TYPEEXACT:
				case OP_TYPEPOSUPTO:
					if(code[1 + IMM2_SIZE] == OP_PROP || code[1 + IMM2_SIZE] == OP_NOTPROP)
						code += 2;
					break;
				case OP_MARK:
				case OP_PRUNE_ARG:
				case OP_SKIP_ARG:
				case OP_THEN_ARG:
					code += code[1];
					break;
			}
			code += PRIV(OP_lengths)[c];
		}
	}
}

static const ssh_ws* find_recurse(ssh_wcs code, BOOL utf)
{
	for(;;)
	{
		register ssh_ws c = *code;
		if(c == OP_END) return NULL;
		if(c == OP_RECURSE) return code;
		if(c == OP_XCLASS) code += GET(code, 1);
		else
		{
			switch(c)
			{
				case OP_TYPESTAR:
				case OP_TYPEMINSTAR:
				case OP_TYPEPLUS:
				case OP_TYPEMINPLUS:
				case OP_TYPEQUERY:
				case OP_TYPEMINQUERY:
				case OP_TYPEPOSSTAR:
				case OP_TYPEPOSPLUS:
				case OP_TYPEPOSQUERY:
					if(code[1] == OP_PROP || code[1] == OP_NOTPROP) code += 2;
					break;
				case OP_TYPEPOSUPTO:
				case OP_TYPEUPTO:
				case OP_TYPEMINUPTO:
				case OP_TYPEEXACT:
					if(code[1 + IMM2_SIZE] == OP_PROP || code[1 + IMM2_SIZE] == OP_NOTPROP)
						code += 2;
					break;
				case OP_MARK:
				case OP_PRUNE_ARG:
				case OP_SKIP_ARG:
				case OP_THEN_ARG:
					code += code[1];
					break;
			}
			code += PRIV(OP_lengths)[c];
		}
	}
}

static BOOL could_be_empty_branch(ssh_wcs code, ssh_wcs endcode, BOOL utf, compile_data *cd, recurse_check *recurses)
{
	register ssh_ws c;
	recurse_check this_recurse;
	for(code = first_significant_code(code + PRIV(OP_lengths)[*code], TRUE); code < endcode; code = first_significant_code(code + PRIV(OP_lengths)[c], TRUE))
	{
		const ssh_ws *ccode;
		c = *code;
		if(c == OP_ASSERT)
		{
			do code += GET(code, 1); while(*code == OP_ALT);
			c = *code;
			continue;
		}
		if(c == OP_RECURSE)
		{
			const ssh_ws *scode = cd->start_code + GET(code, 1);
			const ssh_ws *endgroup = scode;
			BOOL empty_branch;
			if(cd->start_workspace != NULL)
			{
				const ssh_ws *tcode;
				for(tcode = cd->start_workspace; tcode < cd->hwm; tcode += LINK_SIZE) if((ssh_l)GET(tcode, 0) == (ssh_l)(code + 1 - cd->start_code)) return TRUE;
				if(GET(scode, 1) == 0) return TRUE;
			}
			do endgroup += GET(endgroup, 1); while(*endgroup == OP_ALT);
			if(code >= scode && code <= endgroup) continue;
			else
			{
				recurse_check *r = recurses;
				for(r = recurses; r != NULL; r = r->prev)
					if(r->group == scode) break;
				if(r != NULL) continue;
			}
			empty_branch = FALSE;
			this_recurse.prev = recurses;
			this_recurse.group = scode;

			do
			{
				if(could_be_empty_branch(scode, endcode, utf, cd, &this_recurse))
				{
					empty_branch = TRUE;
					break;
				}
				scode += GET(scode, 1);
			} while(*scode == OP_ALT);

			if(!empty_branch) return FALSE;
			continue;
		}
		if(c == OP_BRAZERO || c == OP_BRAMINZERO || c == OP_SKIPZERO || c == OP_BRAPOSZERO)
		{
			code += PRIV(OP_lengths)[c];
			do code += GET(code, 1); while(*code == OP_ALT);
			c = *code;
			continue;
		}
		if(c == OP_SBRA || c == OP_SBRAPOS || c == OP_SCBRA || c == OP_SCBRAPOS)
		{
			do code += GET(code, 1); while(*code == OP_ALT);
			c = *code;
			continue;
		}
		if(c == OP_BRA || c == OP_BRAPOS || c == OP_CBRA || c == OP_CBRAPOS || c == OP_ONCE || c == OP_ONCE_NC || c == OP_COND)
		{
			BOOL empty_branch;
			if(GET(code, 1) == 0) return TRUE;
			if(c == OP_COND && code[GET(code, 1)] != OP_ALT) code += GET(code, 1);
			else
			{
				empty_branch = FALSE;
				do
				{
					if(!empty_branch && could_be_empty_branch(code, endcode, utf, cd, recurses)) empty_branch = TRUE;
					code += GET(code, 1);
				} while(*code == OP_ALT);
				if(!empty_branch) return FALSE;
			}
			c = *code;
			continue;
		}
		switch(c)
		{
			case OP_XCLASS:
				ccode = code += GET(code, 1);
				goto CHECK_CLASS_REPEAT;
			case OP_CLASS:
			case OP_NCLASS:
				ccode = code + PRIV(OP_lengths)[OP_CLASS];
CHECK_CLASS_REPEAT:
				switch(*ccode)
				{
					case OP_CRSTAR:
					case OP_CRMINSTAR:
					case OP_CRQUERY:
					case OP_CRMINQUERY:
					case OP_CRPOSSTAR:
					case OP_CRPOSQUERY:
						break;

					default:
					case OP_CRPLUS:
					case OP_CRMINPLUS:
					case OP_CRPOSPLUS:
						return FALSE;

					case OP_CRRANGE:
					case OP_CRMINRANGE:
					case OP_CRPOSRANGE:
						if(GET2(ccode, 1) > 0) return FALSE;
						break;
				}
				break;
			case OP_ANY:
			case OP_ALLANY:
			case OP_ANYBYTE:
			case OP_PROP:
			case OP_NOTPROP:
			case OP_ANYNL:
			case OP_NOT_HSPACE:
			case OP_HSPACE:
			case OP_NOT_VSPACE:
			case OP_VSPACE:
			case OP_EXTUNI:
			case OP_NOT_DIGIT:
			case OP_DIGIT:
			case OP_NOT_WHITESPACE:
			case OP_WHITESPACE:
			case OP_NOT_WORDCHAR:
			case OP_WORDCHAR:
			case OP_CHAR:
			case OP_CHARI:
			case OP_NOT:
			case OP_NOTI:
			case OP_PLUS:
			case OP_PLUSI:
			case OP_MINPLUS:
			case OP_MINPLUSI:
			case OP_NOTPLUS:
			case OP_NOTPLUSI:
			case OP_NOTMINPLUS:
			case OP_NOTMINPLUSI:
			case OP_POSPLUS:
			case OP_POSPLUSI:
			case OP_NOTPOSPLUS:
			case OP_NOTPOSPLUSI:
			case OP_EXACT:
			case OP_EXACTI:
			case OP_NOTEXACT:
			case OP_NOTEXACTI:
			case OP_TYPEPLUS:
			case OP_TYPEMINPLUS:
			case OP_TYPEPOSPLUS:
			case OP_TYPEEXACT:
				return FALSE;
			case OP_TYPESTAR:
			case OP_TYPEMINSTAR:
			case OP_TYPEPOSSTAR:
			case OP_TYPEQUERY:
			case OP_TYPEMINQUERY:
			case OP_TYPEPOSQUERY:
				if(code[1] == OP_PROP || code[1] == OP_NOTPROP) code += 2;
				break;
			case OP_TYPEUPTO:
			case OP_TYPEMINUPTO:
			case OP_TYPEPOSUPTO:
				if(code[1 + IMM2_SIZE] == OP_PROP || code[1 + IMM2_SIZE] == OP_NOTPROP) code += 2;
				break;
			case OP_KET:
			case OP_KETRMAX:
			case OP_KETRMIN:
			case OP_KETRPOS:
			case OP_ALT:
				return TRUE;
			case OP_MARK:
			case OP_PRUNE_ARG:
			case OP_SKIP_ARG:
			case OP_THEN_ARG:
				code += code[1];
				break;
			default:
				break;
		}
	}

	return TRUE;
}

static BOOL could_be_empty(const ssh_ws *code, const ssh_ws *endcode, branch_chain *bcptr, BOOL utf, compile_data *cd)
{
	while(bcptr != NULL && bcptr->current_branch >= code)
	{
		if(!could_be_empty_branch(bcptr->current_branch, endcode, utf, cd, NULL)) return FALSE;
		bcptr = bcptr->outer;
	}
	return TRUE;
}

static ssh_ws get_repeat_base(ssh_ws c)
{
	return (c > OP_TYPEPOSUPTO) ? c : (c >= OP_TYPESTAR) ? OP_TYPESTAR : (c >= OP_NOTSTARI) ? OP_NOTSTARI : (c >= OP_NOTSTAR) ? OP_NOTSTAR : (c >= OP_STARI) ? OP_STARI : OP_STAR;
}

static const ssh_ws * get_chr_property_list(ssh_wcs code, BOOL utf, const ssh_b* fcc, ssh_u* list)
{
	ssh_ws c = *code;
	ssh_ws base;
	const ssh_ws *end;
	ssh_u chr;
	list[0] = c;
	list[1] = FALSE;
	code++;

	if(c >= OP_STAR && c <= OP_TYPEPOSUPTO)
	{
		base = get_repeat_base(c);
		c -= (base - OP_STAR);
		if(c == OP_UPTO || c == OP_MINUPTO || c == OP_EXACT || c == OP_POSUPTO) code += IMM2_SIZE;
		list[1] = (c != OP_PLUS && c != OP_MINPLUS && c != OP_EXACT && c != OP_POSPLUS);
		switch(base)
		{
			case OP_STAR:
				list[0] = OP_CHAR;
				break;
			case OP_STARI:
				list[0] = OP_CHARI;
				break;
			case OP_NOTSTAR:
				list[0] = OP_NOT;
				break;
			case OP_NOTSTARI:
				list[0] = OP_NOTI;
				break;
			case OP_TYPESTAR:
				list[0] = *code;
				code++;
				break;
		}
		c = (ssh_ws)list[0];
	}

	switch(c)
	{
		case OP_NOT_DIGIT:
		case OP_DIGIT:
		case OP_NOT_WHITESPACE:
		case OP_WHITESPACE:
		case OP_NOT_WORDCHAR:
		case OP_WORDCHAR:
		case OP_ANY:
		case OP_ALLANY:
		case OP_ANYNL:
		case OP_NOT_HSPACE:
		case OP_HSPACE:
		case OP_NOT_VSPACE:
		case OP_VSPACE:
		case OP_EXTUNI:
		case OP_EODN:
		case OP_EOD:
		case OP_DOLL:
		case OP_DOLLM:
			return code;
		case OP_CHAR:
		case OP_NOT:
			GETCHARINCTEST(chr, code);
			list[2] = chr;
			list[3] = NOTACHAR;
			return code;
		case OP_CHARI:
		case OP_NOTI:
			list[0] = (c == OP_CHARI) ? OP_CHAR : OP_NOT;
			GETCHARINCTEST(chr, code);
			list[2] = chr;
			list[3] = (chr < 256) ? fcc[chr] : chr;
			if(chr == list[3]) list[3] = NOTACHAR; else list[4] = NOTACHAR;
			return code;
		case OP_NCLASS:
		case OP_CLASS:
		case OP_XCLASS:
			end = ((c == OP_XCLASS) ? code + GET(code, 0) - 1 : code + 32 / sizeof(ssh_ws));
			switch(*end)
			{
				case OP_CRSTAR:
				case OP_CRMINSTAR:
				case OP_CRQUERY:
				case OP_CRMINQUERY:
				case OP_CRPOSSTAR:
				case OP_CRPOSQUERY:
					list[1] = TRUE;
					end++;
					break;
				case OP_CRPLUS:
				case OP_CRMINPLUS:
				case OP_CRPOSPLUS:
					end++;
					break;
				case OP_CRRANGE:
				case OP_CRMINRANGE:
				case OP_CRPOSRANGE:
					list[1] = (GET2(end, 1) == 0);
					end += 1 + 2 * IMM2_SIZE;
					break;
			}
			list[2] = (ssh_u)(end - code);
			return end;
	}
	return NULL;
}

static BOOL compare_opcodes(ssh_wcs code, BOOL utf, const compile_data *cd, const ssh_u* base_list, ssh_wcs base_end, ssh_l *rec_limit)
{
	ssh_ws c;
	ssh_u list[8];
	const ssh_u *chr_ptr;
	const ssh_u *ochr_ptr;
	const ssh_u *list_ptr;
	const ssh_ws *next_code;
	const ssh_ws *xclass_flags;
	const ssh_b *class_bitset;
	const ssh_b *set1, *set2, *set_end;
	ssh_u chr;
	BOOL accepted, invert_bits;
	BOOL entered_a_group = FALSE;

	if(*rec_limit == 0) return FALSE;
	--(*rec_limit);
	for(;;)
	{
		c = *code;
		if(c == OP_CALLOUT)
		{
			code += PRIV(OP_lengths)[c];
			continue;
		}
		if(c == OP_ALT)
		{
			do code += GET(code, 1); while(*code == OP_ALT);
			c = *code;
		}
		switch(c)
		{
			case OP_END:
			case OP_KETRPOS:
				return base_list[1] != 0;
			case OP_KET:
				if(base_list[1] == 0) return FALSE;
				switch(*(code - GET(code, 1)))
				{
					case OP_ASSERT:
					case OP_ASSERT_NOT:
					case OP_ASSERTBACK:
					case OP_ASSERTBACK_NOT:
					case OP_ONCE:
					case OP_ONCE_NC:
						return !entered_a_group;
				}
				code += PRIV(OP_lengths)[c];
				continue;
			case OP_ONCE:
			case OP_ONCE_NC:
			case OP_BRA:
			case OP_CBRA:
				next_code = code + GET(code, 1);
				code += PRIV(OP_lengths)[c];
				while(*next_code == OP_ALT)
				{
					if(!compare_opcodes(code, utf, cd, base_list, base_end, rec_limit))
						return FALSE;
					code = next_code + 1 + LINK_SIZE;
					next_code += GET(next_code, 1);
				}
				entered_a_group = TRUE;
				continue;
			case OP_BRAZERO:
			case OP_BRAMINZERO:
				next_code = code + 1;
				if(*next_code != OP_BRA && *next_code != OP_CBRA && *next_code != OP_ONCE && *next_code != OP_ONCE_NC) return FALSE;
				do next_code += GET(next_code, 1); while(*next_code == OP_ALT);
				next_code += 1 + LINK_SIZE;
				if(!compare_opcodes(next_code, utf, cd, base_list, base_end, rec_limit))
					return FALSE;
				code += PRIV(OP_lengths)[c];
				continue;
			default:
				break;
		}
		code = get_chr_property_list(code, utf, cd->fcc, list);
		if(code == NULL) return FALSE;
		if(base_list[0] == OP_CHAR)
		{
			chr_ptr = base_list + 2;
			list_ptr = list;
		}
		else if(list[0] == OP_CHAR)
		{
			chr_ptr = list + 2;
			list_ptr = base_list;
		}
		else if(base_list[0] == OP_CLASS || list[0] == OP_CLASS)
		{
			if(base_list[0] == OP_CLASS)
			{
				set1 = (ssh_b *)(base_end - base_list[2]);
				list_ptr = list;
			}
			else
			{
				set1 = (ssh_b *)(code - list[2]);
				list_ptr = base_list;
			}
			invert_bits = FALSE;
			switch(list_ptr[0])
			{
				case OP_CLASS:
				case OP_NCLASS:
					set2 = (ssh_b *)((list_ptr == list ? code : base_end) - list_ptr[2]);
					break;
				case OP_XCLASS:
					xclass_flags = (list_ptr == list ? code : base_end) - list_ptr[2] + LINK_SIZE;
					if((*xclass_flags & XCL_HASPROP) != 0) return FALSE;
					if((*xclass_flags & XCL_MAP) == 0)
					{
						if(list[1] == 0) return TRUE;
						continue;
					}
					set2 = (ssh_b *)(xclass_flags + 1);
					break;
				case OP_NOT_DIGIT:
					invert_bits = TRUE;
				case OP_DIGIT:
					set2 = (ssh_b *)(cd->cbits + cbit_digit);
					break;
				case OP_NOT_WHITESPACE:
					invert_bits = TRUE;
				case OP_WHITESPACE:
					set2 = (ssh_b *)(cd->cbits + cbit_space);
					break;
				case OP_NOT_WORDCHAR:
					invert_bits = TRUE;
				case OP_WORDCHAR:
					set2 = (ssh_b *)(cd->cbits + cbit_word);
					break;
				default:
					return FALSE;
			}
			set_end = set1 + 32;
			if(invert_bits)
			{
				do
				{
					if((*set1++ & ~(*set2++)) != 0) return FALSE;
				} while(set1 < set_end);
			}
			else
			{
				do
				{
					if((*set1++ & *set2++) != 0) return FALSE;
				} while(set1 < set_end);
			}

			if(list[1] == 0) return TRUE;
			continue;
		}
		else
		{
			ssh_u leftop, rightop;
			leftop = base_list[0];
			rightop = list[0];
			accepted = leftop >= FIRST_AUTOTAB_OP && leftop <= LAST_AUTOTAB_LEFT_OP && rightop >= FIRST_AUTOTAB_OP && rightop <= LAST_AUTOTAB_RIGHT_OP && autoposstab[leftop - FIRST_AUTOTAB_OP][rightop - FIRST_AUTOTAB_OP];
			if(!accepted) return FALSE;
			if(list[1] == 0) return TRUE;
			continue;
		}
		do
		{
			chr = *chr_ptr;
			switch(list_ptr[0])
			{
				case OP_CHAR:
					ochr_ptr = list_ptr + 2;
					do
					{
						if(chr == *ochr_ptr) return FALSE;
						ochr_ptr++;
					} while(*ochr_ptr != NOTACHAR);
					break;
				case OP_NOT:
					ochr_ptr = list_ptr + 2;
					do
					{
						if(chr == *ochr_ptr) break;
						ochr_ptr++;
					} while(*ochr_ptr != NOTACHAR);
					if(*ochr_ptr == NOTACHAR) return FALSE;
					break;
				case OP_DIGIT:
					if(chr < 256 && (cd->ctypes[chr] & ctype_digit) != 0) return FALSE;
					break;
				case OP_NOT_DIGIT:
					if(chr > 255 || (cd->ctypes[chr] & ctype_digit) == 0) return FALSE;
					break;
				case OP_WHITESPACE:
					if(chr < 256 && (cd->ctypes[chr] & ctype_space) != 0) return FALSE;
					break;
				case OP_NOT_WHITESPACE:
					if(chr > 255 || (cd->ctypes[chr] & ctype_space) == 0) return FALSE;
					break;
				case OP_WORDCHAR:
					if(chr < 255 && (cd->ctypes[chr] & ctype_word) != 0) return FALSE;
					break;
				case OP_NOT_WORDCHAR:
					if(chr > 255 || (cd->ctypes[chr] & ctype_word) == 0) return FALSE;
					break;
				case OP_HSPACE:
					switch(chr)
					{
HSPACE_CASES: return FALSE;
						default: break;
					}
					break;

				case OP_NOT_HSPACE:
					switch(chr)
					{
HSPACE_CASES: break;
						default: return FALSE;
					}
					break;

				case OP_ANYNL:
				case OP_VSPACE:
					switch(chr)
					{
VSPACE_CASES: return FALSE;
						default: break;
					}
					break;

				case OP_NOT_VSPACE:
					switch(chr)
					{
VSPACE_CASES: break;
						default: return FALSE;
					}
					break;

				case OP_DOLL:
				case OP_EODN:
					switch(chr)
					{
						case CHAR_CR:
						case CHAR_LF:
						case CHAR_VT:
						case CHAR_FF:
						case CHAR_NEL:
						case 0x2028:
						case 0x2029:
							return FALSE;
					}
					break;

				case OP_EOD:
					break;
				case OP_NCLASS:
					if(chr > 255) return FALSE;
				case OP_CLASS:
					if(chr > 255) break;
					class_bitset = (ssh_b *)((list_ptr == list ? code : base_end) - list_ptr[2]);
					if((class_bitset[chr >> 3] & (1 << (chr & 7))) != 0) return FALSE;
					break;
				case OP_XCLASS:
					if(PRIV(xclass)(chr, (list_ptr == list ? code : base_end) - list_ptr[2] + LINK_SIZE, utf)) return FALSE;
					break;
				default:
					return FALSE;
			}
			chr_ptr++;
		} while(*chr_ptr != NOTACHAR);
		if(list[1] == 0) return TRUE;
	}
}

static void auto_possessify(ssh_ws *code, BOOL utf, const compile_data *cd)
{
	register ssh_ws c;
	const ssh_ws *end;
	ssh_ws *repeat_opcode;
	ssh_u list[8];
	ssh_l rec_limit;

	for(;;)
	{
		c = *code;
		if(c >= OP_TABLE_LENGTH) return;
		if(c >= OP_STAR && c <= OP_TYPEPOSUPTO)
		{
			c -= get_repeat_base(c) - OP_STAR;
			end = (c <= OP_MINUPTO) ? get_chr_property_list(code, utf, cd->fcc, list) : NULL;
			list[1] = c == OP_STAR || c == OP_PLUS || c == OP_QUERY || c == OP_UPTO;
			rec_limit = 1000;
			if(end != NULL && compare_opcodes(end, utf, cd, list, end, &rec_limit))
			{
				switch(c)
				{
					case OP_STAR:
						*code += OP_POSSTAR - OP_STAR;
						break;
					case OP_MINSTAR:
						*code += OP_POSSTAR - OP_MINSTAR;
						break;
					case OP_PLUS:
						*code += OP_POSPLUS - OP_PLUS;
						break;
					case OP_MINPLUS:
						*code += OP_POSPLUS - OP_MINPLUS;
						break;
					case OP_QUERY:
						*code += OP_POSQUERY - OP_QUERY;
						break;
					case OP_MINQUERY:
						*code += OP_POSQUERY - OP_MINQUERY;
						break;
					case OP_UPTO:
						*code += OP_POSUPTO - OP_UPTO;
						break;
					case OP_MINUPTO:
						*code += OP_POSUPTO - OP_MINUPTO;
						break;
				}
			}
			c = *code;
		}
		else if(c == OP_CLASS || c == OP_NCLASS || c == OP_XCLASS)
		{
			repeat_opcode = ((c == OP_XCLASS) ? code + GET(code, 1) : code + 1 + (32 / sizeof(ssh_ws)));
			c = *repeat_opcode;
			if(c >= OP_CRSTAR && c <= OP_CRMINRANGE)
			{
				end = get_chr_property_list(code, utf, cd->fcc, list);
				list[1] = (c & 1) == 0;
				rec_limit = 1000;
				if(compare_opcodes(end, utf, cd, list, end, &rec_limit))
				{
					switch(c)
					{
						case OP_CRSTAR:
						case OP_CRMINSTAR:
							*repeat_opcode = OP_CRPOSSTAR;
							break;

						case OP_CRPLUS:
						case OP_CRMINPLUS:
							*repeat_opcode = OP_CRPOSPLUS;
							break;

						case OP_CRQUERY:
						case OP_CRMINQUERY:
							*repeat_opcode = OP_CRPOSQUERY;
							break;

						case OP_CRRANGE:
						case OP_CRMINRANGE:
							*repeat_opcode = OP_CRPOSRANGE;
							break;
					}
				}
			}
			c = *code;
		}

		switch(c)
		{
			case OP_END:
				return;

			case OP_TYPESTAR:
			case OP_TYPEMINSTAR:
			case OP_TYPEPLUS:
			case OP_TYPEMINPLUS:
			case OP_TYPEQUERY:
			case OP_TYPEMINQUERY:
			case OP_TYPEPOSSTAR:
			case OP_TYPEPOSPLUS:
			case OP_TYPEPOSQUERY:
				if(code[1] == OP_PROP || code[1] == OP_NOTPROP) code += 2;
				break;

			case OP_TYPEUPTO:
			case OP_TYPEMINUPTO:
			case OP_TYPEEXACT:
			case OP_TYPEPOSUPTO:
				if(code[1 + IMM2_SIZE] == OP_PROP || code[1 + IMM2_SIZE] == OP_NOTPROP)
					code += 2;
				break;
			case OP_XCLASS:
				code += GET(code, 1);
				break;
			case OP_MARK:
			case OP_PRUNE_ARG:
			case OP_SKIP_ARG:
			case OP_THEN_ARG:
				code += code[1];
				break;
		}
		code += PRIV(OP_lengths)[c];
	}
}

static BOOL check_posix_syntax(ssh_wcs ptr, ssh_wcs* endptr)
{
	ssh_ws terminator;
	terminator = *(++ptr);
	for(++ptr; *ptr != CHAR_NULL; ptr++)
	{
		if(*ptr == CHAR_BACKSLASH && ptr[1] == CHAR_RIGHT_SQUARE_BRACKET) ptr++;
		else if(*ptr == CHAR_RIGHT_SQUARE_BRACKET) return FALSE;
		else
		{
			if(*ptr == terminator && ptr[1] == CHAR_RIGHT_SQUARE_BRACKET)
			{
				*endptr = ptr;
				return TRUE;
			}
			if(*ptr == CHAR_LEFT_SQUARE_BRACKET && (ptr[1] == CHAR_COLON || ptr[1] == CHAR_DOT || ptr[1] == CHAR_EQUALS_SIGN) && check_posix_syntax(ptr, endptr))
			   return FALSE;
		}
	}
	return FALSE;
}

static ssh_l check_posix_name(ssh_wcs ptr, ssh_l len)
{
	ssh_ccs pn = posix_names;
	ssh_l yield = 0;
	while(posix_name_lengths[yield] != 0)
	{
		if(len == posix_name_lengths[yield] && STRNCMP_UC_C8(ptr, pn, (ssh_u)len) == 0) return yield;
		pn += posix_name_lengths[yield] + 1;
		yield++;
	}
	return -1;
}

static void adjust_recurse(ssh_ws *group, ssh_l adjust, BOOL utf, compile_data *cd, size_t save_hwm_offset)
{
	ssh_ws *ptr = group;

	while((ptr = (ssh_ws *)find_recurse(ptr, utf)) != NULL)
	{
		ssh_l offset;
		ssh_ws *hc;
		for(hc = (ssh_ws *)cd->start_workspace + save_hwm_offset; hc < cd->hwm; hc += LINK_SIZE)
		{
			offset = (ssh_l)GET(hc, 0);
			if(cd->start_code + offset == ptr + 1)
			{
				hc[0] = (ssh_ws)(offset + adjust);
				break;
			}
		}
		if(hc >= cd->hwm)
		{
			offset = (ssh_l)GET(ptr, 1);
			if(cd->start_code + offset >= group) ptr[1] = (ssh_ws)(offset + adjust);
		}
		ptr += 1 + LINK_SIZE;
	}
}

static ssh_ws* auto_callout(ssh_ws* code, ssh_wcs ptr, compile_data *cd)
{
	*code++ = OP_CALLOUT;
	*code++ = 255;
	code[0] = (ssh_ws)(ssh_l)(ptr - cd->start_pattern);
	code[LINK_SIZE] = 0;
	return code + 2 * LINK_SIZE;
}

static void complete_callout(ssh_ws *previous_callout, ssh_wcs ptr, compile_data *cd)
{
	ssh_l length = (ssh_l)(ptr - cd->start_pattern - GET(previous_callout, 2));
	previous_callout[2 + LINK_SIZE] = (ssh_ws)length;
}

static ssh_l add_to_class(ssh_b *classbits, ssh_ws **uchardptr, ssh_l options, compile_data *cd, ssh_u start, ssh_u end)
{
	ssh_u c;
	ssh_u classbits_end = (end <= 0xff ? end : 0xff);
	ssh_l n8 = 0;
	if((options & REGEX_CASELESS) != 0)
	{
			for(c = start; c <= classbits_end; c++)
			{
				SETBIT(classbits, cd->fcc[c]);
				n8++;
			}
	}
	if(end > 0xffff) end = 0xffff;
	for(c = start; c <= classbits_end; c++)
	{

		SETBIT(classbits, c);
		n8++;
	}
	if(start <= 0xff) start = 0xff + 1;
	if(end >= start)
	{
		ssh_ws *uchardata = *uchardptr;
		if(start < end)
		{
				*uchardata++ = XCL_RANGE;
				*uchardata++ = (ssh_ws)start;
				*uchardata++ = (ssh_ws)end;
			}
			else if(start == end)
			{
				*uchardata++ = XCL_SINGLE;
				*uchardata++ = (ssh_ws)start;
			}
		*uchardptr = uchardata;
	}
	return n8;
}

static ssh_l add_list_to_class(ssh_b *classbits, ssh_ws **uchardptr, ssh_l options, compile_data *cd, const ssh_u *p, ssh_u except)
{
	ssh_l n8 = 0;
	while(p[0] < NOTACHAR)
	{
		ssh_l n = 0;
		if(p[0] != except)
		{
			while(p[n + 1] == p[0] + n + 1) n++;
			n8 += add_to_class(classbits, uchardptr, options, cd, p[0], p[n]);
		}
		p += n + 1;
	}
	return n8;
}

static ssh_l add_not_list_to_class(ssh_b *classbits, ssh_ws **uchardptr, ssh_l options, compile_data *cd, const ssh_u *p)
{
	BOOL utf = (options & REGEX_UTF8) != 0;
	ssh_l n8 = 0;
	if(p[0] > 0) n8 += add_to_class(classbits, uchardptr, options, cd, 0, p[0] - 1);
	while(p[0] < NOTACHAR)
	{
		while(p[1] == p[0] + 1) p++;
		n8 += add_to_class(classbits, uchardptr, options, cd, p[0] + 1, (p[1] == NOTACHAR) ? (utf ? 0x10ffffu : 0xffffffffu) : p[1] - 1);
		p++;
	}
	return n8;
}

static BOOL compile_branch(ssh_l *optionsptr, ssh_ws **codeptr, ssh_wcs* ptrptr, ssh_l* errorcodeptr, ssh_u* firstcharptr, ssh_l* firstcharflagsptr, ssh_u* reqcharptr, ssh_l* reqcharflagsptr, branch_chain *bcptr, ssh_l cond_depth, compile_data *cd, ssh_l* lengthptr)
{
	ssh_l repeat_type, op_type;
	ssh_l repeat_min = 0, repeat_max = 0;
	ssh_l bravalue = 0;
	ssh_l greedy_default, greedy_non_default;
	ssh_u firstchar, reqchar;
	ssh_l firstcharflags, reqcharflags;
	ssh_u zeroreqchar, zerofirstchar;
	ssh_l zeroreqcharflags, zerofirstcharflags;
	ssh_l req_caseopt, reqvary, tempreqvary;
	ssh_l options = *optionsptr;
	ssh_l after_manual_callout = 0;
	ssh_l length_prevgroup = 0;
	register ssh_u c;
	ssh_l escape;
	register ssh_ws *code = *codeptr;
	ssh_ws *last_code = code;
	ssh_ws *orig_code = code;
	ssh_ws *tempcode;
	BOOL inescq = FALSE;
	BOOL groupsetfirstchar = FALSE;
	const ssh_ws *ptr = *ptrptr;
	const ssh_ws *tempptr;
	const ssh_ws *nestptr = NULL;
	ssh_ws *previous = NULL;
	ssh_ws *previous_callout = NULL;
	size_t save_hwm_offset = 0;
	ssh_b classbits[32];
	BOOL utf = FALSE;
	ssh_ws *class_uchardata;
	BOOL xclass;
	ssh_ws *class_uchardata_base;
	greedy_default = ((options & REGEX_UNGREEDY) != 0);
	greedy_non_default = greedy_default ^ 1;
	firstchar = reqchar = zerofirstchar = zeroreqchar = 0;
	firstcharflags = reqcharflags = zerofirstcharflags = zeroreqcharflags = REQ_UNSET;
	req_caseopt = ((options & REGEX_CASELESS) != 0) ? REQ_CASELESS : 0;
	for(;; ptr++)
	{
		BOOL negate_class;
		BOOL should_flip_negation;
		BOOL possessive_quantifier;
		BOOL is_quantifier;
		BOOL is_recurse;
		BOOL reset_bracount;
		ssh_l class_has_8bitchar;
		ssh_l class_one_char;
		BOOL xclass_has_prop;
		ssh_l newoptions;
		ssh_l recno;
		ssh_l refsign;
		ssh_l skipbytes;
		ssh_u subreqchar, subfirstchar;
		ssh_l subreqcharflags, subfirstcharflags;
		ssh_l terminator;
		ssh_u mclength;
		ssh_u tempbracount;
		ssh_u ec;
		ssh_ws mcbuffer[8];
		c = *ptr;
		if(c == CHAR_NULL && nestptr != NULL)
		{
			ptr = nestptr;
			nestptr = NULL;
			c = *ptr;
		}
		if(lengthptr != NULL)
		{
			if(code > cd->start_workspace + cd->workspace_size - WORK_SIZE_SAFETY_MARGIN)
			{
				*errorcodeptr = ERR52;
				goto FAILED;
			}
			if(code < last_code) code = last_code;
			if(OFLOW_MAX - *lengthptr < code - last_code)
			{
				*errorcodeptr = ERR20;
				goto FAILED;
			}
			*lengthptr += (ssh_l)(code - last_code);
			if(previous != NULL)
			{
				if(previous > orig_code)
				{
					memmove(orig_code, previous, IN_UCHARS(code - previous));
					code -= previous - orig_code;
					previous = orig_code;
				}
			}
			else code = orig_code;
			last_code = code;
		}
		else if(cd->hwm > cd->start_workspace + cd->workspace_size - WORK_SIZE_SAFETY_MARGIN)
		{
			*errorcodeptr = ERR52;
			goto FAILED;
		}
		if(inescq && c != CHAR_NULL)
		{
			if(c == CHAR_BACKSLASH && ptr[1] == CHAR_E)
			{
				inescq = FALSE;
				ptr++;
				continue;
			}
			else
			{
				if(previous_callout != NULL)
				{
					if(lengthptr == NULL) complete_callout(previous_callout, ptr, cd);
					previous_callout = NULL;
				}
				if((options & REGEX_AUTO_CALLOUT) != 0)
				{
					previous_callout = code;
					code = auto_callout(code, ptr, cd);
				}
				goto NORMAL_CHAR;
			}

		}
		if((options & REGEX_EXTENDED) != 0)
		{
			for(;;)
			{
				while(MAX_255(c) && (cd->ctypes[c] & ctype_space) != 0) c = *(++ptr);
				if(c != CHAR_NUMBER_SIGN) break;
				ptr++;
				while(*ptr != CHAR_NULL)
				{
					if(IS_NEWLINE(ptr))
					{
						ptr += cd->nllen;
						break;
					}
					ptr++;
				}
				c = *ptr;
			}
		}
		is_quantifier = c == CHAR_ASTERISK || c == CHAR_PLUS || c == CHAR_QUESTION_MARK || (c == CHAR_LEFT_CURLY_BRACKET && is_counted_repeat(ptr + 1));
		if(!is_quantifier && previous_callout != NULL && nestptr == NULL && after_manual_callout-- <= 0)
		{
			if(lengthptr == NULL) complete_callout(previous_callout, ptr, cd);
			previous_callout = NULL;
		}
		if((options & REGEX_AUTO_CALLOUT) != 0 && !is_quantifier && nestptr == NULL)
		{
			previous_callout = code;
			code = auto_callout(code, ptr, cd);
		}
		switch(c)
		{

			case CHAR_NULL:
			case CHAR_VERTICAL_LINE:
			case CHAR_RIGHT_PARENTHESIS:
				*firstcharptr = firstchar;
				*firstcharflagsptr = firstcharflags;
				*reqcharptr = reqchar;
				*reqcharflagsptr = reqcharflags;
				*codeptr = code;
				*ptrptr = ptr;
				if(lengthptr != NULL)
				{
					if(OFLOW_MAX - *lengthptr < code - last_code)
					{
						*errorcodeptr = ERR20;
						goto FAILED;
					}
					*lengthptr += (ssh_l)(code - last_code);
				}
				return TRUE;
			case CHAR_CIRCUMFLEX_ACCENT:
				previous = NULL;
				if((options & REGEX_MULTILINE) != 0)
				{
					if(firstcharflags == REQ_UNSET) zerofirstcharflags = firstcharflags = REQ_NONE;
					*code++ = OP_CIRCM;
				}
				else *code++ = OP_CIRC;
				break;
			case CHAR_DOLLAR_SIGN:
				previous = NULL;
				*code++ = ((options & REGEX_MULTILINE) != 0) ? OP_DOLLM : OP_DOLL;
				break;
			case CHAR_DOT:
				if(firstcharflags == REQ_UNSET) firstcharflags = REQ_NONE;
				zerofirstchar = firstchar;
				zerofirstcharflags = firstcharflags;
				zeroreqchar = reqchar;
				zeroreqcharflags = reqcharflags;
				previous = code;
				*code++ = ((options & REGEX_DOTALL) != 0) ? OP_ALLANY : OP_ANY;
				break;
			case CHAR_RIGHT_SQUARE_BRACKET:
				if((cd->external_options & REGEX_JAVASCRIPT_COMPAT) != 0)
				{
					*errorcodeptr = ERR64;
					goto FAILED;
				}
				goto NORMAL_CHAR;
			case CHAR_LEFT_SQUARE_BRACKET:
				if(STRNCMP_UC_C8(ptr + 1, STRING_WEIRD_STARTWORD, 6) == 0)
				{
					nestptr = ptr + 7;
					ptr = sub_start_of_word - 1;
					continue;
				}
				if(STRNCMP_UC_C8(ptr + 1, STRING_WEIRD_ENDWORD, 6) == 0)
				{
					nestptr = ptr + 7;
					ptr = sub_end_of_word - 1;
					continue;
				}
				previous = code;
				if((ptr[1] == CHAR_COLON || ptr[1] == CHAR_DOT || ptr[1] == CHAR_EQUALS_SIGN) && check_posix_syntax(ptr, &tempptr))
				{
					*errorcodeptr = (ptr[1] == CHAR_COLON) ? ERR13 : ERR31;
					goto FAILED;
				}
				negate_class = FALSE;
				for(;;)
				{
					c = *(++ptr);
					if(c == CHAR_BACKSLASH)
					{
						if(ptr[1] == CHAR_E) ptr++;
						else if(STRNCMP_UC_C8(ptr + 1, STR_Q STR_BACKSLASH STR_E, 3) == 0) ptr += 3;
						else break;
					}
					else if(!negate_class && c == CHAR_CIRCUMFLEX_ACCENT)
						negate_class = TRUE;
					else break;
				}
				if(c == CHAR_RIGHT_SQUARE_BRACKET && (cd->external_options & REGEX_JAVASCRIPT_COMPAT) != 0)
				{
					*code++ = negate_class ? OP_ALLANY : OP_FAIL;
					if(firstcharflags == REQ_UNSET) firstcharflags = REQ_NONE;
					zerofirstchar = firstchar;
					zerofirstcharflags = firstcharflags;
					break;
				}
				should_flip_negation = FALSE;
				xclass = FALSE;
				class_uchardata = code + LINK_SIZE + 2;
				class_uchardata_base = class_uchardata;
				class_has_8bitchar = 0;
				class_one_char = 0;
				xclass_has_prop = FALSE;
				memset(classbits, 0, 32 * sizeof(ssh_b));
				if(c != CHAR_NULL) do
				{
					const ssh_ws *oldptr;
					if(lengthptr != NULL && class_uchardata > class_uchardata_base)
					{
						xclass = TRUE;
						*lengthptr += (ssh_l)(class_uchardata - class_uchardata_base);
						class_uchardata = class_uchardata_base;
					}
					if(inescq)
					{
						if(c == CHAR_BACKSLASH && ptr[1] == CHAR_E)
						{
							inescq = FALSE;
							ptr++;
							continue;
						}
						goto CHECK_RANGE;
					}
					if(c == CHAR_LEFT_SQUARE_BRACKET && (ptr[1] == CHAR_COLON || ptr[1] == CHAR_DOT || ptr[1] == CHAR_EQUALS_SIGN) && check_posix_syntax(ptr, &tempptr))
					{
						BOOL local_negate = FALSE;
						ssh_l posix_class, taboffset, tabopt;
						register const ssh_b *cbits = cd->cbits;
						ssh_b pbits[32];
						if(ptr[1] != CHAR_COLON)
						{
							*errorcodeptr = ERR31;
							goto FAILED;
						}
						ptr += 2;
						if(*ptr == CHAR_CIRCUMFLEX_ACCENT)
						{
							local_negate = TRUE;
							should_flip_negation = TRUE;
							ptr++;
						}
						posix_class = check_posix_name(ptr, (ssh_l)(tempptr - ptr));
						if(posix_class < 0)
						{
							*errorcodeptr = ERR30;
							goto FAILED;
						}
						if((options & REGEX_CASELESS) != 0 && posix_class <= 2) posix_class = 0;
						posix_class *= 3;
						memcpy(pbits, cbits + posix_class_maps[posix_class], 32 * sizeof(ssh_b));
						taboffset = posix_class_maps[posix_class + 1];
						tabopt = posix_class_maps[posix_class + 2];

						if(taboffset >= 0)
						{
							if(tabopt >= 0)
								for(c = 0; c < 32; c++) pbits[c] |= cbits[c + taboffset];
							else
								for(c = 0; c < 32; c++) pbits[c] &= ~cbits[c + taboffset];
						}
						if(tabopt < 0) tabopt = -tabopt;
						if(tabopt == 1) pbits[1] &= ~0x3c;
						else if(tabopt == 2) pbits[11] &= 0x7f;
						if(local_negate)
							for(c = 0; c < 32; c++) classbits[c] |= ~pbits[c];
						else
							for(c = 0; c < 32; c++) classbits[c] |= pbits[c];
						ptr = tempptr + 1;
						class_has_8bitchar = 1;
						class_one_char = 2;
						continue;
					}
					if(c == CHAR_BACKSLASH)
					{
						escape = check_escape(&ptr, &ec, errorcodeptr, cd->bracount, options, TRUE);
						if(*errorcodeptr != 0) goto FAILED;
						if(escape == 0) c = ec;
						else if(escape == ESC_b) c = CHAR_BS;
						else if(escape == ESC_N)
						{
							*errorcodeptr = ERR71;
							goto FAILED;
						}
						else if(escape == ESC_Q)
						{
							if(ptr[1] == CHAR_BACKSLASH && ptr[2] == CHAR_E)
							{
								ptr += 2;
							}
							else inescq = TRUE;
							continue;
						}
						else if(escape == ESC_E) continue;

						else
						{
							register const ssh_b *cbits = cd->cbits;

							class_has_8bitchar++;

							class_one_char += 2;

							switch(escape)
							{
								case ESC_d:
									for(c = 0; c < 32; c++) classbits[c] |= cbits[c + cbit_digit];
									continue;
								case ESC_D:
									should_flip_negation = TRUE;
									for(c = 0; c < 32; c++) classbits[c] |= ~cbits[c + cbit_digit];
									continue;
								case ESC_w:
									for(c = 0; c < 32; c++) classbits[c] |= cbits[c + cbit_word];
									continue;
								case ESC_W:
									should_flip_negation = TRUE;
									for(c = 0; c < 32; c++) classbits[c] |= ~cbits[c + cbit_word];
									continue;
								case ESC_s:
									for(c = 0; c < 32; c++) classbits[c] |= cbits[c + cbit_space];
									continue;
								case ESC_S:
									should_flip_negation = TRUE;
									for(c = 0; c < 32; c++) classbits[c] |= ~cbits[c + cbit_space];
									continue;
								case ESC_h:
									(void)add_list_to_class(classbits, &class_uchardata, options, cd, PRIV(hspace_list), NOTACHAR);
									continue;
								case ESC_H:
									(void)add_not_list_to_class(classbits, &class_uchardata, options, cd, PRIV(hspace_list));
									continue;
								case ESC_v:
									(void)add_list_to_class(classbits, &class_uchardata, options, cd, PRIV(vspace_list), NOTACHAR);
									continue;
								case ESC_V:
									(void)add_not_list_to_class(classbits, &class_uchardata, options, cd, PRIV(vspace_list));
									continue;
								default:
									if((options & REGEX_EXTRA) != 0)
									{
										*errorcodeptr = ERR7;
										goto FAILED;
									}
									class_has_8bitchar--;
									class_one_char -= 2;
									c = *ptr;
									break;
							}
						}



						escape = 0;

					}



CHECK_RANGE:
					while(ptr[1] == CHAR_BACKSLASH && ptr[2] == CHAR_E)
					{
						inescq = FALSE;
						ptr += 2;
					}
					oldptr = ptr;



					if(c == CHAR_CR || c == CHAR_NL) cd->external_flags |= REGEX_HASCRORLF;



					if(!inescq && ptr[1] == CHAR_MINUS)
					{
						ssh_u d;
						ptr += 2;
						while(*ptr == CHAR_BACKSLASH && ptr[1] == CHAR_E) ptr += 2;



						while(*ptr == CHAR_BACKSLASH && ptr[1] == CHAR_Q)
						{
							ptr += 2;
							if(*ptr == CHAR_BACKSLASH && ptr[1] == CHAR_E)
							{ ptr += 2; continue; }
							inescq = TRUE;
							break;
						}



						if(*ptr == CHAR_NULL || (!inescq && *ptr == CHAR_RIGHT_SQUARE_BRACKET))
						{
							ptr = oldptr;
							goto CLASS_SINGLE_CHARACTER;
						}
						d = *ptr;
						if(!inescq)
						{
							if(d == CHAR_BACKSLASH)
							{
								ssh_l descape;
								descape = check_escape(&ptr, &d, errorcodeptr, cd->bracount, options, TRUE);
								if(*errorcodeptr != 0) goto FAILED;
								if(descape != 0)
								{
									if(descape == ESC_b) d = CHAR_BS; else
									{
										*errorcodeptr = ERR83;
										goto FAILED;
									}
								}
							}
							else if(d == CHAR_LEFT_SQUARE_BRACKET && (ptr[1] == CHAR_COLON || ptr[1] == CHAR_DOT || ptr[1] == CHAR_EQUALS_SIGN) && check_posix_syntax(ptr, &tempptr))
							{
								*errorcodeptr = ERR83;
								goto FAILED;
							}
						}



						if(d < c)
						{
							*errorcodeptr = ERR8;
							goto FAILED;
						}
						if(d == c) goto CLASS_SINGLE_CHARACTER;
						class_one_char = 2;
						if(d == CHAR_CR || d == CHAR_NL) cd->external_flags |= REGEX_HASCRORLF;
						class_has_8bitchar += add_to_class(classbits, &class_uchardata, options, cd, c, d);
						continue;
					}
CLASS_SINGLE_CHARACTER:
					if(class_one_char < 2) class_one_char++;
					if(!inescq && class_one_char == 1 && ptr[1] == CHAR_RIGHT_SQUARE_BRACKET)
					{
						ptr++;
						zeroreqchar = reqchar;
						zeroreqcharflags = reqcharflags;

						if(negate_class)
						{
							if(firstcharflags == REQ_UNSET) firstcharflags = REQ_NONE;
							zerofirstchar = firstchar;
							zerofirstcharflags = firstcharflags;
							*code++ = ((options & REGEX_CASELESS) != 0) ? OP_NOTI : OP_NOT;
							*code++ = (ssh_ws)c;
							goto END_CLASS;
						}
						mcbuffer[0] = (ssh_ws)c;
						mclength = 1;
						goto ONE_CHAR;
					}
					class_has_8bitchar += add_to_class(classbits, &class_uchardata, options, cd, c, c);
				}
				while(((c = *(++ptr)) != CHAR_NULL ||
				(nestptr != NULL &&
				(ptr = nestptr, nestptr = NULL, c = *(++ptr)) != CHAR_NULL)) &&
				(c != CHAR_RIGHT_SQUARE_BRACKET || inescq));
				if(c == CHAR_NULL)
				{
					*errorcodeptr = ERR6;
					goto FAILED;
				}
				if(class_uchardata > class_uchardata_base) xclass = TRUE;
				if(firstcharflags == REQ_UNSET) firstcharflags = REQ_NONE;
				zerofirstchar = firstchar;
				zerofirstcharflags = firstcharflags;
				zeroreqchar = reqchar;
				zeroreqcharflags = reqcharflags;
				if(xclass && !should_flip_negation)
				{
					*class_uchardata++ = XCL_END;
					*code++ = OP_XCLASS;
					code += LINK_SIZE;
					*code = negate_class ? XCL_NOT : 0;
					if(xclass_has_prop) *code |= XCL_HASPROP;
					if(class_has_8bitchar > 0)
					{
						*code++ |= XCL_MAP;
						memmove(code + (32 / sizeof(ssh_ws)), code, IN_UCHARS(class_uchardata - code));
						if(negate_class && !xclass_has_prop)
							for(c = 0; c < 32; c++) classbits[c] = ~classbits[c];
						memcpy(code, classbits, 32);
						code = class_uchardata + (32 / sizeof(ssh_ws));
					}
					else code = class_uchardata;
					previous[1] = (ssh_ws)(ssh_l)(code - previous);
					break;
				}
				if(lengthptr != NULL) *lengthptr += (ssh_l)(class_uchardata - class_uchardata_base);
				*code++ = (negate_class == should_flip_negation) ? OP_CLASS : OP_NCLASS;
				if(lengthptr == NULL)
				{
					if(negate_class) for(c = 0; c < 32; c++) classbits[c] = ~classbits[c];
					memcpy(code, classbits, 32);
				}
				code += 32 / sizeof(ssh_ws);
END_CLASS:
				break;
			case CHAR_LEFT_CURLY_BRACKET:
				if(!is_quantifier) goto NORMAL_CHAR;
				ptr = read_repeat_counts(ptr + 1, &repeat_min, &repeat_max, errorcodeptr);
				if(*errorcodeptr != 0) goto FAILED;
				goto REPEAT;
			case CHAR_ASTERISK:
				repeat_min = 0;
				repeat_max = -1;
				goto REPEAT;
			case CHAR_PLUS:
				repeat_min = 1;
				repeat_max = -1;
				goto REPEAT;
			case CHAR_QUESTION_MARK:
				repeat_min = 0;
				repeat_max = 1;

REPEAT:
				if(previous == NULL)
				{
					*errorcodeptr = ERR9;
					goto FAILED;
				}

				if(repeat_min == 0)
				{
					firstchar = zerofirstchar;
					firstcharflags = zerofirstcharflags;
					reqchar = zeroreqchar;
					reqcharflags = zeroreqcharflags;
				}
				reqvary = (repeat_min == repeat_max) ? 0 : REQ_VARY;
				op_type = 0;
				possessive_quantifier = FALSE;
				tempcode = previous;
				if((options & REGEX_EXTENDED) != 0)
				{
					const ssh_ws *p = ptr + 1;
					for(;;)
					{
						while(MAX_255(*p) && (cd->ctypes[*p] & ctype_space) != 0) p++;
						if(*p != CHAR_NUMBER_SIGN) break;
						p++;
						while(*p != CHAR_NULL)
						{
							if(IS_NEWLINE(p))
							{
								p += cd->nllen;
								break;
							}
							p++;
						}
					}
					ptr = p - 1;
				}



				if(ptr[1] == CHAR_PLUS)
				{
					repeat_type = 0;
					possessive_quantifier = TRUE;
					ptr++;
				}
				else if(ptr[1] == CHAR_QUESTION_MARK)
				{
					repeat_type = greedy_non_default;
					ptr++;
				}
				else repeat_type = greedy_default;
				if(*previous == OP_RECURSE)
				{
					memmove(previous + 1 + LINK_SIZE, previous, IN_UCHARS(1 + LINK_SIZE));
					*previous = OP_ONCE;
					previous[1] = (2 + 2 * LINK_SIZE);
					previous[2 + 2 * LINK_SIZE] = OP_KET;
					previous[3 + 2 * LINK_SIZE] =  (2 + 2 * LINK_SIZE);
					code += 2 + 2 * LINK_SIZE;
					length_prevgroup = 3 + 3 * LINK_SIZE;
					if(lengthptr == NULL && cd->hwm >= cd->start_workspace + LINK_SIZE)
					{
						ssh_l offset = GET(cd->hwm, -LINK_SIZE);
						if(offset == previous + 1 - cd->start_code)
							cd->hwm[-LINK_SIZE] = (ssh_ws)(offset + 1 + LINK_SIZE);
					}
				}
				if(*previous == OP_CHAR || *previous == OP_CHARI || *previous == OP_NOT || *previous == OP_NOTI)
				{
					switch(*previous)
					{
						default:
						case OP_CHAR:  op_type = OP_STAR - OP_STAR; break;
						case OP_CHARI: op_type = OP_STARI - OP_STAR; break;
						case OP_NOT:   op_type = OP_NOTSTAR - OP_STAR; break;
						case OP_NOTI:  op_type = OP_NOTSTARI - OP_STAR; break;
					}
					c = code[-1];
					if(*previous <= OP_CHARI && repeat_min > 1)
					{
						reqchar = c;
						reqcharflags = req_caseopt | cd->req_varyopt;
					}
					goto OUTPUT_SINGLE_REPEAT;
				}
				else if(*previous < OP_EODN)
				{
					ssh_ws *oldcode;
					ssh_l prop_type, prop_value;
					op_type = OP_TYPESTAR - OP_STAR;
					c = *previous;
OUTPUT_SINGLE_REPEAT:
					if(*previous == OP_PROP || *previous == OP_NOTPROP)
					{
						prop_type = previous[1];
						prop_value = previous[2];
					}
					else prop_type = prop_value = -1;

					oldcode = code;
					code = previous;
					if(repeat_max == 0) goto END_REPEAT;
					repeat_type += op_type;
					if(repeat_min == 0)
					{
						if(repeat_max == -1) *code++ = (ssh_ws)(OP_STAR + repeat_type);
						else if(repeat_max == 1) *code++ = (ssh_ws)(OP_QUERY + repeat_type);
						else
						{
							*code++ = (ssh_ws)(OP_UPTO + repeat_type);
							PUT2INC(code, 0, repeat_max);
						}
					}
					else if(repeat_min == 1)
					{
						if(repeat_max == -1) *code++ = (ssh_ws)(OP_PLUS + repeat_type);
						else
						{
							code = oldcode;
							if(repeat_max == 1) goto END_REPEAT;
							*code++ = (ssh_ws)(OP_UPTO + repeat_type);
							PUT2INC(code, 0, repeat_max - 1);
						}
					}
					else
					{
						*code++ = (ssh_ws)(OP_EXACT + op_type);
						PUT2INC(code, 0, repeat_min);
						if(repeat_max < 0)
						{
							*code++ = (ssh_ws)c;
							if(prop_type >= 0)
							{
								*code++ = (ssh_ws)prop_type;
								*code++ = (ssh_ws)prop_value;
							}
							*code++ = (ssh_ws)(OP_STAR + repeat_type);
						}
						else if(repeat_max != repeat_min)
						{
							*code++ = (ssh_ws)c;
							if(prop_type >= 0)
							{
								*code++ = (ssh_ws)prop_type;
								*code++ = (ssh_ws)prop_value;
							}
							repeat_max -= repeat_min;

							if(repeat_max == 1)
							{
								*code++ = (ssh_ws)(OP_QUERY + repeat_type);
							}
							else
							{
								*code++ = (ssh_ws)(OP_UPTO + repeat_type);
								PUT2INC(code, 0, repeat_max);
							}
						}
					}

					*code++ = (ssh_ws)c;
				}
				else if(*previous == OP_CLASS || *previous == OP_NCLASS || *previous == OP_XCLASS || *previous == OP_REF || *previous == OP_REFI || *previous == OP_DNREF || *previous == OP_DNREFI)
				{
					if(repeat_max == 0)
					{
						code = previous;
						goto END_REPEAT;
					}
					if(repeat_min == 0 && repeat_max == -1) *code++ = (ssh_ws)(OP_CRSTAR + repeat_type);
					else if(repeat_min == 1 && repeat_max == -1) *code++ = (ssh_ws)(OP_CRPLUS + repeat_type);
					else if(repeat_min == 0 && repeat_max == 1) *code++ = (ssh_ws)(OP_CRQUERY + repeat_type);
					else
					{
						*code++ = (ssh_ws)(OP_CRRANGE + repeat_type);
						PUT2INC(code, 0, repeat_min);
						if(repeat_max == -1) repeat_max = 0;
						PUT2INC(code, 0, repeat_max);
					}
				}
				else if(*previous >= OP_ASSERT && *previous <= OP_COND)
				{
					register ssh_l i;
					ssh_l len = (ssh_l)(code - previous);
					size_t base_hwm_offset = save_hwm_offset;
					ssh_ws *bralink = NULL;
					ssh_ws *brazeroptr = NULL;
					if(*previous == OP_COND && previous[LINK_SIZE + 1] == OP_DEF)
						goto END_REPEAT;
					if(*previous < OP_ONCE)
					{
						if(repeat_min > 0) goto END_REPEAT;
						if(repeat_max < 0 || repeat_max > 1) repeat_max = 1;
					}
					if(repeat_min == 0)
					{
						if(repeat_max <= 1)
						{
							*code = OP_END;
							adjust_recurse(previous, 1, utf, cd, save_hwm_offset);
							memmove(previous + 1, previous, IN_UCHARS(len));
							code++;
							if(repeat_max == 0)
							{
								*previous++ = OP_SKIPZERO;
								goto END_REPEAT;
							}
							brazeroptr = previous;
							*previous++ = (ssh_ws)(OP_BRAZERO + repeat_type);
						}
						else
						{
							ssh_l offset;
							*code = OP_END;
							adjust_recurse(previous, 2 + LINK_SIZE, utf, cd, save_hwm_offset);
							memmove(previous + 2 + LINK_SIZE, previous, IN_UCHARS(len));
							code += 2 + LINK_SIZE;
							*previous++ = (ssh_ws)(OP_BRAZERO + repeat_type);
							*previous++ = OP_BRA;
							offset = (bralink == NULL) ? 0 : (ssh_l)(previous - bralink);
							bralink = previous;
							previous[0] = (ssh_ws)offset; previous += LINK_SIZE;
						}
						repeat_max--;
					}
					else
					{
						if(repeat_min > 1)
						{
							if(lengthptr != NULL)
							{
								ssh_l delta = (repeat_min - 1)*length_prevgroup;
								if((INT64_OR_DOUBLE)(repeat_min - 1) * (INT64_OR_DOUBLE)length_prevgroup > (INT64_OR_DOUBLE)INT_MAX || OFLOW_MAX - *lengthptr < delta)
								{
									*errorcodeptr = ERR20;
									goto FAILED;
								}
								*lengthptr += delta;
							}
							else
							{
								if(groupsetfirstchar && reqcharflags < 0)
								{
									reqchar = firstchar;
									reqcharflags = firstcharflags;
								}

								for(i = 1; i < repeat_min; i++)
								{
									ssh_ws *hc;
									size_t this_hwm_offset = cd->hwm - cd->start_workspace;
									memcpy(code, previous, IN_UCHARS(len));

									while(cd->hwm > cd->start_workspace + cd->workspace_size - WORK_SIZE_SAFETY_MARGIN - (this_hwm_offset - base_hwm_offset))
									{
										*errorcodeptr = expand_workspace(cd);
										if(*errorcodeptr != 0) goto FAILED;
									}

									for(hc = (ssh_ws *)cd->start_workspace + base_hwm_offset; hc < (ssh_ws *)cd->start_workspace + this_hwm_offset; hc += LINK_SIZE)
									{
										cd->hwm[0] = (ssh_ws)(GET(hc, 0) + len); cd->hwm += LINK_SIZE;
									}
									base_hwm_offset = this_hwm_offset;
									code += len;
								}
							}
						}

						if(repeat_max > 0) repeat_max -= repeat_min;
					}
					if(repeat_max >= 0)
					{
						if(lengthptr != NULL && repeat_max > 0)
						{
							ssh_l delta = repeat_max * (length_prevgroup + 1 + 2 + 2 * LINK_SIZE) - 2 - 2 * LINK_SIZE;
							if((INT64_OR_DOUBLE)repeat_max * (INT64_OR_DOUBLE)(length_prevgroup + 1 + 2 + 2 * LINK_SIZE) > (INT64_OR_DOUBLE)INT_MAX || OFLOW_MAX - *lengthptr < delta)
							{
								*errorcodeptr = ERR20;
								goto FAILED;
							}
							*lengthptr += delta;
						}
						else for(i = repeat_max - 1; i >= 0; i--)
						{
							ssh_ws *hc;
							size_t this_hwm_offset = cd->hwm - cd->start_workspace;
							*code++ = (ssh_ws)(OP_BRAZERO + repeat_type);
							if(i != 0)
							{
								ssh_l offset;
								*code++ = OP_BRA;
								offset = (bralink == NULL) ? 0 : (ssh_l)(code - bralink);
								bralink = code;
								code[0] = (ssh_ws)offset; code += LINK_SIZE;
							}
							memcpy(code, previous, IN_UCHARS(len));
							while(cd->hwm > cd->start_workspace + cd->workspace_size - WORK_SIZE_SAFETY_MARGIN - (this_hwm_offset - base_hwm_offset))
							{
								*errorcodeptr = expand_workspace(cd);
								if(*errorcodeptr != 0) goto FAILED;
							}
							for(hc = (ssh_ws *)cd->start_workspace + base_hwm_offset; hc < (ssh_ws *)cd->start_workspace + this_hwm_offset; hc += LINK_SIZE)
							{
								cd->hwm[0] = (ssh_ws)(GET(hc, 0) + len + ((i != 0) ? 2 + LINK_SIZE : 1)); cd->hwm += LINK_SIZE;
							}
							base_hwm_offset = this_hwm_offset;
							code += len;
						}
						while(bralink != NULL)
						{
							ssh_l oldlinkoffset;
							ssh_l offset = (ssh_l)(code - bralink + 1);
							ssh_ws *bra = code - offset;
							oldlinkoffset = GET(bra, 1);
							bralink = (oldlinkoffset == 0) ? NULL : bralink - oldlinkoffset;
							*code++ = OP_KET;
							code[0] = (ssh_ws)offset; code += LINK_SIZE;
							bra[1] = (ssh_ws)offset;
						}
					}
					else
					{
						ssh_ws *ketcode = code - 1 - LINK_SIZE;
						ssh_ws *bracode = ketcode - GET(ketcode, 1);
						if((*bracode == OP_ONCE || *bracode == OP_ONCE_NC) && possessive_quantifier) *bracode = OP_BRA;
						if(*bracode == OP_ONCE || *bracode == OP_ONCE_NC) *ketcode = (ssh_ws)(OP_KETRMAX + repeat_type);
						else
						{
							if(lengthptr == NULL)
							{
								ssh_ws *scode = bracode;
								do
								{
									if(could_be_empty_branch(scode, ketcode, utf, cd, NULL))
									{
										*bracode += OP_SBRA - OP_BRA;
										break;
									}
									scode += GET(scode, 1);
								} while(*scode == OP_ALT);
							}
							if(possessive_quantifier)
							{
								if(*bracode == OP_COND || *bracode == OP_SCOND)
								{
									ssh_l nlen = (ssh_l)(code - bracode);
									*code = OP_END;
									adjust_recurse(bracode, 1 + LINK_SIZE, utf, cd, save_hwm_offset);
									memmove(bracode + 1 + LINK_SIZE, bracode, IN_UCHARS(nlen));
									code += 1 + LINK_SIZE;
									nlen += 1 + LINK_SIZE;
									*bracode = OP_BRAPOS;
									*code++ = OP_KETRPOS;
									code[0] = (ssh_ws)nlen; code += LINK_SIZE;
									bracode[1] = (ssh_ws)nlen;
								}
								else
								{
									*bracode += 1;
									*ketcode = OP_KETRPOS;
								}
								if(brazeroptr != NULL) *brazeroptr = OP_BRAPOSZERO;
								if(repeat_min < 2) possessive_quantifier = FALSE;
							}
							else *ketcode = (ssh_ws)(OP_KETRMAX + repeat_type);
						}
					}
				}
				else if(*previous == OP_FAIL) goto END_REPEAT;
				else
				{
					*errorcodeptr = ERR11;
					goto FAILED;
				}
				if(possessive_quantifier)
				{
					ssh_l len;
					switch(*tempcode)
					{
						case OP_TYPEEXACT:
							tempcode += PRIV(OP_lengths)[*tempcode] + ((tempcode[1 + IMM2_SIZE] == OP_PROP || tempcode[1 + IMM2_SIZE] == OP_NOTPROP) ? 2 : 0);
							break;
						case OP_CHAR:
						case OP_CHARI:
						case OP_NOT:
						case OP_NOTI:
						case OP_EXACT:
						case OP_EXACTI:
						case OP_NOTEXACT:
						case OP_NOTEXACTI:
							tempcode += PRIV(OP_lengths)[*tempcode];
							break;
						case OP_CLASS:
						case OP_NCLASS:
							tempcode += 1 + 32 / sizeof(ssh_ws);
							break;
						case OP_XCLASS:
							tempcode += GET(tempcode, 1);
							break;
					}
					len = (ssh_l)(code - tempcode);
					if(len > 0)
					{
						ssh_u repcode = *tempcode;
						if(repcode < OP_CALLOUT && opcode_possessify[repcode] > 0) *tempcode = opcode_possessify[repcode];
						else
						{
							*code = OP_END;
							adjust_recurse(tempcode, 1 + LINK_SIZE, utf, cd, save_hwm_offset);
							memmove(tempcode + 1 + LINK_SIZE, tempcode, IN_UCHARS(len));
							code += 1 + LINK_SIZE;
							len += 1 + LINK_SIZE;
							tempcode[0] = OP_ONCE;
							*code++ = OP_KET;
							code[0] = (ssh_ws)len; code += LINK_SIZE;
							tempcode[1] = (ssh_ws)len;
						}
					}
				}
END_REPEAT:
				previous = NULL;
				cd->req_varyopt |= reqvary;
				break;
			case CHAR_LEFT_PARENTHESIS:
				ptr++;
				if(ptr[0] == CHAR_QUESTION_MARK && ptr[1] == CHAR_NUMBER_SIGN)
				{
					ptr += 2;
					while(*ptr != CHAR_NULL && *ptr != CHAR_RIGHT_PARENTHESIS) ptr++;
					if(*ptr == CHAR_NULL)
					{
						*errorcodeptr = ERR18;
						goto FAILED;
					}
					continue;
				}
				if(ptr[0] == CHAR_ASTERISK && (ptr[1] == ':' || (MAX_255(ptr[1]) && ((cd->ctypes[ptr[1]] & ctype_letter) != 0))))
				{
					ssh_l i, namelen;
					ssh_l arglen = 0;
					const char *vn = verbnames;
					const ssh_ws *name = ptr + 1;
					const ssh_ws *arg = NULL;
					previous = NULL;
					ptr++;
					while(MAX_255(*ptr) && (cd->ctypes[*ptr] & ctype_letter) != 0) ptr++;
					namelen = (ssh_l)(ptr - name);
					if(*ptr == CHAR_COLON)
					{
						arg = ++ptr;
						while(*ptr != CHAR_NULL && *ptr != CHAR_RIGHT_PARENTHESIS) ptr++;
						arglen = (ssh_l)(ptr - arg);
						if((ssh_u)arglen > MAX_MARK)
						{
							*errorcodeptr = ERR75;
							goto FAILED;
						}
					}
					if(*ptr != CHAR_RIGHT_PARENTHESIS)
					{
						*errorcodeptr = ERR60;
						goto FAILED;
					}
					for(i = 0; i < verbcount; i++)
					{
						if(namelen == verbs[i].len && STRNCMP_UC_C8(name, vn, namelen) == 0)
						{
							ssh_l setverb;
							if(verbs[i].op == OP_ACCEPT)
							{
								open_capitem *oc;
								if(arglen != 0)
								{
									*errorcodeptr = ERR59;
									goto FAILED;
								}
								cd->had_accept = TRUE;
								for(oc = cd->open_caps; oc != NULL; oc = oc->next)
								{
									*code++ = OP_CLOSE;
									PUT2INC(code, 0, oc->number);
								}
								setverb = *code++ = (cd->assert_depth > 0) ? OP_ASSERT_ACCEPT : OP_ACCEPT;
								if(firstcharflags == REQ_UNSET) firstcharflags = REQ_NONE;
							}
							else if(arglen == 0)
							{
								if(verbs[i].op < 0)
								{
									*errorcodeptr = ERR66;
									goto FAILED;
								}
								setverb = verbs[i].op;
								*code++ = (ssh_ws)setverb;
							}

							else
							{
								if(verbs[i].op_arg < 0)
								{
									*errorcodeptr = ERR59;
									goto FAILED;
								}
								setverb = verbs[i].op_arg;
								*code++ = (ssh_ws)setverb;
								*code++ = (ssh_ws)arglen;
								memcpy(code, arg, IN_UCHARS(arglen));
								code += arglen;
								*code++ = 0;
							}

							switch(setverb)
							{
								case OP_THEN:
								case OP_THEN_ARG:
									cd->external_flags |= REGEX_HASTHEN;
									break;

								case OP_PRUNE:
								case OP_PRUNE_ARG:
								case OP_SKIP:
								case OP_SKIP_ARG:
									cd->had_pruneorskip = TRUE;
									break;
							}

							break;
						}

						vn += verbs[i].len + 1;
					}

					if(i < verbcount) continue;
					*errorcodeptr = ERR60;
					goto FAILED;
				}
				newoptions = options;
				skipbytes = 0;
				bravalue = OP_CBRA;
				save_hwm_offset = cd->hwm - cd->start_workspace;
				reset_bracount = FALSE;
				if(*ptr == CHAR_QUESTION_MARK)
				{
					ssh_l i, set, unset, namelen;
					ssh_l *optset;
					const ssh_ws *name;
					ssh_ws *slot;
					switch(*(++ptr))
					{
						case CHAR_VERTICAL_LINE:
							reset_bracount = TRUE;
						case CHAR_COLON:
							bravalue = OP_BRA;
							ptr++;
							break;
						case CHAR_LEFT_PARENTHESIS:
							bravalue = OP_COND;
							tempptr = ptr;
							if(ptr[1] == CHAR_QUESTION_MARK && ptr[2] == CHAR_C)
							{
								for(i = 3;; i++) if(!IS_DIGIT(ptr[i])) break;
								if(ptr[i] == CHAR_RIGHT_PARENTHESIS) tempptr += i + 1;
							}
							if(tempptr[1] == CHAR_QUESTION_MARK && (tempptr[2] == CHAR_EQUALS_SIGN || tempptr[2] == CHAR_EXCLAMATION_MARK || (tempptr[2] == CHAR_LESS_THAN_SIGN && (tempptr[3] == CHAR_EQUALS_SIGN || tempptr[3] == CHAR_EXCLAMATION_MARK))))
							{
								cd->iscondassert = TRUE;
								break;
							}
							code[1 + LINK_SIZE] = OP_CREF;
							skipbytes = 1 + IMM2_SIZE;
							refsign = -1;
							namelen = -1;
							name = NULL;
							recno = 0;
							ptr++;
							if(*ptr == CHAR_R && ptr[1] == CHAR_AMPERSAND)
							{
								terminator = -1;
								ptr += 2;
								code[1 + LINK_SIZE] = OP_RREF;
							}
							else if(*ptr == CHAR_LESS_THAN_SIGN)
							{
								terminator = CHAR_GREATER_THAN_SIGN;
								ptr++;
							}
							else if(*ptr == CHAR_APOSTROPHE)
							{
								terminator = CHAR_APOSTROPHE;
								ptr++;
							}
							else
							{
								terminator = CHAR_NULL;
								if(*ptr == CHAR_MINUS || *ptr == CHAR_PLUS) refsign = *ptr++;
								else if(IS_DIGIT(*ptr)) refsign = 0;
							}
							if(refsign >= 0)
							{
								while(IS_DIGIT(*ptr))
								{
									recno = recno * 10 + (ssh_l)(*ptr - CHAR_0);
									ptr++;
								}
							}
							else
							{
								if(IS_DIGIT(*ptr))
								{
									*errorcodeptr = ERR84;
									goto FAILED;
								}
								if(!MAX_255(*ptr) || (cd->ctypes[*ptr] & ctype_word) == 0)
								{
									*errorcodeptr = ERR28;
									goto FAILED;
								}
								name = ptr++;
								while(MAX_255(*ptr) && (cd->ctypes[*ptr] & ctype_word) != 0)
								{
									ptr++;
								}
								namelen = (ssh_l)(ptr - name);
								if(lengthptr != NULL) *lengthptr += IMM2_SIZE;
							}
							if((terminator > 0 && *ptr++ != (ssh_ws)terminator) || *ptr++ != CHAR_RIGHT_PARENTHESIS)
							{
								ptr--;
								*errorcodeptr = ERR26;
								goto FAILED;
							}
							if(lengthptr != NULL) break;
							if(refsign >= 0)
							{
								if(recno <= 0)
								{
									*errorcodeptr = ERR35;
									goto FAILED;
								}
								if(refsign != 0) recno = (refsign == CHAR_MINUS) ? cd->bracount - recno + 1 : recno + cd->bracount;
								if(recno <= 0 || recno > cd->final_bracount)
								{
									*errorcodeptr = ERR15;
									goto FAILED;
								}
								PUT2(code, 2 + LINK_SIZE, recno);
								if(recno > cd->top_backref) cd->top_backref = recno;
								break;
							}
							slot = cd->name_table;
							for(i = 0; i < cd->names_found; i++)
							{
								if(STRNCMP_UC_UC(name, slot + IMM2_SIZE, namelen) == 0) break;
								slot += cd->name_entry_size;
							}
							if(i < cd->names_found)
							{
								ssh_l offset = i++;
								ssh_l count = 1;
								recno = GET2(slot, 0);
								if(recno > cd->top_backref) cd->top_backref = recno;
								for(; i < cd->names_found; i++)
								{
									slot += cd->name_entry_size;
									if(STRNCMP_UC_UC(name, slot + IMM2_SIZE, namelen) != 0 || (slot + IMM2_SIZE)[namelen] != 0) break;
									count++;
								}
								if(count > 1)
								{
									PUT2(code, 2 + LINK_SIZE, offset);
									PUT2(code, 2 + LINK_SIZE + IMM2_SIZE, count);
									skipbytes += IMM2_SIZE;
									code[1 + LINK_SIZE]++;
								}
								else
								{
									PUT2(code, 2 + LINK_SIZE, recno);
								}
							}
							else if(terminator != CHAR_NULL)
							{
								*errorcodeptr = ERR15;
								goto FAILED;
							}
							else if(*name == CHAR_R)
							{
								recno = 0;
								for(i = 1; i < namelen; i++)
								{
									if(!IS_DIGIT(name[i]))
									{
										*errorcodeptr = ERR15;
										goto FAILED;
									}
									recno = recno * 10 + name[i] - CHAR_0;
								}
								if(recno == 0) recno = RREF_ANY;
								code[1 + LINK_SIZE] = OP_RREF;
								PUT2(code, 2 + LINK_SIZE, recno);
							}
							else if(namelen == 6 && STRNCMP_UC_C8(name, STRING_DEFINE, 6) == 0)
							{
								code[1 + LINK_SIZE] = OP_DEF;
								skipbytes = 1;
							}
							else
							{
								*errorcodeptr = ERR15;
								goto FAILED;
							}
							break;
						case CHAR_EQUALS_SIGN:
							bravalue = OP_ASSERT;
							cd->assert_depth += 1;
							ptr++;
							break;
						case CHAR_EXCLAMATION_MARK:
							ptr++;
							if(*ptr == CHAR_RIGHT_PARENTHESIS && ptr[1] != CHAR_ASTERISK && ptr[1] != CHAR_PLUS && ptr[1] != CHAR_QUESTION_MARK && (ptr[1] != CHAR_LEFT_CURLY_BRACKET || !is_counted_repeat(ptr + 2)))
							{
								*code++ = OP_FAIL;
								previous = NULL;
								continue;
							}
							bravalue = OP_ASSERT_NOT;
							cd->assert_depth += 1;
							break;
						case CHAR_LESS_THAN_SIGN:
							switch(ptr[1])
							{
								case CHAR_EQUALS_SIGN:
									bravalue = OP_ASSERTBACK;
									cd->assert_depth += 1;
									ptr += 2;
									break;
								case CHAR_EXCLAMATION_MARK:
									bravalue = OP_ASSERTBACK_NOT;
									cd->assert_depth += 1;
									ptr += 2;
									break;
								default:
									if(MAX_255(ptr[1]) && (cd->ctypes[ptr[1]] & ctype_word) != 0) goto DEFINE_NAME;
									ptr++;
									*errorcodeptr = ERR24;
									goto FAILED;
							}
							break;
						case CHAR_GREATER_THAN_SIGN:
							bravalue = OP_ONCE;
							ptr++;
							break;
						case CHAR_C:
							previous_callout = code;
							after_manual_callout = 1;
							*code++ = OP_CALLOUT;
							{
								ssh_l n = 0;
								ptr++;
								while(IS_DIGIT(*ptr)) n = n * 10 + *ptr++ - CHAR_0;
								if(*ptr != CHAR_RIGHT_PARENTHESIS)
								{
									*errorcodeptr = ERR39;
									goto FAILED;
								}
								if(n > 255)
								{
									*errorcodeptr = ERR38;
									goto FAILED;
								}
								*code++ = (ssh_ws)n;
								code[0] = (ssh_ws)(ssh_l)(ptr - cd->start_pattern + 1);
								code[LINK_SIZE] = 0;
								code += 2 * LINK_SIZE;
							}
							previous = NULL;
							continue;
						case CHAR_P:
							if(*(++ptr) == CHAR_EQUALS_SIGN || *ptr == CHAR_GREATER_THAN_SIGN)
							{
								is_recurse = *ptr == CHAR_GREATER_THAN_SIGN;
								terminator = CHAR_RIGHT_PARENTHESIS;
								goto NAMED_REF_OR_RECURSE;
							}
							else if(*ptr != CHAR_LESS_THAN_SIGN)
							{
								*errorcodeptr = ERR41;
								goto FAILED;
							}
DEFINE_NAME:
						case CHAR_APOSTROPHE:
							terminator = (*ptr == CHAR_LESS_THAN_SIGN) ? CHAR_GREATER_THAN_SIGN : CHAR_APOSTROPHE;
							name = ++ptr;
							if(IS_DIGIT(*ptr))
							{
								*errorcodeptr = ERR84;
								goto FAILED;
							}
							while(MAX_255(*ptr) && (cd->ctypes[*ptr] & ctype_word) != 0) ptr++;
							namelen = (ssh_l)(ptr - name);
							if(lengthptr != NULL)
							{
								named_group *ng;
								ssh_u number = cd->bracount + 1;

								if(*ptr != (ssh_ws)terminator)
								{
									*errorcodeptr = ERR42;
									goto FAILED;
								}

								if(cd->names_found >= MAX_NAME_COUNT)
								{
									*errorcodeptr = ERR49;
									goto FAILED;
								}

								if(namelen + IMM2_SIZE + 1 > cd->name_entry_size)
								{
									cd->name_entry_size = namelen + IMM2_SIZE + 1;
									if(namelen > MAX_NAME_SIZE)
									{
										*errorcodeptr = ERR48;
										goto FAILED;
									}
								}
								ng = cd->named_groups;
								for(i = 0; i < cd->names_found; i++, ng++)
								{
									if(namelen == ng->length && STRNCMP_UC_UC(name, ng->name, namelen) == 0)
									{
										if(ng->number == number) break;
										if((options & REGEX_DUPNAMES) == 0)
										{
											*errorcodeptr = ERR43;
											goto FAILED;
										}
										cd->dupnames = TRUE;
									}
									else if(ng->number == number)
									{
										*errorcodeptr = ERR65;
										goto FAILED;
									}
								}
								if(i >= cd->names_found)
								{
									if(cd->names_found >= cd->named_group_list_size)
									{
										ssh_l newsize = cd->named_group_list_size * 2;
										named_group *newspace = (named_group*)malloc(newsize * sizeof(named_group));
										if(newspace == NULL)
										{
											*errorcodeptr = ERR21;
											goto FAILED;
										}
										memcpy(newspace, cd->named_groups, cd->named_group_list_size * sizeof(named_group));
										if(cd->named_group_list_size > NAMED_GROUP_LIST_SIZE) free((void *)cd->named_groups);
										cd->named_groups = newspace;
										cd->named_group_list_size = newsize;
									}
									cd->named_groups[cd->names_found].name = name;
									cd->named_groups[cd->names_found].length = namelen;
									cd->named_groups[cd->names_found].number = number;
									cd->names_found++;
								}
							}
							ptr++;
							goto NUMBERED_GROUP;
						case CHAR_AMPERSAND:
							terminator = CHAR_RIGHT_PARENTHESIS;
							is_recurse = TRUE;
NAMED_REF_OR_RECURSE:
							name = ++ptr;
							if(IS_DIGIT(*ptr))
							{
								*errorcodeptr = ERR84;
								goto FAILED;
							}
							while(MAX_255(*ptr) && (cd->ctypes[*ptr] & ctype_word) != 0) ptr++;
							namelen = (ssh_l)(ptr - name);
							if(lengthptr != NULL)
							{
								named_group *ng;
								if(namelen == 0)
								{
									*errorcodeptr = ERR62;
									goto FAILED;
								}
								if(*ptr != (ssh_ws)terminator)
								{
									*errorcodeptr = ERR42;
									goto FAILED;
								}
								if(namelen > MAX_NAME_SIZE)
								{
									*errorcodeptr = ERR48;
									goto FAILED;
								}
								ng = cd->named_groups;
								for(i = 0; i < cd->names_found; i++, ng++)
								{
									if(namelen == ng->length && STRNCMP_UC_UC(name, ng->name, namelen) == 0)
									   break;
								}
								recno = (i < cd->names_found) ? ng->number : 0;
								if(!is_recurse) cd->namedrefcount++;
								*lengthptr += IMM2_SIZE;
							}
							else
							{
								slot = cd->name_table;
								for(i = 0; i < cd->names_found; i++)
								{
									if(STRNCMP_UC_UC(name, slot + IMM2_SIZE, namelen) == 0 &&
									   slot[IMM2_SIZE + namelen] == 0)
									   break;
									slot += cd->name_entry_size;
								}
								if(i < cd->names_found)
								{
									recno = GET2(slot, 0);
								}
								else
								{
									*errorcodeptr = ERR15;
									goto FAILED;
								}
							}
							if(is_recurse) goto HANDLE_RECURSION;
							if(lengthptr == NULL && cd->dupnames)
							{
								ssh_l count = 1;
								ssh_u index = i;
								ssh_ws *cslot = slot + cd->name_entry_size;

								for(i++; i < cd->names_found; i++)
								{
									if(STRCMP_UC_UC(slot + IMM2_SIZE, cslot + IMM2_SIZE) != 0) break;
									count++;
									cslot += cd->name_entry_size;
								}
								if(count > 1)
								{
									if(firstcharflags == REQ_UNSET) firstcharflags = REQ_NONE;
									previous = code;
									*code++ = ((options & REGEX_CASELESS) != 0) ? OP_DNREFI : OP_DNREF;
									PUT2INC(code, 0, index);
									PUT2INC(code, 0, count);
									for(; slot < cslot; slot += cd->name_entry_size)
									{
										open_capitem *oc;
										recno = GET2(slot, 0);
										cd->backref_map |= (recno < 32) ? (1 << recno) : 1;
										if(recno > cd->top_backref) cd->top_backref = recno;
										for(oc = cd->open_caps; oc != NULL; oc = oc->next)
										{
											if(oc->number == recno)
											{
												oc->flag = TRUE;
												break;
											}
										}
									}
									continue;
								}
							}
							goto HANDLE_REFERENCE;
						case CHAR_R:
							ptr++;
						case CHAR_MINUS: case CHAR_PLUS:
						case CHAR_0: case CHAR_1: case CHAR_2: case CHAR_3: case CHAR_4:
						case CHAR_5: case CHAR_6: case CHAR_7: case CHAR_8: case CHAR_9:
						{
							const ssh_ws *called;
							terminator = CHAR_RIGHT_PARENTHESIS;
HANDLE_NUMERICAL_RECURSION:
							if((refsign = *ptr) == CHAR_PLUS)
							{
								ptr++;
								if(!IS_DIGIT(*ptr))
								{
									*errorcodeptr = ERR63;
									goto FAILED;
								}
							}
							else if(refsign == CHAR_MINUS)
							{
								if(!IS_DIGIT(ptr[1]))
									goto OTHER_CHAR_AFTER_QUERY;
								ptr++;
							}

							recno = 0;
							while(IS_DIGIT(*ptr)) recno = recno * 10 + *ptr++ - CHAR_0;
							if(*ptr != (ssh_ws)terminator)
							{
								*errorcodeptr = ERR29;
								goto FAILED;
							}

							if(refsign == CHAR_MINUS)
							{
								if(recno == 0)
								{
									*errorcodeptr = ERR58;
									goto FAILED;
								}
								recno = cd->bracount - recno + 1;
								if(recno <= 0)
								{
									*errorcodeptr = ERR15;
									goto FAILED;
								}
							}
							else if(refsign == CHAR_PLUS)
							{
								if(recno == 0)
								{
									*errorcodeptr = ERR58;
									goto FAILED;
								}
								recno += cd->bracount;
							}
HANDLE_RECURSION:
							previous = code;
							called = cd->start_code;
							if(lengthptr == NULL)
							{
								*code = OP_END;
								if(recno != 0) called = PRIV(find_bracket)(cd->start_code, utf, recno);
								if(called == NULL)
								{
									if(recno > cd->final_bracount)
									{
										*errorcodeptr = ERR15;
										goto FAILED;
									}
									called = cd->start_code + recno;
									if(cd->hwm >= cd->start_workspace + cd->workspace_size - WORK_SIZE_SAFETY_MARGIN)
									{
										*errorcodeptr = expand_workspace(cd);
										if(*errorcodeptr != 0) goto FAILED;
									}
									cd->hwm[0] = (ssh_ws)(ssh_l)(code + 1 - cd->start_code); cd->hwm += LINK_SIZE;
								}
								else if(GET(called, 1) == 0 && cond_depth <= 0 && could_be_empty(called, code, bcptr, utf, cd))
								{
									*errorcodeptr = ERR40;
									goto FAILED;
								}
							}
							*code = OP_RECURSE;
							code[1] = (ssh_ws)(ssh_l)(called - cd->start_code);
							code += 1 + LINK_SIZE;
							groupsetfirstchar = FALSE;
						}
						if(firstcharflags == REQ_UNSET) firstcharflags = REQ_NONE;
						continue;
						default:
OTHER_CHAR_AFTER_QUERY :
						set = unset = 0;
					   optset = &set;
					   while(*ptr != CHAR_RIGHT_PARENTHESIS && *ptr != CHAR_COLON)
					   {
						   switch(*ptr++)
						   {
							   case CHAR_MINUS: optset = &unset; break;

							   case CHAR_J:
								   *optset |= REGEX_DUPNAMES;
								   cd->external_flags |= REGEX_JCHANGED;
								   break;

							   case CHAR_i: *optset |= REGEX_CASELESS; break;
							   case CHAR_m: *optset |= REGEX_MULTILINE; break;
							   case CHAR_s: *optset |= REGEX_DOTALL; break;
							   case CHAR_x: *optset |= REGEX_EXTENDED; break;
							   case CHAR_U: *optset |= REGEX_UNGREEDY; break;
							   case CHAR_X: *optset |= REGEX_EXTRA; break;

							   default:  *errorcodeptr = ERR12;
								   ptr--;
								   goto FAILED;
						   }
					   }
					   newoptions = (options | set) & (~unset);
					   if(*ptr == CHAR_RIGHT_PARENTHESIS)
					   {
						   if(code == cd->start_code + 1 + LINK_SIZE && (lengthptr == NULL || *lengthptr == 2 + 2 * LINK_SIZE)) cd->external_options = newoptions;
						   else
						   {
							   greedy_default = ((newoptions & REGEX_UNGREEDY) != 0);
							   greedy_non_default = greedy_default ^ 1;
							   req_caseopt = ((newoptions & REGEX_CASELESS) != 0) ? REQ_CASELESS : 0;
						   }
						   *optionsptr = options = newoptions;
						   previous = NULL;
						   continue;
					   }
					   bravalue = OP_BRA;
					   ptr++;
					}
				}
				else if((options & REGEX_NO_AUTO_CAPTURE) != 0) bravalue = OP_BRA;
				else
				{
NUMBERED_GROUP:
					cd->bracount += 1;
					PUT2(code, 1 + LINK_SIZE, cd->bracount);
					skipbytes = IMM2_SIZE;
				}

				if((cd->parens_depth += 1) > PARENS_NEST_LIMIT)
				{
					*errorcodeptr = ERR82;
					goto FAILED;
				}
				if(bravalue >= OP_ASSERT && bravalue <= OP_ASSERTBACK_NOT && cd->iscondassert)
				{
					previous = NULL;
					cd->iscondassert = FALSE;
				}
				else previous = code;
				*code = (ssh_ws)bravalue;
				tempcode = code;
				tempreqvary = cd->req_varyopt;
				tempbracount = cd->bracount;
				length_prevgroup = 0;
				if(!compile_regex(newoptions, &tempcode, &ptr, errorcodeptr, (bravalue == OP_ASSERTBACK || bravalue == OP_ASSERTBACK_NOT), reset_bracount, skipbytes, cond_depth + ((bravalue == OP_COND) ? 1 : 0), &subfirstchar, &subfirstcharflags, &subreqchar, &subreqcharflags, bcptr, cd, (lengthptr == NULL) ? NULL : &length_prevgroup ))
					goto FAILED;
				cd->parens_depth -= 1;
				if(bravalue == OP_ONCE && cd->bracount <= tempbracount) *code = OP_ONCE_NC;
				if(bravalue >= OP_ASSERT && bravalue <= OP_ASSERTBACK_NOT) cd->assert_depth -= 1;
				if(bravalue == OP_COND && lengthptr == NULL)
				{
					ssh_ws *tc = code;
					ssh_l condcount = 0;
					do
					{
						condcount++;
						tc += GET(tc, 1);
					} while(*tc != OP_KET);
					if(code[LINK_SIZE + 1] == OP_DEF)
					{
						if(condcount > 1)
						{
							*errorcodeptr = ERR54;
							goto FAILED;
						}
						bravalue = OP_DEF;
					}
					else
					{
						if(condcount > 2)
						{
							*errorcodeptr = ERR27;
							goto FAILED;
						}
						if(condcount == 1) subfirstcharflags = subreqcharflags = REQ_NONE;
					}
				}
				if(*ptr != CHAR_RIGHT_PARENTHESIS)
				{
					*errorcodeptr = ERR14;
					goto FAILED;
				}
				if(lengthptr != NULL)
				{
					if(OFLOW_MAX - *lengthptr < length_prevgroup - 2 - 2 * LINK_SIZE)
					{
						*errorcodeptr = ERR20;
						goto FAILED;
					}
					*lengthptr += length_prevgroup - 2 - 2 * LINK_SIZE;
					code++;
					code[0] = 1 + LINK_SIZE; code += LINK_SIZE;
					*code++ = OP_KET;
					code[0] = 1 + LINK_SIZE; code += LINK_SIZE;
					break;
				}
				code = tempcode;
				if(bravalue == OP_DEF) break;
				zeroreqchar = reqchar;
				zeroreqcharflags = reqcharflags;
				zerofirstchar = firstchar;
				zerofirstcharflags = firstcharflags;
				groupsetfirstchar = FALSE;
				if(bravalue >= OP_ONCE)
				{
					if(firstcharflags == REQ_UNSET)
					{
						if(subfirstcharflags >= 0)
						{
							firstchar = subfirstchar;
							firstcharflags = subfirstcharflags;
							groupsetfirstchar = TRUE;
						}
						else firstcharflags = REQ_NONE;
						zerofirstcharflags = REQ_NONE;
					}
					else if(subfirstcharflags >= 0 && subreqcharflags < 0)
					{
						subreqchar = subfirstchar;
						subreqcharflags = subfirstcharflags | tempreqvary;
					}
					if(subreqcharflags >= 0)
					{
						reqchar = subreqchar;
						reqcharflags = subreqcharflags;
					}
				}
				else if(bravalue == OP_ASSERT && subreqcharflags >= 0)
				{
					reqchar = subreqchar;
					reqcharflags = subreqcharflags;
				}
				break;
			case CHAR_BACKSLASH:
				tempptr = ptr;
				escape = check_escape(&ptr, &ec, errorcodeptr, cd->bracount, options, FALSE);
				if(*errorcodeptr != 0) goto FAILED;
				if(escape == 0) c = ec;
				else
				{
					if(escape == ESC_Q)
					{
						if(ptr[1] == CHAR_BACKSLASH && ptr[2] == CHAR_E) ptr += 2;
						else inescq = TRUE;
						continue;
					}
					if(escape == ESC_E) continue;
					if(firstcharflags == REQ_UNSET && escape > ESC_b && escape < ESC_Z) firstcharflags = REQ_NONE;
					zerofirstchar = firstchar;
					zerofirstcharflags = firstcharflags;
					zeroreqchar = reqchar;
					zeroreqcharflags = reqcharflags;
					if(escape == ESC_g)
					{
						const ssh_ws *p;
						ssh_u cf;
						save_hwm_offset = cd->hwm - cd->start_workspace;
						terminator = (*(++ptr) == CHAR_LESS_THAN_SIGN) ?
CHAR_GREATER_THAN_SIGN : CHAR_APOSTROPHE;
						skipbytes = 0;
						reset_bracount = FALSE;
						cf = ptr[1];
						if(cf != CHAR_PLUS && cf != CHAR_MINUS && !IS_DIGIT(cf))
						{
							is_recurse = TRUE;
							goto NAMED_REF_OR_RECURSE;
						}
						p = ptr + 2;
						while(IS_DIGIT(*p)) p++;
						if(*p != (ssh_ws)terminator)
						{
							*errorcodeptr = ERR57;
							break;
						}
						ptr++;
						goto HANDLE_NUMERICAL_RECURSION;
					}

					if(escape == ESC_k)
					{
						if((ptr[1] != CHAR_LESS_THAN_SIGN && ptr[1] != CHAR_APOSTROPHE && ptr[1] != CHAR_LEFT_CURLY_BRACKET))
						{
							*errorcodeptr = ERR69;
							break;
						}
						is_recurse = FALSE;
						terminator = (*(++ptr) == CHAR_LESS_THAN_SIGN) ?
CHAR_GREATER_THAN_SIGN : (*ptr == CHAR_APOSTROPHE) ?
CHAR_APOSTROPHE : CHAR_RIGHT_CURLY_BRACKET;
						goto NAMED_REF_OR_RECURSE;
					}
					if(escape < 0)
					{
						open_capitem *oc;
						recno = -escape;
HANDLE_REFERENCE:
						if(firstcharflags == REQ_UNSET) firstcharflags = REQ_NONE;
						previous = code;
						*code++ = ((options & REGEX_CASELESS) != 0) ? OP_REFI : OP_REF;
						PUT2INC(code, 0, recno);
						cd->backref_map |= (recno < 32) ? (1 << recno) : 1;
						if(recno > cd->top_backref) cd->top_backref = recno;
						for(oc = cd->open_caps; oc != NULL; oc = oc->next)
						{
							if(oc->number == recno)
							{
								oc->flag = TRUE;
								break;
							}
						}
					}
					else if(escape == ESC_X || escape == ESC_P || escape == ESC_p)
					{
						*errorcodeptr = ERR45;
						goto FAILED;
					}
					else
					{
						if((escape == ESC_b || escape == ESC_B || escape == ESC_A) && cd->max_lookbehind == 0) cd->max_lookbehind = 1;
						previous = (escape > ESC_b && escape < ESC_Z) ? code : NULL;
						*code++ = (ssh_ws)((!utf && escape == ESC_C) ? OP_ALLANY : escape);
					}
					continue;
				}
				mcbuffer[0] = (ssh_ws)c;
				mclength = 1;
				goto ONE_CHAR;
			default:
NORMAL_CHAR :
			mclength = 1;
			mcbuffer[0] = (ssh_ws)c;
ONE_CHAR:
			previous = code;
			*code++ = ((options & REGEX_CASELESS) != 0) ? OP_CHARI : OP_CHAR;
			for(c = 0; c < mclength; c++) *code++ = mcbuffer[c];
			if(mcbuffer[0] == CHAR_CR || mcbuffer[0] == CHAR_NL) cd->external_flags |= REGEX_HASCRORLF;
			if(firstcharflags == REQ_UNSET)
			{
				zerofirstcharflags = REQ_NONE;
				zeroreqchar = reqchar;
				zeroreqcharflags = reqcharflags;
				if(mclength == 1 || req_caseopt == 0)
				{
					firstchar = mcbuffer[0] | req_caseopt;
					firstchar = mcbuffer[0];
					firstcharflags = req_caseopt;

					if(mclength != 1)
					{
						reqchar = code[-1];
						reqcharflags = cd->req_varyopt;
					}
				}
				else firstcharflags = reqcharflags = REQ_NONE;
			}
			else
			{
				zerofirstchar = firstchar;
				zerofirstcharflags = firstcharflags;
				zeroreqchar = reqchar;
				zeroreqcharflags = reqcharflags;
				if(mclength == 1 || req_caseopt == 0)
				{
					reqchar = code[-1];
					reqcharflags = req_caseopt | cd->req_varyopt;
				}
			}

			break;
		}
	}
FAILED:
	*ptrptr = ptr;
	return FALSE;
}

static BOOL compile_regex(ssh_l options, ssh_ws **codeptr, const ssh_ws **ptrptr, ssh_l *errorcodeptr, BOOL lookbehind, BOOL reset_bracount, ssh_l skipbytes, ssh_l cond_depth, ssh_u *firstcharptr, ssh_l *firstcharflagsptr, ssh_u *reqcharptr, ssh_l *reqcharflagsptr, branch_chain *bcptr, compile_data *cd, ssh_l *lengthptr)
{
	const ssh_ws *ptr = *ptrptr;
	ssh_ws *code = *codeptr;
	ssh_ws *last_branch = code;
	ssh_ws *start_bracket = code;
	ssh_ws *reverse_count = NULL;
	open_capitem capitem;
	ssh_l capnumber = 0;
	ssh_u firstchar, reqchar;
	ssh_l firstcharflags, reqcharflags;
	ssh_u branchfirstchar, branchreqchar;
	ssh_l branchfirstcharflags, branchreqcharflags;
	ssh_l length;
	ssh_u orig_bracount;
	ssh_u max_bracount;
	branch_chain bc;
	size_t save_hwm_offset;
	bc.outer = bcptr;
	bc.current_branch = code;
	firstchar = reqchar = 0;
	firstcharflags = reqcharflags = REQ_UNSET;
	save_hwm_offset = cd->hwm - cd->start_workspace;
	length = 2 + 2 * LINK_SIZE + skipbytes;
	if(*code == OP_CBRA)
	{
		capnumber = GET2(code, 1 + LINK_SIZE);
		capitem.number = (ssh_ws)capnumber;
		capitem.next = cd->open_caps;
		capitem.flag = FALSE;
		cd->open_caps = &capitem;
	}
	code[1] = 0;
	code += 1 + LINK_SIZE + skipbytes;
	orig_bracount = max_bracount = cd->bracount;
	for(;;)
	{
		if(reset_bracount) cd->bracount = orig_bracount;
		if(lookbehind)
		{
			*code++ = OP_REVERSE;
			reverse_count = code;
			code[0] = 0; code += LINK_SIZE;
			length += 1 + LINK_SIZE;
		}
		if(!compile_branch(&options, &code, &ptr, errorcodeptr, &branchfirstchar, &branchfirstcharflags, &branchreqchar, &branchreqcharflags, &bc, cond_depth, cd, (lengthptr == NULL) ? NULL : &length))
		{
			*ptrptr = ptr;
			return FALSE;
		}
		if(cd->bracount > max_bracount) max_bracount = cd->bracount;
		if(lengthptr == NULL)
		{
			if(*last_branch != OP_ALT)
			{
				firstchar = branchfirstchar;
				firstcharflags = branchfirstcharflags;
				reqchar = branchreqchar;
				reqcharflags = branchreqcharflags;
			}
			else
			{
				if(firstcharflags >= 0 && (firstcharflags != branchfirstcharflags || firstchar != branchfirstchar))
				{
					if(reqcharflags < 0)
					{
						reqchar = firstchar;
						reqcharflags = firstcharflags;
					}
					firstcharflags = REQ_NONE;
				}
				if(firstcharflags < 0 && branchfirstcharflags >= 0 && branchreqcharflags < 0)
				{
					branchreqchar = branchfirstchar;
					branchreqcharflags = branchfirstcharflags;
				}
				if(((reqcharflags & ~REQ_VARY) != (branchreqcharflags & ~REQ_VARY)) || reqchar != branchreqchar) reqcharflags = REQ_NONE;
				else
				{
					reqchar = branchreqchar;
					reqcharflags |= branchreqcharflags;
				}
			}
			if(lookbehind)
			{
				ssh_l fixed_length;
				*code = OP_END;
				fixed_length = find_fixedlength(last_branch, (options & REGEX_UTF8) != 0, FALSE, cd, NULL);
				if(fixed_length == -3) cd->check_lookbehind = TRUE;
				else if(fixed_length < 0)
				{
					*errorcodeptr = (fixed_length == -2) ? ERR36 : (fixed_length == -4) ? ERR70 : ERR25;
					*ptrptr = ptr;
					return FALSE;
				}
				else
				{
					if(fixed_length > cd->max_lookbehind) cd->max_lookbehind = fixed_length;
					reverse_count[0] = (ssh_ws)fixed_length;
				}
			}
		}
		if(*ptr != CHAR_VERTICAL_LINE)
		{
			if(lengthptr == NULL)
			{
				ssh_l branch_length = (ssh_l)(code - last_branch);
				do
				{
					ssh_l prev_length = GET(last_branch, 1);
					last_branch[1] = (ssh_ws)branch_length;
					branch_length = prev_length;
					last_branch -= branch_length;
				} while(branch_length > 0);
			}
			*code = OP_KET;
			code[1] = (ssh_ws)(ssh_l)(code - start_bracket);
			code += 1 + LINK_SIZE;
			if(capnumber > 0)
			{
				if(cd->open_caps->flag)
				{
					*code = OP_END;
					adjust_recurse(start_bracket, 1 + LINK_SIZE, (options & REGEX_UTF8) != 0, cd, save_hwm_offset);
					memmove(start_bracket + 1 + LINK_SIZE, start_bracket, IN_UCHARS(code - start_bracket));
					*start_bracket = OP_ONCE;
					code += 1 + LINK_SIZE;
					start_bracket[1] = (ssh_ws)(ssh_l)(code - start_bracket);
					*code = OP_KET;
					code[1] = (ssh_ws)(ssh_l)(code - start_bracket);
					code += 1 + LINK_SIZE;
					length += 2 + 2 * LINK_SIZE;
				}
				cd->open_caps = cd->open_caps->next;
			}
			cd->bracount = max_bracount;
			*codeptr = code;
			*ptrptr = ptr;
			*firstcharptr = firstchar;
			*firstcharflagsptr = firstcharflags;
			*reqcharptr = reqchar;
			*reqcharflagsptr = reqcharflags;
			if(lengthptr != NULL)
			{
				if(OFLOW_MAX - *lengthptr < length)
				{
					*errorcodeptr = ERR20;
					return FALSE;
				}
				*lengthptr += length;
			}
			return TRUE;
		}
		if(lengthptr != NULL)
		{
			code = *codeptr + 1 + LINK_SIZE + skipbytes;
			length += 1 + LINK_SIZE;
		}
		else
		{
			*code = OP_ALT;
			code[1] = (ssh_ws)(ssh_l)(code - last_branch);
			bc.current_branch = last_branch = code;
			code += 1 + LINK_SIZE;
		}

		ptr++;
	}

}

static BOOL is_anchored(ssh_wcs code, ssh_u bracket_map, compile_data *cd, ssh_l atomcount)
{
	do
	{
		ssh_wcs scode = first_significant_code(code + PRIV(OP_lengths)[*code], FALSE);
		register ssh_l op = *scode;
		if(op == OP_BRA || op == OP_BRAPOS || op == OP_SBRA || op == OP_SBRAPOS)
		{
			if(!is_anchored(scode, bracket_map, cd, atomcount)) return FALSE;
		}
		else if(op == OP_CBRA || op == OP_CBRAPOS || op == OP_SCBRA || op == OP_SCBRAPOS)
		{
			ssh_l n = GET2(scode, 1 + LINK_SIZE);
			ssh_l new_map = bracket_map | ((n < 32) ? (1 << n) : 1);
			if(!is_anchored(scode, new_map, cd, atomcount)) return FALSE;
		}
		else if(op == OP_ASSERT || op == OP_COND)
		{
			if(!is_anchored(scode, bracket_map, cd, atomcount)) return FALSE;
		}
		else if(op == OP_ONCE || op == OP_ONCE_NC)
		{
			if(!is_anchored(scode, bracket_map, cd, atomcount + 1))
				return FALSE;
		}
		else if((op == OP_TYPESTAR || op == OP_TYPEMINSTAR || op == OP_TYPEPOSSTAR))
		{
			if(scode[1] != OP_ALLANY || (bracket_map & cd->backref_map) != 0 || atomcount > 0 || cd->had_pruneorskip)
			   return FALSE;
		}
		else if(op != OP_SOD && op != OP_SOM && op != OP_CIRC) return FALSE;
		code += GET(code, 1);
	} while(*code == OP_ALT);
	return TRUE;
}

static BOOL is_startline(ssh_wcs code, ssh_u bracket_map, compile_data *cd, ssh_l atomcount)
{
	do
	{
		ssh_wcs scode = first_significant_code( code + PRIV(OP_lengths)[*code], FALSE);
		register ssh_l op = *scode;
		if(op == OP_COND)
		{
			scode += 1 + LINK_SIZE;
			if(*scode == OP_CALLOUT) scode += PRIV(OP_lengths)[OP_CALLOUT];
			switch(*scode)
			{
				case OP_CREF:
				case OP_DNCREF:
				case OP_RREF:
				case OP_DNRREF:
				case OP_DEF:
				case OP_FAIL:
					return FALSE;
				default:
					if(!is_startline(scode, bracket_map, cd, atomcount)) return FALSE;
					do scode += GET(scode, 1); while(*scode == OP_ALT);
					scode += 1 + LINK_SIZE;
					break;
			}
			scode = first_significant_code(scode, FALSE);
			op = *scode;
		}
		if(op == OP_BRA || op == OP_BRAPOS || op == OP_SBRA || op == OP_SBRAPOS)
		{
			if(!is_startline(scode, bracket_map, cd, atomcount)) return FALSE;
		}
		else if(op == OP_CBRA || op == OP_CBRAPOS || op == OP_SCBRA || op == OP_SCBRAPOS)
		{
			ssh_l n = GET2(scode, 1 + LINK_SIZE);
			ssh_l new_map = bracket_map | ((n < 32) ? (1 << n) : 1);
			if(!is_startline(scode, new_map, cd, atomcount)) return FALSE;
		}
		else if(op == OP_ASSERT)
		{
			if(!is_startline(scode, bracket_map, cd, atomcount)) return FALSE;
		}
		else if(op == OP_ONCE || op == OP_ONCE_NC)
		{
			if(!is_startline(scode, bracket_map, cd, atomcount + 1)) return FALSE;
		}
		else if(op == OP_TYPESTAR || op == OP_TYPEMINSTAR || op == OP_TYPEPOSSTAR)
		{
			if(scode[1] != OP_ANY || (bracket_map & cd->backref_map) != 0 || atomcount > 0 || cd->had_pruneorskip)
			   return FALSE;
		}
		else if(op != OP_CIRC && op != OP_CIRCM) return FALSE;
		code += GET(code, 1);
	} while(*code == OP_ALT);
	return TRUE;
}

static ssh_u find_firstassertedchar(ssh_wcs code, ssh_l *flags, BOOL inassert)
{
	register ssh_u c = 0;
	ssh_l cflags = REQ_NONE;
	*flags = REQ_NONE;
	do
	{
		ssh_u d;
		ssh_l dflags;
		ssh_l xl = (*code == OP_CBRA || *code == OP_SCBRA || *code == OP_CBRAPOS || *code == OP_SCBRAPOS) ? IMM2_SIZE : 0;
		const ssh_ws *scode = first_significant_code(code + 1 + LINK_SIZE + xl, TRUE);
		register ssh_ws op = *scode;
		switch(op)
		{
			default:
				return 0;
			case OP_BRA:
			case OP_BRAPOS:
			case OP_CBRA:
			case OP_SCBRA:
			case OP_CBRAPOS:
			case OP_SCBRAPOS:
			case OP_ASSERT:
			case OP_ONCE:
			case OP_ONCE_NC:
				d = find_firstassertedchar(scode, &dflags, op == OP_ASSERT);
				if(dflags < 0) return 0;
				if(cflags < 0) { c = d; cflags = dflags; }
				else if(c != d || cflags != dflags) return 0;
				break;
			case OP_EXACT:
				scode += IMM2_SIZE;
			case OP_CHAR:
			case OP_PLUS:
			case OP_MINPLUS:
			case OP_POSPLUS:
				if(!inassert) return 0;
				if(cflags < 0) { c = scode[1]; cflags = 0; }
				else if(c != scode[1]) return 0;
				break;

			case OP_EXACTI:
				scode += IMM2_SIZE;
			case OP_CHARI:
			case OP_PLUSI:
			case OP_MINPLUSI:
			case OP_POSPLUSI:
				if(!inassert) return 0;
				if(cflags < 0) { c = scode[1]; cflags = REQ_CASELESS; }
				else if(c != scode[1]) return 0;
				break;
		}
		code += GET(code, 1);
	} while(*code == OP_ALT);
	*flags = cflags;
	return c;
}

static void add_name(compile_data *cd, ssh_wcs name, ssh_l length, ssh_u groupno)
{
	ssh_l i;
	ssh_ws *slot = cd->name_table;

	for(i = 0; i < cd->names_found; i++)
	{
		ssh_l crc = memcmp(name, slot + IMM2_SIZE, IN_UCHARS(length));
		if(crc == 0 && slot[IMM2_SIZE + length] != 0) crc = -1;
		if(crc < 0)
		{
			memmove(slot + cd->name_entry_size, slot, IN_UCHARS((cd->names_found - i) * cd->name_entry_size));
			break;
		}
		slot += cd->name_entry_size;
	}
	PUT2(slot, 0, groupno);
	memcpy(slot + IMM2_SIZE, name, IN_UCHARS(length));
	slot[IMM2_SIZE + length] = 0;
	cd->names_found++;
}

REGEX_EXP_DEFN void regex_free(void* p)
{
	::free(p);
}

REGEX_EXP_DEFN regex16* regex16_compile(REGEX_SPTR16 pattern, ssh_l options)
{
	REAL_PCRE *re;
	ssh_l length = 1;
	ssh_l firstcharflags, reqcharflags;
	ssh_u firstchar, reqchar;
	ssh_u limit_match = REGEX_UINT32_MAX;
	ssh_u limit_recursion = REGEX_UINT32_MAX;
	ssh_l newline;
	ssh_l errorcode = 0;
	ssh_l skipatstart = 0;
	BOOL utf;
	BOOL never_utf = FALSE;
	size_t size;
	ssh_ws *code;
	ssh_wcs codestart;
	ssh_wcs ptr;
	compile_data compile_block;
	compile_data *cd = &compile_block;
	ssh_ws cworkspace[COMPILE_WORK_SIZE];
	named_group named_groups[NAMED_GROUP_LIST_SIZE];
	ptr = (const ssh_ws*)pattern;
	const unsigned char* tables(PRIV(default_tables));
	cd->lcc = tables + lcc_offset;
	cd->fcc = tables + fcc_offset;
	cd->cbits = tables + cbits_offset;
	cd->ctypes = tables + ctypes_offset;
	if((options & ~PUBLIC_COMPILE_OPTIONS) != 0)
	{
		errorcode = ERR17;
		goto REGEX_EARLY_ERROR_RETURN;
	}
	if((options & REGEX_NEVER_UTF) != 0) never_utf = TRUE;
	cd->external_flags = 0;
	while(ptr[skipatstart] == CHAR_LEFT_PARENTHESIS && ptr[skipatstart + 1] == CHAR_ASTERISK)
	{
		ssh_l newnl = 0;
		ssh_l newbsr = 0;
		if(STRNCMP_UC_C8(ptr + skipatstart + 2, STRING_UTF16_RIGHTPAR, 6) == 0) { skipatstart += 8; options |= REGEX_UTF16; continue; }
		else if(STRNCMP_UC_C8(ptr + skipatstart + 2, STRING_UTF_RIGHTPAR, 4) == 0) { skipatstart += 6; options |= REGEX_UTF8; continue; }
		else if(STRNCMP_UC_C8(ptr + skipatstart + 2, STRING_UCP_RIGHTPAR, 4) == 0) { skipatstart += 6; options |= REGEX_UCP; continue; }
		else if(STRNCMP_UC_C8(ptr + skipatstart + 2, STRING_NO_AUTO_POSSESS_RIGHTPAR, 16) == 0) { skipatstart += 18; options |= REGEX_NO_AUTO_POSSESS; continue; }
		else if(STRNCMP_UC_C8(ptr + skipatstart + 2, STRING_NO_START_OPT_RIGHTPAR, 13) == 0) { skipatstart += 15; options |= REGEX_NO_START_OPTIMIZE; continue; }
		else if(STRNCMP_UC_C8(ptr + skipatstart + 2, STRING_LIMIT_MATCH_EQ, 12) == 0)
		{
			ssh_u c = 0;
			ssh_l p = skipatstart + 14;
			while(isdigit(ptr[p]))
			{
				if(c > REGEX_UINT32_MAX / 10 - 1) break;
				c = c * 10 + ptr[p++] - CHAR_0;
			}
			if(ptr[p++] != CHAR_RIGHT_PARENTHESIS) break;
			if(c < limit_match)
			{
				limit_match = c;
				cd->external_flags |= REGEX_MLSET;
			}
			skipatstart = p;
			continue;
		}
		else if(STRNCMP_UC_C8(ptr + skipatstart + 2, STRING_LIMIT_RECURSION_EQ, 16) == 0)
		{
			ssh_u c = 0;
			ssh_l p = skipatstart + 18;
			while(isdigit(ptr[p]))
			{
				if(c > REGEX_UINT32_MAX / 10 - 1) break;
				c = c * 10 + ptr[p++] - CHAR_0;
			}
			if(ptr[p++] != CHAR_RIGHT_PARENTHESIS) break;
			if(c < limit_recursion)
			{
				limit_recursion = c;
				cd->external_flags |= REGEX_RLSET;
			}
			skipatstart = p;
			continue;
		}
		if(STRNCMP_UC_C8(ptr + skipatstart + 2, STRING_CR_RIGHTPAR, 3) == 0) { skipatstart += 5; newnl = REGEX_NEWLINE_CR; }
		else if(STRNCMP_UC_C8(ptr + skipatstart + 2, STRING_LF_RIGHTPAR, 3) == 0) { skipatstart += 5; newnl = REGEX_NEWLINE_LF; }
		else if(STRNCMP_UC_C8(ptr + skipatstart + 2, STRING_CRLF_RIGHTPAR, 5) == 0) { skipatstart += 7; newnl = REGEX_NEWLINE_CR + REGEX_NEWLINE_LF; }
		else if(STRNCMP_UC_C8(ptr + skipatstart + 2, STRING_ANY_RIGHTPAR, 4) == 0) { skipatstart += 6; newnl = REGEX_NEWLINE_ANY; }
		else if(STRNCMP_UC_C8(ptr + skipatstart + 2, STRING_ANYCRLF_RIGHTPAR, 8) == 0) { skipatstart += 10; newnl = REGEX_NEWLINE_ANYCRLF; }
		else if(STRNCMP_UC_C8(ptr + skipatstart + 2, STRING_BSR_ANYCRLF_RIGHTPAR, 12) == 0) { skipatstart += 14; newbsr = REGEX_BSR_ANYCRLF; }
		else if(STRNCMP_UC_C8(ptr + skipatstart + 2, STRING_BSR_UNICODE_RIGHTPAR, 12) == 0) { skipatstart += 14; newbsr = REGEX_BSR_UNICODE; }
		if(newnl != 0) options = (options & ~REGEX_NEWLINE_BITS) | newnl;
		else if(newbsr != 0) options = (options & ~(REGEX_BSR_ANYCRLF | REGEX_BSR_UNICODE)) | newbsr;
		else break;
	}
	utf = (options & REGEX_UTF8) != 0;
	if(utf && never_utf)
	{
		errorcode = ERR78;
		goto REGEX_EARLY_ERROR_RETURN2;
	}
	if(utf)
	{
		errorcode = ERR32;
		goto REGEX_EARLY_ERROR_RETURN;
	}
	if((options & REGEX_UCP) != 0)
	{
		errorcode = ERR67;
		goto REGEX_EARLY_ERROR_RETURN;
	}
	if((options & (REGEX_BSR_ANYCRLF | REGEX_BSR_UNICODE)) == (REGEX_BSR_ANYCRLF | REGEX_BSR_UNICODE))
	{
		errorcode = ERR56;
		goto REGEX_EARLY_ERROR_RETURN;
	}
	switch(options & REGEX_NEWLINE_BITS)
	{
		case 0: newline = NEWLINE; break;
		case REGEX_NEWLINE_CR: newline = CHAR_CR; break;
		case REGEX_NEWLINE_LF: newline = CHAR_NL; break;
		case REGEX_NEWLINE_CR + REGEX_NEWLINE_LF: newline = (CHAR_CR << 8) | CHAR_NL; break;
		case REGEX_NEWLINE_ANY: newline = -1; break;
		case REGEX_NEWLINE_ANYCRLF: newline = -2; break;
		default: errorcode = ERR56; goto REGEX_EARLY_ERROR_RETURN;
	}
	if(newline == -2)
	{
		cd->nltype = NLTYPE_ANYCRLF;
	}
	else if(newline < 0)
	{
		cd->nltype = NLTYPE_ANY;
	}
	else
	{
		cd->nltype = NLTYPE_FIXED;
		if(newline > 255)
		{
			cd->nllen = 2;
			cd->nl[0] = (newline >> 8) & 255;
			cd->nl[1] = newline & 255;
		}
		else
		{
			cd->nllen = 1;
			cd->nl[0] = (ssh_ws)newline;
		}
	}
	cd->top_backref = 0;
	cd->backref_map = 0;
	cd->bracount = cd->final_bracount = 0;
	cd->names_found = 0;
	cd->name_entry_size = 0;
	cd->name_table = NULL;
	cd->dupnames = FALSE;
	cd->namedrefcount = 0;
	cd->start_code = cworkspace;
	cd->hwm = cworkspace;
	cd->iscondassert = FALSE;
	cd->start_workspace = cworkspace;
	cd->workspace_size = COMPILE_WORK_SIZE;
	cd->named_groups = named_groups;
	cd->named_group_list_size = NAMED_GROUP_LIST_SIZE;
	cd->start_pattern = (const ssh_ws *)pattern;
	cd->end_pattern = (const ssh_ws *)(pattern + wcslen(pattern));
	cd->req_varyopt = 0;
	cd->parens_depth = 0;
	cd->assert_depth = 0;
	cd->max_lookbehind = 0;
	cd->external_options = options;
	cd->open_caps = NULL;
	ptr += skipatstart;
	code = cworkspace;
	*code = OP_BRA;
	(void)compile_regex(cd->external_options, &code, &ptr, &errorcode, FALSE, FALSE, 0, 0, &firstchar, &firstcharflags, &reqchar, &reqcharflags, NULL, cd, &length);
	if(errorcode != 0) goto REGEX_EARLY_ERROR_RETURN;
	if(length > MAX_PATTERN_SIZE)
	{
		errorcode = ERR20;
		goto REGEX_EARLY_ERROR_RETURN;
	}
	size = sizeof(REAL_PCRE) + (length + cd->names_found * cd->name_entry_size) * sizeof(ssh_ws);
	re = (REAL_PCRE *)malloc(size);
	re->magic_number = MAGIC_NUMBER;
	re->size = (ssh_l)size;
	re->options = cd->external_options;
	re->flags = cd->external_flags;
	re->limit_match = limit_match;
	re->limit_recursion = limit_recursion;
	re->first_char = 0;
	re->req_char = 0;
	re->name_table_offset = sizeof(REAL_PCRE) / sizeof(ssh_ws);
	re->name_entry_size = (ssh_ws)cd->name_entry_size;
	re->name_count = (ssh_ws)cd->names_found;
	re->ref_count = 0;
	re->tables = (tables == PRIV(default_tables)) ? NULL : tables;
	re->nullpad = NULL;
	re->dummy1 = re->dummy2 = re->dummy3 = 0;
	cd->final_bracount = cd->bracount;
	cd->parens_depth = 0;
	cd->assert_depth = 0;
	cd->bracount = 0;
	cd->max_lookbehind = 0;
	cd->name_table = (ssh_ws *)re + re->name_table_offset;
	codestart = cd->name_table + re->name_entry_size * re->name_count;
	cd->start_code = codestart;
	cd->hwm = (ssh_ws *)(cd->start_workspace);
	cd->iscondassert = FALSE;
	cd->req_varyopt = 0;
	cd->had_accept = FALSE;
	cd->had_pruneorskip = FALSE;
	cd->check_lookbehind = FALSE;
	cd->open_caps = NULL;
	if(cd->names_found > 0)
	{
		ssh_l i = cd->names_found;
		named_group *ng = cd->named_groups;
		cd->names_found = 0;
		for(; i > 0; i--, ng++) add_name(cd, ng->name, ng->length, ng->number);
		if(cd->named_group_list_size > NAMED_GROUP_LIST_SIZE) free((void *)cd->named_groups);
	}
	ptr = (ssh_wcs)pattern + skipatstart;
	code = (ssh_ws*)codestart;
	*code = OP_BRA;
	(void)compile_regex(re->options, &code, &ptr, &errorcode, FALSE, FALSE, 0, 0, &firstchar, &firstcharflags, &reqchar, &reqcharflags, NULL, cd, NULL);
	re->top_bracket = (ssh_ws)cd->bracount;
	re->top_backref = (ssh_ws)cd->top_backref;
	re->max_lookbehind = (ssh_ws)cd->max_lookbehind;
	re->flags = cd->external_flags | REGEX_MODE;
	if(cd->had_accept)
	{
		reqchar = 0;
		reqcharflags = REQ_NONE;
	}
	if(errorcode == 0 && *ptr != CHAR_NULL) errorcode = ERR22;
	*code++ = OP_END;
	if(cd->hwm > cd->start_workspace)
	{
		ssh_l prev_recno = -1;
		ssh_wcs groupptr = NULL;
		while(errorcode == 0 && cd->hwm > cd->start_workspace)
		{
			ssh_l offset, recno;
			cd->hwm -= LINK_SIZE;
			offset = GET(cd->hwm, 0);
			recno = GET(codestart, offset);
			if(recno != prev_recno)
			{
				groupptr = PRIV(find_bracket)(codestart, utf, recno);
				prev_recno = recno;
			}
			if(groupptr == NULL) errorcode = ERR53;
			else PUT2(((ssh_ws*)codestart), offset, (int)(groupptr - codestart));
		}
	}
	if(cd->workspace_size > COMPILE_WORK_SIZE) free((void *)cd->start_workspace);
	cd->start_workspace = NULL;
	if(errorcode == 0 && re->top_backref > re->top_bracket) errorcode = ERR15;
	if((options & REGEX_NO_AUTO_POSSESS) == 0)
	{
		ssh_ws *temp = (ssh_ws *)codestart;
		auto_possessify(temp, utf, cd);
	}
	if(cd->check_lookbehind)
	{
		ssh_ws *cc = (ssh_ws *)codestart;
		for(cc = (ssh_ws *)PRIV(find_bracket)(codestart, utf, -1); cc != NULL; cc = (ssh_ws *)PRIV(find_bracket)(cc, utf, -1))
		{
			if(GET(cc, 1) == 0)
			{
				ssh_l fixed_length;
				ssh_ws *be = cc - 1 - LINK_SIZE + GET(cc, -LINK_SIZE);
				ssh_l end_op = *be;
				*be = OP_END;
				fixed_length = find_fixedlength(cc, (re->options & REGEX_UTF8) != 0, TRUE, cd, NULL);
				*be = (ssh_ws)end_op;
				if(fixed_length < 0)
				{
					errorcode = (fixed_length == -2) ? ERR36 : (fixed_length == -4) ? ERR70 : ERR25;
					break;
				}
				if(fixed_length > cd->max_lookbehind) cd->max_lookbehind = fixed_length;
				cc[1] = (ssh_ws)fixed_length;
			}
			cc += 1 + LINK_SIZE;
		}
	}
	if(errorcode != 0)
	{
		free(re);
REGEX_EARLY_ERROR_RETURN:
REGEX_EARLY_ERROR_RETURN2:
		return nullptr;
	}
	if((re->options & REGEX_ANCHORED) == 0)
	{
		if(is_anchored(codestart, 0, cd, 0)) re->options |= REGEX_ANCHORED;
		else
		{
			if(firstcharflags < 0) firstchar = find_firstassertedchar(codestart, &firstcharflags, FALSE);
			if(firstcharflags >= 0)
			{
				re->first_char = firstchar & 0xffff;
				if((firstcharflags & REQ_CASELESS) != 0)
				{
					if(MAX_255(re->first_char) && cd->fcc[re->first_char] != re->first_char)
						re->flags |= REGEX_FCH_CASELESS;
				}
				re->flags |= REGEX_FIRSTSET;
			}
			else if(is_startline(codestart, 0, cd, 0)) re->flags |= REGEX_STARTLINE;
		}
	}
	if(reqcharflags >= 0 && ((re->options & REGEX_ANCHORED) == 0 || (reqcharflags & REQ_VARY) != 0))
	{
		re->req_char = reqchar & 0xffff;
		if((reqcharflags & REQ_CASELESS) != 0)
		{
			if(MAX_255(re->req_char) && cd->fcc[re->req_char] != re->req_char)
				re->flags |= REGEX_RCH_CASELESS;
		}
		re->flags |= REGEX_REQCHSET;
	}
	do
	{
		if(could_be_empty_branch(codestart, code, utf, cd, NULL))
		{
			re->flags |= REGEX_MATCH_EMPTY;
			break;
		}
		codestart += GET(codestart, 1);
	} while(*codestart == OP_ALT);
	return (regex16 *)re;
}
