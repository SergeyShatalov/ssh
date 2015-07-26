
#pragma once

#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN             // Исключите редко используемые компоненты из заголовков Windows

#include <windows.h>
#include <stdlib.h>
#include <Common\regex_config.h>

#ifndef REGEX_EXP_DECL
	#define REGEX_EXP_DECL       extern __declspec(dllexport)
	#define REGEX_EXP_DEFN       __declspec(dllexport)
	#define REGEX_EXP_DATA_DEFN  __declspec(dllexport)
#endif 

#define REGEX_UINT32_MAX UINT_MAX
#define REGEX_INT32_MAX INT_MAX
#define REGEX_UINT16_MAX USHRT_MAX
#define REGEX_INT16_MAX SHRT_MAX
#define INT64_OR_DOUBLE double
#define UCHAR_SHIFT (1)
#define IN_UCHARS(x) ((x) << UCHAR_SHIFT)
#define MAX_255(c) ((c) <= 255u)
#define TABLE_GET(c, table, default) (MAX_255(c)? ((table)[c]):(default))
#define NOTACHAR 0xffffffff
#define NLTYPE_FIXED    0     
#define NLTYPE_ANY      1     
#define NLTYPE_ANYCRLF  2     
#define IS_NEWLINE(p) ((NLBLOCK->nltype != NLTYPE_FIXED) ? ((p) < NLBLOCK->PSEND && PRIV(is_newline)((p), NLBLOCK->nltype, NLBLOCK->PSEND, &(NLBLOCK->nllen), utf)) : ((p) <= NLBLOCK->PSEND - NLBLOCK->nllen && UCHAR21TEST(p) == NLBLOCK->nl[0] && (NLBLOCK->nllen == 1 || UCHAR21TEST(p+1) == NLBLOCK->nl[1])))
#define WAS_NEWLINE(p) ((NLBLOCK->nltype != NLTYPE_FIXED) ? ((p) > NLBLOCK->PSSTART && PRIV(was_newline)((p), NLBLOCK->nltype, NLBLOCK->PSSTART, &(NLBLOCK->nllen), utf)) : ((p) >= NLBLOCK->PSSTART + NLBLOCK->nllen && UCHAR21TEST(p - NLBLOCK->nllen) == NLBLOCK->nl[0] && (NLBLOCK->nllen == 1 || UCHAR21TEST(p - NLBLOCK->nllen + 1) == NLBLOCK->nl[1])))
#define REGEX_PUCHAR const ssh_ws *

#include <Common\regex.h>

static void* regex_memmove(void *d, const void *s, size_t n)
{
	size_t i;
	unsigned char *dest = (unsigned char *)d;
	const unsigned char *src = (const unsigned char *)s;
	if(dest > src)
	{
		dest += n;
		src += n;
		for(i = 0; i < n; ++i) *(--dest) = *(--src);
		return (void *)dest;
	}
	else
	{
		for(i = 0; i < n; ++i) *dest++ = *src++;
		return (void *)(dest - n);
	}
}
#define memmove(a, b, c) regex_memmove(a, b, c)

#if LINK_SIZE == 2
#undef LINK_SIZE
#define LINK_SIZE 1
#define PUT2(a,n,d) a[n] = (ssh_ws)(d)
#define GET(a,n) (a[n])
#define MAX_PATTERN_SIZE (1 << 16)
#elif LINK_SIZE == 3 || LINK_SIZE == 4
#undef LINK_SIZE
#define LINK_SIZE 2
#define PUT(a,n,d) (a[n] = (d) >> 16), (a[(n)+1] = (d) & 65535)
#define GET(a,n) (((a)[n] << 16) | (a)[(n)+1])
#define MAX_PATTERN_SIZE (1 << 30)
#else
#error LINK_SIZE must be either 2, 3, or 4
#endif
#define IMM2_SIZE 1
#define GET2(a,n) a[n]
#define PUT2INC(a,n,d)  PUT2(a,n,d), a += IMM2_SIZE
#define MAX_MARK ((1u << 16) - 1)
#define UCHAR21(eptr)        (*(eptr))
#define UCHAR21TEST(eptr)    (*(eptr))
#define UCHAR21INC(eptr)     (*(eptr)++)
#define UCHAR21INCTEST(eptr) (*(eptr)++)
#define GETCHAR(c, eptr) c = *eptr;
#define GETCHARTEST(c, eptr) c = *eptr;
#define GETCHARINC(c, eptr) c = *eptr++;
#define GETCHARINCTEST(c, eptr) c = *eptr++;
#define GETCHARLEN(c, eptr, len) c = *eptr;



#define HSPACE_LIST \
  CHAR_HT, CHAR_SPACE, 0xa0, \
  0x1680, 0x180e, 0x2000, 0x2001, 0x2002, 0x2003, 0x2004, 0x2005, \
  0x2006, 0x2007, 0x2008, 0x2009, 0x200A, 0x202f, 0x205f, 0x3000, \
  NOTACHAR

#define HSPACE_MULTIBYTE_CASES \
  case 0x1680:   \
  case 0x180e:   \
  case 0x2000:   \
  case 0x2001:   \
  case 0x2002:   \
  case 0x2003:   \
  case 0x2004:   \
  case 0x2005:   \
  case 0x2006:   \
  case 0x2007:   \
  case 0x2008:   \
  case 0x2009:   \
  case 0x200A:   \
  case 0x202f:   \
  case 0x205f:   \
  case 0x3000   

