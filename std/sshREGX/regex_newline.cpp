
#include "stdafx.h"

BOOL PRIV(is_newline)(REGEX_PUCHAR ptr, ssh_l type, REGEX_PUCHAR endptr, ssh_l *lenptr, BOOL utf)
{
	ssh_u c;
	c = *ptr;
	if(type == NLTYPE_ANYCRLF) switch(c)
	{
		case CHAR_LF: *lenptr = 1; return TRUE;
		case CHAR_CR: *lenptr = (ptr < endptr - 1 && ptr[1] == CHAR_LF) ? 2 : 1;
			return TRUE;
		default: return FALSE;
	}
	else switch(c)
	{
		case CHAR_LF:
		case CHAR_VT:
		case CHAR_FF: *lenptr = 1; return TRUE;
		case CHAR_CR:
			*lenptr = (ptr < endptr - 1 && ptr[1] == CHAR_LF) ? 2 : 1;
			return TRUE;
		case CHAR_NEL:
		case 0x2028:
		case 0x2029: *lenptr = 1; return TRUE;
		default: return FALSE;
	}
}

BOOL PRIV(was_newline)(REGEX_PUCHAR ptr, ssh_l type, REGEX_PUCHAR startptr, ssh_l *lenptr, BOOL utf)
{
	ssh_u c;
	ptr--;
	c = *ptr;
	if(type == NLTYPE_ANYCRLF) switch(c)
	{
		case CHAR_LF:
			*lenptr = (ptr > startptr && ptr[-1] == CHAR_CR) ? 2 : 1;
			return TRUE;
		case CHAR_CR: *lenptr = 1; return TRUE;
		default: return FALSE;
	}
	else switch(c)
	{
		case CHAR_LF:
			*lenptr = (ptr > startptr && ptr[-1] == CHAR_CR) ? 2 : 1;
			return TRUE;
		case CHAR_VT:
		case CHAR_FF:
		case CHAR_CR: *lenptr = 1; return TRUE;
		case CHAR_NEL:
		case 0x2028:
		case 0x2029: *lenptr = 1; return TRUE;
		default: return FALSE;
	}
}

ssh_l PRIV(strncmp_uc_c8)(const ssh_ws *str1, const char *str2, ssh_u num)
{
	const ssh_b *ustr2 = (ssh_b *)str2;
	ssh_ws c1;
	ssh_ws c2;

	while(num-- > 0)
	{
		c1 = *str1++;
		c2 = (ssh_ws)*ustr2++;
		if(c1 != c2) return ((c1 > c2) << 1) - 1;
	}

	return 0;
}
BOOL PRIV(xclass)(ssh_u c, const ssh_ws *data, BOOL utf)
{
	ssh_ws t;
	BOOL negated = (*data & XCL_NOT) != 0;
	if(c < 256)
	{
		if((*data & XCL_HASPROP) == 0)
		{
			if((*data & XCL_MAP) == 0) return negated;
			return (((ssh_b *)(data + 1))[c / 8] & (1 << (c & 7))) != 0;
		}
		if((*data & XCL_MAP) != 0 && (((ssh_b *)(data + 1))[c / 8] & (1 << (c & 7))) != 0)
			return !negated;
	}
	if((*data++ & XCL_MAP) != 0) data += 32 / sizeof(ssh_ws);
	while((t = *data++) != XCL_END)
	{
		ssh_u x, y;
		if(t == XCL_SINGLE)
		{
			x = *data++;
			if(c == x) return !negated;
		}
		else if(t == XCL_RANGE)
		{
			x = *data++;
			y = *data++;
			if(c >= x && c <= y) return !negated;
		}
	}

	return negated;
}