#define HSPACE_BYTE_CASES \
  case CHAR_HT: \
  case CHAR_SPACE: \
  case 0xa0     

#define HSPACE_CASES \
  HSPACE_BYTE_CASES: \
  HSPACE_MULTIBYTE_CASES

#define VSPACE_LIST \
  CHAR_LF, CHAR_VT, CHAR_FF, CHAR_CR, CHAR_NEL, 0x2028, 0x2029, NOTACHAR

#define VSPACE_MULTIBYTE_CASES \
  case 0x2028:     \
  case 0x2029     

#define VSPACE_BYTE_CASES \
  case CHAR_LF: \
  case CHAR_VT: \
  case CHAR_FF: \
  case CHAR_CR: \
  case CHAR_NEL

#define VSPACE_CASES \
  VSPACE_BYTE_CASES: \
  VSPACE_MULTIBYTE_CASES
#define REGEX_MODE8         0x00000001  
#define REGEX_MODE16        0x00000002  
#define REGEX_MODE32        0x00000004  
#define REGEX_FIRSTSET      0x00000010  
#define REGEX_FCH_CASELESS  0x00000020  
#define REGEX_REQCHSET      0x00000040  
#define REGEX_RCH_CASELESS  0x00000080  
#define REGEX_STARTLINE     0x00000100  
#define REGEX_NOPARTIAL     0x00000200  
#define REGEX_JCHANGED      0x00000400  
#define REGEX_HASCRORLF     0x00000800  
#define REGEX_HASTHEN       0x00001000  
#define REGEX_MLSET         0x00002000  
#define REGEX_RLSET         0x00004000  
#define REGEX_MATCH_EMPTY   0x00008000  
#define REGEX_MODE          REGEX_MODE16
#define REGEX_MODE_MASK     (REGEX_MODE8 | REGEX_MODE16 | REGEX_MODE32)
#define REGEX_STUDY_MAPPED  0x0001  
#define REGEX_STUDY_MINLEN  0x0002  
#define REGEX_NEWLINE_BITS (REGEX_NEWLINE_CR|REGEX_NEWLINE_LF|REGEX_NEWLINE_ANY|REGEX_NEWLINE_ANYCRLF)
#define PUBLIC_COMPILE_OPTIONS \
  (REGEX_CASELESS|REGEX_EXTENDED|REGEX_ANCHORED|REGEX_MULTILINE| \
   REGEX_DOTALL|REGEX_DOLLAR_ENDONLY|REGEX_EXTRA|REGEX_UNGREEDY|REGEX_UTF8| \
   REGEX_NO_AUTO_CAPTURE|REGEX_NO_AUTO_POSSESS| \
   REGEX_NO_UTF8_CHECK|REGEX_AUTO_CALLOUT|REGEX_FIRSTLINE| \
   REGEX_DUPNAMES|REGEX_NEWLINE_BITS|REGEX_BSR_ANYCRLF|REGEX_BSR_UNICODE| \
   REGEX_JAVASCRIPT_COMPAT|REGEX_UCP|REGEX_NO_START_OPTIMIZE|REGEX_NEVER_UTF)

#define PUBLIC_EXEC_OPTIONS \
  (REGEX_ANCHORED|REGEX_NOTBOL|REGEX_NOTEOL|REGEX_NOTEMPTY|REGEX_NOTEMPTY_ATSTART| \
   REGEX_NO_UTF8_CHECK|REGEX_PARTIAL_HARD|REGEX_PARTIAL_SOFT|REGEX_NEWLINE_BITS| \
   REGEX_BSR_ANYCRLF|REGEX_BSR_UNICODE|REGEX_NO_START_OPTIMIZE)

#define PUBLIC_DFA_EXEC_OPTIONS \
  (REGEX_ANCHORED|REGEX_NOTBOL|REGEX_NOTEOL|REGEX_NOTEMPTY|REGEX_NOTEMPTY_ATSTART| \
   REGEX_NO_UTF8_CHECK|REGEX_PARTIAL_HARD|REGEX_PARTIAL_SOFT|REGEX_DFA_SHORTEST| \
   REGEX_DFA_RESTART|REGEX_NEWLINE_BITS|REGEX_BSR_ANYCRLF|REGEX_BSR_UNICODE| \
   REGEX_NO_START_OPTIMIZE)

#define MAGIC_NUMBER  0x50435245UL   
#define REVERSED_MAGIC_NUMBER  0x45524350UL   
#define REQ_BYTE_MAX 1000
typedef int BOOL;

#ifndef FALSE
#define FALSE   0
#define TRUE    1
#endif

#define CHAR_LF                     '\n'
#define CHAR_NL                     CHAR_LF
#define CHAR_NEL                    ((unsigned char)'\x85')
#define CHAR_ESC                    '\033'
#define CHAR_DEL                    '\177'

#define STR_LF                      "\n"
#define STR_NL                      STR_LF
#define STR_NEL                     "\x85"
#define STR_ESC                     "\033"
#define STR_DEL                     "\177"


#define CHAR_NULL                   '\0'
#define CHAR_HT                     '\t'
#define CHAR_VT                     '\v'
#define CHAR_FF                     '\f'
#define CHAR_CR                     '\r'
#define CHAR_BS                     '\b'
#define CHAR_BEL                    '\a'

#define CHAR_SPACE                  ' '
#define CHAR_EXCLAMATION_MARK       '!'
#define CHAR_QUOTATION_MARK         '"'
#define CHAR_NUMBER_SIGN            '#'
#define CHAR_DOLLAR_SIGN            '$'
#define CHAR_PERCENT_SIGN           '%'
#define CHAR_AMPERSAND              '&'
#define CHAR_APOSTROPHE             '\''
#define CHAR_LEFT_PARENTHESIS       '('
#define CHAR_RIGHT_PARENTHESIS      ')'
#define CHAR_ASTERISK               '*'
#define CHAR_PLUS                   '+'
#define CHAR_COMMA                  ','
#define CHAR_MINUS                  '-'
#define CHAR_DOT                    '.'
#define CHAR_SLASH                  '/'
#define CHAR_0                      '0'
#define CHAR_1                      '1'
#define CHAR_2                      '2'
#define CHAR_3                      '3'
#define CHAR_4                      '4'
#define CHAR_5                      '5'
#define CHAR_6                      '6'
#define CHAR_7                      '7'
#define CHAR_8                      '8'
#define CHAR_9                      '9'
#define CHAR_COLON                  ':'
#define CHAR_SEMICOLON              ';'
#define CHAR_LESS_THAN_SIGN         '<'
#define CHAR_EQUALS_SIGN            '='
#define CHAR_GREATER_THAN_SIGN      '>'
#define CHAR_QUESTION_MARK          '?'
#define CHAR_COMMERCIAL_AT          '@'
#define CHAR_A                      'A'
#define CHAR_B                      'B'
#define CHAR_C                      'C'
#define CHAR_D                      'D'
#define CHAR_E                      'E'
#define CHAR_F                      'F'
#define CHAR_G                      'G'
#define CHAR_H                      'H'
#define CHAR_I                      'I'
#define CHAR_J                      'J'
#define CHAR_K                      'K'
#define CHAR_L                      'L'
#define CHAR_M                      'M'
#define CHAR_N                      'N'
#define CHAR_O                      'O'
#define CHAR_P                      'P'
#define CHAR_Q                      'Q'
#define CHAR_R                      'R'
#define CHAR_S                      'S'
#define CHAR_T                      'T'
#define CHAR_U                      'U'
#define CHAR_V                      'V'
#define CHAR_W                      'W'
#define CHAR_X                      'X'
#define CHAR_Y                      'Y'
#define CHAR_Z                      'Z'
#define CHAR_LEFT_SQUARE_BRACKET    '['
#define CHAR_BACKSLASH              '\\'
#define CHAR_RIGHT_SQUARE_BRACKET   ']'
#define CHAR_CIRCUMFLEX_ACCENT      '^'
#define CHAR_UNDERSCORE             '_'
#define CHAR_GRAVE_ACCENT           '`'
#define CHAR_a                      'a'
#define CHAR_b                      'b'
#define CHAR_c                      'c'
#define CHAR_d                      'd'
#define CHAR_e                      'e'
#define CHAR_f                      'f'
#define CHAR_g                      'g'
#define CHAR_h                      'h'
#define CHAR_i                      'i'
#define CHAR_j                      'j'
#define CHAR_k                      'k'
#define CHAR_l                      'l'
#define CHAR_m                      'm'
#define CHAR_n                      'n'
#define CHAR_o                      'o'
#define CHAR_p                      'p'
#define CHAR_q                      'q'
#define CHAR_r                      'r'
#define CHAR_s                      's'
#define CHAR_t                      't'
#define CHAR_u                      'u'
#define CHAR_v                      'v'
#define CHAR_w                      'w'
#define CHAR_x                      'x'
#define CHAR_y                      'y'
#define CHAR_z                      'z'
#define CHAR_LEFT_CURLY_BRACKET     '{'
#define CHAR_VERTICAL_LINE          '|'
#define CHAR_RIGHT_CURLY_BRACKET    '}'
#define CHAR_TILDE                  '~'

#define STR_HT                      "\t"
#define STR_VT                      "\v"
#define STR_FF                      "\f"
#define STR_CR                      "\r"
#define STR_BS                      "\b"
#define STR_BEL                     "\a"

#define STR_SPACE                   " "
#define STR_EXCLAMATION_MARK        "!"
#define STR_QUOTATION_MARK          "\""
#define STR_NUMBER_SIGN             "#"
#define STR_DOLLAR_SIGN             "$"
#define STR_PERCENT_SIGN            "%"
#define STR_AMPERSAND               "&"
#define STR_APOSTROPHE              "'"
#define STR_LEFT_PARENTHESIS        "("
#define STR_RIGHT_PARENTHESIS       ")"
#define STR_ASTERISK                "*"
#define STR_PLUS                    "+"
#define STR_COMMA                   ","
#define STR_MINUS                   "-"
#define STR_DOT                     "."
#define STR_SLASH                   "/"
#define STR_0                       "0"
#define STR_1                       "1"
#define STR_2                       "2"
#define STR_3                       "3"
#define STR_4                       "4"
#define STR_5                       "5"
#define STR_6                       "6"
#define STR_7                       "7"
#define STR_8                       "8"
#define STR_9                       "9"
#define STR_COLON                   ":"
#define STR_SEMICOLON               ";"
#define STR_LESS_THAN_SIGN          "<"
#define STR_EQUALS_SIGN             "="
#define STR_GREATER_THAN_SIGN       ">"
#define STR_QUESTION_MARK           "?"
#define STR_COMMERCIAL_AT           "@"
#define STR_A                       "A"
#define STR_B                       "B"
#define STR_C                       "C"
#define STR_D                       "D"
#define STR_E                       "E"
#define STR_F                       "F"
#define STR_G                       "G"
#define STR_H                       "H"
#define STR_I                       "I"
#define STR_J                       "J"
#define STR_K                       "K"
#define STR_L                       "L"
#define STR_M                       "M"
#define STR_N                       "N"
#define STR_O                       "O"
#define STR_P                       "P"
#define STR_Q                       "Q"
#define STR_R                       "R"
#define STR_S                       "S"
#define STR_T                       "T"
#define STR_U                       "U"
#define STR_V                       "V"
#define STR_W                       "W"
#define STR_X                       "X"
#define STR_Y                       "Y"
#define STR_Z                       "Z"
#define STR_LEFT_SQUARE_BRACKET     "["
#define STR_BACKSLASH               "\\"
#define STR_RIGHT_SQUARE_BRACKET    "]"
#define STR_CIRCUMFLEX_ACCENT       "^"
#define STR_UNDERSCORE              "_"
#define STR_GRAVE_ACCENT            "`"
#define STR_a                       "a"
#define STR_b                       "b"
#define STR_c                       "c"
#define STR_d                       "d"
#define STR_e                       "e"
#define STR_f                       "f"
#define STR_g                       "g"
#define STR_h                       "h"
#define STR_i                       "i"
#define STR_j                       "j"
#define STR_k                       "k"
#define STR_l                       "l"
#define STR_m                       "m"
#define STR_n                       "n"
#define STR_o                       "o"
#define STR_p                       "p"
#define STR_q                       "q"
#define STR_r                       "r"
#define STR_s                       "s"
#define STR_t                       "t"
#define STR_u                       "u"
#define STR_v                       "v"
#define STR_w                       "w"
#define STR_x                       "x"
#define STR_y                       "y"
#define STR_z                       "z"
#define STR_LEFT_CURLY_BRACKET      "{"
#define STR_VERTICAL_LINE           "|"
#define STR_RIGHT_CURLY_BRACKET     "}"
#define STR_TILDE                   "~"

#define STRING_ACCEPT0              "ACCEPT\0"
#define STRING_COMMIT0              "COMMIT\0"
#define STRING_F0                   "F\0"
#define STRING_FAIL0                "FAIL\0"
#define STRING_MARK0                "MARK\0"
#define STRING_PRUNE0               "PRUNE\0"
#define STRING_SKIP0                "SKIP\0"
#define STRING_THEN                 "THEN"

#define STRING_alpha0               "alpha\0"
#define STRING_lower0               "lower\0"
#define STRING_upper0               "upper\0"
#define STRING_alnum0               "alnum\0"
#define STRING_ascii0               "ascii\0"
#define STRING_blank0               "blank\0"
#define STRING_cntrl0               "cntrl\0"
#define STRING_digit0               "digit\0"
#define STRING_graph0               "graph\0"
#define STRING_print0               "print\0"
#define STRING_punct0               "punct\0"
#define STRING_space0               "space\0"
#define STRING_word0                "word\0"
#define STRING_xdigit               "xdigit"

#define STRING_DEFINE               "DEFINE"
#define STRING_WEIRD_STARTWORD      "[:<:]]"
#define STRING_WEIRD_ENDWORD        "[:>:]]"

#define STRING_CR_RIGHTPAR              "CR)"
#define STRING_LF_RIGHTPAR              "LF)"
#define STRING_CRLF_RIGHTPAR            "CRLF)"
#define STRING_ANY_RIGHTPAR             "ANY)"
#define STRING_ANYCRLF_RIGHTPAR         "ANYCRLF)"
#define STRING_BSR_ANYCRLF_RIGHTPAR     "BSR_ANYCRLF)"
#define STRING_BSR_UNICODE_RIGHTPAR     "BSR_UNICODE)"
#define STRING_UTF8_RIGHTPAR            "UTF8)"
#define STRING_UTF16_RIGHTPAR           "UTF16)"
#define STRING_UTF32_RIGHTPAR           "UTF32)"
#define STRING_UTF_RIGHTPAR             "UTF)"
#define STRING_UCP_RIGHTPAR             "UCP)"
#define STRING_NO_AUTO_POSSESS_RIGHTPAR "NO_AUTO_POSSESS)"
#define STRING_NO_START_OPT_RIGHTPAR    "NO_START_OPT)"
#define STRING_LIMIT_MATCH_EQ           "LIMIT_MATCH="
#define STRING_LIMIT_RECURSION_EQ       "LIMIT_RECURSION="

#ifndef ESC_e
#define ESC_e CHAR_ESC
#endif

#ifndef ESC_f
#define ESC_f CHAR_FF
#endif

#ifndef ESC_n
#define ESC_n CHAR_LF
#endif

#ifndef ESC_r
#define ESC_r CHAR_CR
#endif



#ifndef ESC_tee
#define ESC_tee CHAR_HT
#endif



#define PT_ANY        0    
#define PT_LAMP       1    
#define PT_GC         2    
#define PT_PC         3    
#define PT_SC         4    
#define PT_ALNUM      5    
#define PT_SPACE      6    
#define PT_PXSPACE    7    
#define PT_WORD       8    
#define PT_CLIST      9    
#define PT_UCNC      10    
#define PT_TABSIZE   11    



#define PT_PXGRAPH   11    
#define PT_PXPRINT   12    
#define PT_PXPUNCT   13    



#define XCL_NOT       0x01    
#define XCL_MAP       0x02    
#define XCL_HASPROP   0x04    

#define XCL_END       0    
#define XCL_SINGLE    1    
#define XCL_RANGE     2    
#define XCL_PROP      3    
#define XCL_NOTPROP   4    



enum
{
	ESC_A = 1, ESC_G, ESC_K, ESC_B, ESC_b, ESC_D, ESC_d, ESC_S, ESC_s,
	ESC_W, ESC_w, ESC_N, ESC_dum, ESC_C, ESC_P, ESC_p, ESC_R, ESC_H,
	ESC_h, ESC_V, ESC_v, ESC_X, ESC_Z, ESC_z,
	ESC_E, ESC_Q, ESC_g, ESC_k,
	ESC_DU, ESC_du, ESC_SU, ESC_su, ESC_WU, ESC_wu
};









#define FIRST_AUTOTAB_OP       OP_NOT_DIGIT
#define LAST_AUTOTAB_LEFT_OP   OP_EXTUNI
#define LAST_AUTOTAB_RIGHT_OP  OP_DOLLM

enum
{
	OP_END,



	OP_SOD,
	OP_SOM,
	OP_SET_SOM,
	OP_NOT_WORD_BOUNDARY,
	OP_WORD_BOUNDARY,
	OP_NOT_DIGIT,
	OP_DIGIT,
	OP_NOT_WHITESPACE,
	OP_WHITESPACE,
	OP_NOT_WORDCHAR,
	OP_WORDCHAR,

	OP_ANY,
	OP_ALLANY,
	OP_ANYBYTE,
	OP_NOTPROP,
	OP_PROP,
	OP_ANYNL,
	OP_NOT_HSPACE,
	OP_HSPACE,
	OP_NOT_VSPACE,
	OP_VSPACE,
	OP_EXTUNI,
	OP_EODN,
	OP_EOD,



	OP_DOLL,
	OP_DOLLM,
	OP_CIRC,
	OP_CIRCM,



	OP_CHAR,
	OP_CHARI,
	OP_NOT,
	OP_NOTI,





	OP_STAR,
	OP_MINSTAR,
	OP_PLUS,
	OP_MINPLUS,
	OP_QUERY,
	OP_MINQUERY,

	OP_UPTO,
	OP_MINUPTO,
	OP_EXACT,

	OP_POSSTAR,
	OP_POSPLUS,
	OP_POSQUERY,
	OP_POSUPTO,



	OP_STARI,
	OP_MINSTARI,
	OP_PLUSI,
	OP_MINPLUSI,
	OP_QUERYI,
	OP_MINQUERYI,

	OP_UPTOI,
	OP_MINUPTOI,
	OP_EXACTI,

	OP_POSSTARI,
	OP_POSPLUSI,
	OP_POSQUERYI,
	OP_POSUPTOI,




	OP_NOTSTAR,
	OP_NOTMINSTAR,
	OP_NOTPLUS,
	OP_NOTMINPLUS,
	OP_NOTQUERY,
	OP_NOTMINQUERY,

	OP_NOTUPTO,
	OP_NOTMINUPTO,
	OP_NOTEXACT,

	OP_NOTPOSSTAR,
	OP_NOTPOSPLUS,
	OP_NOTPOSQUERY,
	OP_NOTPOSUPTO,



	OP_NOTSTARI,
	OP_NOTMINSTARI,
	OP_NOTPLUSI,
	OP_NOTMINPLUSI,
	OP_NOTQUERYI,
	OP_NOTMINQUERYI,

	OP_NOTUPTOI,
	OP_NOTMINUPTOI,
	OP_NOTEXACTI,

	OP_NOTPOSSTARI,
	OP_NOTPOSPLUSI,
	OP_NOTPOSQUERYI,
	OP_NOTPOSUPTOI,



	OP_TYPESTAR,
	OP_TYPEMINSTAR,
	OP_TYPEPLUS,
	OP_TYPEMINPLUS,
	OP_TYPEQUERY,
	OP_TYPEMINQUERY,

	OP_TYPEUPTO,
	OP_TYPEMINUPTO,
	OP_TYPEEXACT,

	OP_TYPEPOSSTAR,
	OP_TYPEPOSPLUS,
	OP_TYPEPOSQUERY,
	OP_TYPEPOSUPTO,



	OP_CRSTAR,
	OP_CRMINSTAR,
	OP_CRPLUS,
	OP_CRMINPLUS,
	OP_CRQUERY,
	OP_CRMINQUERY,

	OP_CRRANGE,
	OP_CRMINRANGE,

	OP_CRPOSSTAR,
	OP_CRPOSPLUS,
	OP_CRPOSQUERY,
	OP_CRPOSRANGE,



	OP_CLASS,
	OP_NCLASS,
	OP_XCLASS,
	OP_REF,
	OP_REFI,
	OP_DNREF,
	OP_DNREFI,
	OP_RECURSE,
	OP_CALLOUT,

	OP_ALT,
	OP_KET,
	OP_KETRMAX,
	OP_KETRMIN,
	OP_KETRPOS,



	OP_REVERSE,
	OP_ASSERT,
	OP_ASSERT_NOT,
	OP_ASSERTBACK,
	OP_ASSERTBACK_NOT,



	OP_ONCE,
	OP_ONCE_NC,
	OP_BRA,
	OP_BRAPOS,
	OP_CBRA,
	OP_CBRAPOS,
	OP_COND,



	OP_SBRA,
	OP_SBRAPOS,
	OP_SCBRA,
	OP_SCBRAPOS,
	OP_SCOND,



	OP_CREF,
	OP_DNCREF,
	OP_RREF,
	OP_DNRREF,
	OP_DEF,

	OP_BRAZERO,
	OP_BRAMINZERO,
	OP_BRAPOSZERO,



	OP_MARK,
	OP_PRUNE,
	OP_PRUNE_ARG,
	OP_SKIP,
	OP_SKIP_ARG,
	OP_THEN,
	OP_THEN_ARG,
	OP_COMMIT,



	OP_FAIL,
	OP_ACCEPT,
	OP_ASSERT_ACCEPT,
	OP_CLOSE,



	OP_SKIPZERO,



	OP_TABLE_LENGTH
};






#define OP_NAME_LIST \
  "End", "\\A", "\\G", "\\K", "\\B", "\\b", "\\D", "\\d",         \
  "\\S", "\\s", "\\W", "\\w", "Any", "AllAny", "Anybyte",         \
  "notprop", "prop", "\\R", "\\H", "\\h", "\\V", "\\v",           \
  "extuni",  "\\Z", "\\z",                                        \
  "$", "$", "^", "^", "char", "chari", "not", "noti",             \
  "*", "*?", "+", "+?", "?", "??",                                \
  "{", "{", "{",                                                  \
  "*+","++", "?+", "{",                                           \
  "*", "*?", "+", "+?", "?", "??",                                \
  "{", "{", "{",                                                  \
  "*+","++", "?+", "{",                                           \
  "*", "*?", "+", "+?", "?", "??",                                \
  "{", "{", "{",                                                  \
  "*+","++", "?+", "{",                                           \
  "*", "*?", "+", "+?", "?", "??",                                \
  "{", "{", "{",                                                  \
  "*+","++", "?+", "{",                                           \
  "*", "*?", "+", "+?", "?", "??", "{", "{", "{",                 \
  "*+","++", "?+", "{",                                           \
  "*", "*?", "+", "+?", "?", "??", "{", "{",                      \
  "*+","++", "?+", "{",                                           \
  "class", "nclass", "xclass", "Ref", "Refi", "DnRef", "DnRefi",  \
  "Recurse", "Callout",                                           \
  "Alt", "Ket", "KetRmax", "KetRmin", "KetRpos",                  \
  "Reverse", "Assert", "Assert not", "AssertB", "AssertB not",    \
  "Once", "Once_NC",                                              \
  "Bra", "BraPos", "CBra", "CBraPos",                             \
  "Cond",                                                         \
  "SBra", "SBraPos", "SCBra", "SCBraPos",                         \
  "SCond",                                                        \
  "Cond ref", "Cond dnref", "Cond rec", "Cond dnrec", "Cond def", \
  "Brazero", "Braminzero", "Braposzero",                          \
  "*MARK", "*PRUNE", "*PRUNE", "*SKIP", "*SKIP",                  \
  "*THEN", "*THEN", "*COMMIT", "*FAIL",                           \
  "*ACCEPT", "*ASSERT_ACCEPT",                                    \
  "Close", "Skip zero"




#define OP_LENGTHS \
  1,                              \
  1, 1, 1, 1, 1,                  \
  1, 1, 1, 1, 1, 1,               \
  1, 1, 1,                        \
  3, 3,                           \
  1, 1, 1, 1, 1,                  \
  1,                              \
  1, 1, 1, 1, 1, 1,               \
  2,                              \
  2,                              \
  2,                              \
  2,                              \
   \
  2, 2, 2, 2, 2, 2,               \
  2+IMM2_SIZE, 2+IMM2_SIZE,       \
  2+IMM2_SIZE,                    \
  2, 2, 2, 2+IMM2_SIZE,           \
  2, 2, 2, 2, 2, 2,               \
  2+IMM2_SIZE, 2+IMM2_SIZE,       \
  2+IMM2_SIZE,                    \
  2, 2, 2, 2+IMM2_SIZE,           \
   \
  2, 2, 2, 2, 2, 2,               \
  2+IMM2_SIZE, 2+IMM2_SIZE,       \
  2+IMM2_SIZE,                    \
  2, 2, 2, 2+IMM2_SIZE,           \
  2, 2, 2, 2, 2, 2,               \
  2+IMM2_SIZE, 2+IMM2_SIZE,       \
  2+IMM2_SIZE,                    \
  2, 2, 2, 2+IMM2_SIZE,           \
   \
  2, 2, 2, 2, 2, 2,               \
  2+IMM2_SIZE, 2+IMM2_SIZE,       \
  2+IMM2_SIZE,                    \
  2, 2, 2, 2+IMM2_SIZE,           \
   \
  1, 1, 1, 1, 1, 1,               \
  1+2*IMM2_SIZE, 1+2*IMM2_SIZE,   \
  1, 1, 1, 1+2*IMM2_SIZE,         \
  1+(32/sizeof(ssh_ws)),      \
  1+(32/sizeof(ssh_ws)),      \
  0,                              \
  1+IMM2_SIZE,                    \
  1+IMM2_SIZE,                    \
  1+2*IMM2_SIZE,                  \
  1+2*IMM2_SIZE,                  \
  1+LINK_SIZE,                    \
  2+2*LINK_SIZE,                  \
  1+LINK_SIZE,                    \
  1+LINK_SIZE,                    \
  1+LINK_SIZE,                    \
  1+LINK_SIZE,                    \
  1+LINK_SIZE,                    \
  1+LINK_SIZE,                    \
  1+LINK_SIZE,                    \
  1+LINK_SIZE,                    \
  1+LINK_SIZE,                    \
  1+LINK_SIZE,                    \
  1+LINK_SIZE,                    \
  1+LINK_SIZE,                    \
  1+LINK_SIZE,                    \
  1+LINK_SIZE,                    \
  1+LINK_SIZE+IMM2_SIZE,          \
  1+LINK_SIZE+IMM2_SIZE,          \
  1+LINK_SIZE,                    \
  1+LINK_SIZE,                    \
  1+LINK_SIZE,                    \
  1+LINK_SIZE+IMM2_SIZE,          \
  1+LINK_SIZE+IMM2_SIZE,          \
  1+LINK_SIZE,                    \
  1+IMM2_SIZE, 1+2*IMM2_SIZE,     \
  1+IMM2_SIZE, 1+2*IMM2_SIZE,     \
  1,                              \
  1, 1, 1,                        \
  3, 1, 3,                        \
  1, 3,                           \
  1, 3,                           \
  1, 1, 1, 1,                     \
  1+IMM2_SIZE, 1                 



#define RREF_ANY  0xffff



enum
{
	ERR0, ERR1, ERR2, ERR3, ERR4, ERR5, ERR6, ERR7, ERR8, ERR9,
	ERR10, ERR11, ERR12, ERR13, ERR14, ERR15, ERR16, ERR17, ERR18, ERR19,
	ERR20, ERR21, ERR22, ERR23, ERR24, ERR25, ERR26, ERR27, ERR28, ERR29,
	ERR30, ERR31, ERR32, ERR33, ERR34, ERR35, ERR36, ERR37, ERR38, ERR39,
	ERR40, ERR41, ERR42, ERR43, ERR44, ERR45, ERR46, ERR47, ERR48, ERR49,
	ERR50, ERR51, ERR52, ERR53, ERR54, ERR55, ERR56, ERR57, ERR58, ERR59,
	ERR60, ERR61, ERR62, ERR63, ERR64, ERR65, ERR66, ERR67, ERR68, ERR69,
	ERR70, ERR71, ERR72, ERR73, ERR74, ERR75, ERR76, ERR77, ERR78, ERR79,
	ERR80, ERR81, ERR82, ERR83, ERR84, ERR85, ERR86, ERRCOUNT
};



enum
{
	JIT_COMPILE, JIT_PARTIAL_SOFT_COMPILE, JIT_PARTIAL_HARD_COMPILE,
	JIT_NUMBER_OF_COMPILE_MODES
};



struct real_regex16
{
	ssh_u magic_number;
	ssh_u size;
	ssh_u options;
	ssh_u flags;
	ssh_u limit_match;
	ssh_u limit_recursion;
	ssh_ws first_char;
	ssh_ws req_char;
	ssh_ws max_lookbehind;
	ssh_ws top_bracket;
	ssh_ws top_backref;
	ssh_ws name_table_offset;
	ssh_ws name_entry_size;
	ssh_ws name_count;
	ssh_ws ref_count;
	ssh_ws dummy1;
	ssh_ws dummy2;
	ssh_ws dummy3;
	const ssh_b *tables;
	void             *nullpad;
};

struct real_pcre32
{
	ssh_u magic_number;
	ssh_u size;
	ssh_u options;
	ssh_u flags;
	ssh_u limit_match;
	ssh_u limit_recursion;
	ssh_u first_char;
	ssh_u req_char;
	ssh_ws max_lookbehind;
	ssh_ws top_bracket;
	ssh_ws top_backref;
	ssh_ws name_table_offset;
	ssh_ws name_entry_size;
	ssh_ws name_count;
	ssh_ws ref_count;
	ssh_ws dummy;
	const ssh_b *tables;
	void             *nullpad;
};

#define REAL_PCRE real_regex16

typedef ssh_l __assert_real_regex_size_divisible_8[(sizeof(REAL_PCRE) % 8) == 0 ? 1 : -1];


#define REAL_REGEX_MAGIC(re)     (((REAL_PCRE*)re)->magic_number)
#define REAL_REGEX_SIZE(re)      (((REAL_PCRE*)re)->size)
#define REAL_REGEX_OPTIONS(re)   (((REAL_PCRE*)re)->options)
#define REAL_REGEX_FLAGS(re)     (((REAL_PCRE*)re)->flags)

typedef struct regex_study_data
{
	ssh_u size;
	ssh_u flags;
	ssh_b start_bits[32];
	ssh_u minlength;
} regex_study_data;

typedef struct open_capitem
{
	struct open_capitem *next;
	ssh_ws number;
	ssh_ws flag;
} open_capitem;

typedef struct named_group
{
	ssh_wcs name;
	ssh_l     length;
	ssh_u        number;
} named_group;

typedef struct compile_data
{
	const ssh_b *lcc;
	const ssh_b *fcc;
	const ssh_b *cbits;
	const ssh_b *ctypes;
	ssh_wcs start_workspace;
	ssh_wcs start_code;
	ssh_wcs start_pattern;
	ssh_wcs end_pattern;
	ssh_ws* hwm;
	open_capitem *open_caps;
	named_group *named_groups;
	ssh_ws* name_table;
	ssh_l  names_found;
	ssh_l  name_entry_size;
	ssh_l  named_group_list_size;
	ssh_l  workspace_size;
	ssh_u bracount;
	ssh_l  final_bracount;
	ssh_l  max_lookbehind;
	ssh_l  top_backref;
	ssh_u backref_map;
	ssh_u namedrefcount;
	ssh_l  parens_depth;
	ssh_l  assert_depth;
	ssh_u external_options;
	ssh_u external_flags;
	ssh_l  req_varyopt;
	BOOL had_accept;
	BOOL had_pruneorskip;
	BOOL check_lookbehind;
	BOOL dupnames;
	BOOL iscondassert;
	ssh_l  nltype;
	ssh_l  nllen;
	ssh_ws nl[4];
} compile_data;

typedef struct branch_chain
{
	struct branch_chain *outer;
	ssh_ws* current_branch;
} branch_chain;

typedef struct recurse_check
{
	struct recurse_check *prev;
	ssh_wcs group;
} recurse_check;

typedef struct recursion_info
{
	struct recursion_info *prevrec;
	ssh_u group_num;
	ssh_l *offset_save;
	ssh_l saved_max;
	ssh_l saved_capture_last;
	REGEX_PUCHAR subject_position;
} recursion_info;

struct eptrblock
{
	struct eptrblock *epb_prev;
	REGEX_PUCHAR epb_saved_eptr;
};

struct match_data
{
	ssh_u	match_call_count;
	ssh_u	match_limit;
	ssh_u	match_limit_recursion;
	ssh_l*	offset_vector;
	ssh_l	offset_end;
	ssh_l	offset_max;
	ssh_l	nltype;
	ssh_l	nllen;
	ssh_l	name_count;
	ssh_l	name_entry_size;
	ssh_u	skip_arg_count;
	ssh_u	ignore_skip_arg;
	ssh_ws*	name_table;
	ssh_ws	nl[4];
	const ssh_b *lcc;
	const ssh_b *fcc;
	const ssh_b *ctypes;
	BOOL	notbol;
	BOOL	noteol;
	BOOL	utf;
	BOOL	jscript_compat;
	BOOL	use_ucp;
	BOOL	endonly;
	BOOL	notempty;
	BOOL	notempty_atstart;
	BOOL	hitend;
	BOOL	bsr_anycrlf;
	BOOL	hasthen;
	ssh_wcs	start_code;
	ssh_wcs start_subject;
	ssh_wcs end_subject;
	ssh_wcs start_match_ptr;
	ssh_wcs end_match_ptr;
	ssh_wcs start_used_ptr;
	ssh_l	partial;
	ssh_l	end_offset_top;
	ssh_l	capture_last;
	ssh_l	start_offset;
	ssh_l	match_function_type;
	eptrblock *eptrchain;
	ssh_l	eptrn;
	recursion_info *recursive;
	void	*callout_data;
	ssh_wcs mark;
	ssh_wcs nomatch_mark;
	ssh_wcs once_target;
};

#define ctype_space   0x01
#define ctype_letter  0x02
#define ctype_digit   0x04
#define ctype_xdigit  0x08
#define ctype_word    0x10   
#define ctype_meta    0x80   

#define cbit_space     0      
#define cbit_xdigit   32      
#define cbit_digit    64      
#define cbit_upper    96      
#define cbit_lower   128      
#define cbit_word    160      
#define cbit_graph   192      
#define cbit_print   224      
#define cbit_punct   256      
#define cbit_cntrl   288      
#define cbit_length  320      

#define lcc_offset      0
#define fcc_offset    256
#define cbits_offset  512
#define ctypes_offset (cbits_offset + cbit_length)
#define tables_length (ctypes_offset + 256)

#ifndef PUBL
#define PUBL(name) regex16_##name
#endif

#ifndef PRIV
#define PRIV(name) _regex16_##name
#endif

extern const ssh_b		PRIV(OP_lengths)[];
extern const ssh_b		PRIV(default_tables)[];
extern const ssh_u		PRIV(hspace_list)[];
extern const ssh_u		PRIV(vspace_list)[];
extern ssh_l			PRIV(strncmp_uc_c8)(ssh_wcs, ssh_ccs, ssh_u num);

#define STRCMP_UC_UC(str1, str2) wcscmp(str1, str2)
#define STRNCMP_UC_UC(str1, str2, num) wcsncmp(str1, str2, num)
#define STRNCMP_UC_C8(str1, str2, num) PRIV(strncmp_uc_c8)((str1), (str2), (num))
#define STRCMP_UC_UC_TEST(str1, str2) STRCMP_UC_UC(str1, str2)

extern ssh_wcs			PRIV(find_bracket)(ssh_wcs, BOOL, ssh_l);
extern BOOL				PRIV(is_newline)(ssh_wcs, ssh_l, ssh_wcs, ssh_l*, BOOL);
extern ssh_u		PRIV(ord2utf)(ssh_u, ssh_ws*);
extern ssh_l				PRIV(valid_utf)(ssh_wcs, ssh_l, ssh_l*);
extern BOOL				PRIV(was_newline)(ssh_wcs, ssh_l, ssh_wcs, ssh_l*, BOOL);
extern BOOL				PRIV(xclass)(ssh_u, ssh_wcs, BOOL);

struct ucd_record
{
	ssh_b script;
	ssh_b chartype;
	ssh_b gbprop;
	ssh_b caseset;
	ssh_l other_case;
};

extern const ssh_u PRIV(ucd_caseless_sets)[];
extern const ucd_record  PRIV(ucd_records)[];
extern const ssh_b  PRIV(ucd_stage1)[];
extern const ssh_ws PRIV(ucd_stage2)[];
extern const ssh_u PRIV(ucp_gentype)[];
extern const ssh_u PRIV(ucp_gbtable)[];
