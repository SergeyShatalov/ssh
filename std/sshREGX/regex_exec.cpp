
#include "stdafx.h"

#define NLBLOCK md             
#define PSSTART start_subject  
#define PSEND   end_subject    

#undef min
#undef max

#define CAPLMASK    0x0000ffff    
#define OVFLMASK    0xffff0000    
#define OVFLBIT     0x00010000    

#define MATCH_CONDASSERT     1  
#define MATCH_CBEGROUP       2  
#define MATCH_MATCH        1
#define MATCH_NOMATCH      0
#define MATCH_ACCEPT       (-999)
#define MATCH_KETRPOS      (-998)
#define MATCH_ONCE         (-997)
#define MATCH_COMMIT       (-996)
#define MATCH_PRUNE        (-995)
#define MATCH_SKIP         (-994)
#define MATCH_SKIP_ARG     (-993)
#define MATCH_THEN         (-992)
#define MATCH_BACKTRACK_MAX MATCH_THEN
#define MATCH_BACKTRACK_MIN MATCH_COMMIT
#define REC_STACK_SAVE_MAX 30
static const char rep_min[] = {0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, };
static const char rep_max[] = {0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, };

static ssh_l match_ref(ssh_l offset, ssh_wcs eptr, ssh_l length, match_data *md, BOOL caseless)
{
	ssh_wcs eptr_start = eptr;
	ssh_wcs p = md->start_subject + md->offset_vector[offset];
	if(length < 0) return -1;
	if(caseless)
	{
		while(length-- > 0)
		{
			ssh_u cc, cp;
			if(eptr >= md->end_subject) return -2;
			cc = UCHAR21TEST(eptr);
			cp = UCHAR21TEST(p);
			if(TABLE_GET(cp, md->lcc, cp) != TABLE_GET(cc, md->lcc, cc)) return -1;
			p++;
			eptr++;
		}
	}
	else
	{
		while(length-- > 0)
		{
			if(eptr >= md->end_subject) return -2;
			if(UCHAR21INCTEST(p) != UCHAR21INCTEST(eptr)) return -1;
		}
	}
	return (eptr - eptr_start);
}

enum
{
	RM1 = 1, RM2, RM3, RM4, RM5, RM6, RM7, RM8, RM9, RM10,
	RM11, RM12, RM13, RM14, RM15, RM16, RM17, RM18, RM19, RM20,
	RM21, RM22, RM23, RM24, RM25, RM26, RM27, RM28, RM29, RM30,
	RM31, RM32, RM33, RM34, RM35, RM36, RM37, RM38, RM39, RM40,
	RM41, RM42, RM43, RM44, RM45, RM46, RM47, RM48, RM49, RM50,
	RM51, RM52, RM53, RM54, RM55, RM56, RM57, RM58, RM59, RM60,
	RM61, RM62, RM63, RM64, RM65, RM66, RM67
};

#define REGISTER register
#define RMATCH(ra,rb,rc,rd,re,rw) rrc = match(ra,rb,mstart,rc,rd,re,rdepth+1)
#define RRETURN(ra) return ra

#define CHECK_PARTIAL()  if (md->partial != 0 && eptr >= md->end_subject && eptr > md->start_used_ptr) { md->hitend = TRUE; if (md->partial > 1) RRETURN(REGEX_ERROR_PARTIAL); }
#define SCHECK_PARTIAL() if (md->partial != 0 && eptr > md->start_used_ptr) { md->hitend = TRUE; if (md->partial > 1) RRETURN(REGEX_ERROR_PARTIAL); }

static ssh_l match(ssh_wcs eptr, ssh_wcs ecode, ssh_wcs mstart, ssh_l offset_top, match_data *md, eptrblock *eptrb, ssh_u rdepth)
{
	register ssh_l  rrc;
	register ssh_l  i;
	register ssh_u c;
	register BOOL utf;
	BOOL minimize, possessive;
	BOOL caseless;
	ssh_l condcode;
 
#define fi i
#define fc c

	const ssh_ws *callpat;
	const ssh_ws *data;
	const ssh_ws *next;
	REGEX_PUCHAR       pp;
	const ssh_ws *prev;
	REGEX_PUCHAR       saved_eptr;
	recursion_info new_recursive;
	BOOL cur_is_word;
	BOOL condition;
	BOOL prev_is_word;
	ssh_l codelink;
	ssh_l ctype;
	ssh_l length;
	ssh_l max;
	ssh_l min;
	ssh_u number;
	ssh_l offset;
	ssh_u op;
	ssh_l save_capture_last;
	ssh_l save_offset1, save_offset2, save_offset3;
	ssh_l stacksave[REC_STACK_SAVE_MAX];
	eptrblock newptrb;
	if(ecode == NULL)
	{
		if(rdepth == 0) return match((REGEX_PUCHAR)&rdepth, NULL, NULL, 0, NULL, NULL, 1);
		else
		{
			ssh_l len = (ssh_l)((char *)&rdepth - (char *)eptr);
			return (len > 0) ? -len : len;
		}
	}
#define allow_zero    cur_is_word
#define cbegroup      condition
#define code_offset   codelink
#define condassert    condition
#define matched_once  prev_is_word
#define foc           number
#define save_mark     data
TAIL_RECURSE:
	utf = FALSE;

	if(md->match_call_count++ >= md->match_limit) RRETURN(REGEX_ERROR_MATCHLIMIT);
	if(rdepth >= md->match_limit_recursion) RRETURN(REGEX_ERROR_RECURSIONLIMIT);
	if(md->match_function_type == MATCH_CBEGROUP)
	{
		newptrb.epb_saved_eptr = eptr;
		newptrb.epb_prev = eptrb;
		eptrb = &newptrb;
		md->match_function_type = 0;
	}
	for(;;)
	{
		minimize = possessive = FALSE;
		op = *ecode;

		switch(op)
		{
			case OP_MARK:
				md->nomatch_mark = ecode + 2;
				md->mark = NULL;
				RMATCH(eptr, ecode + PRIV(OP_lengths)[*ecode] + ecode[1], offset_top, md, eptrb, RM55);
				if((rrc == MATCH_MATCH || rrc == MATCH_ACCEPT) && md->mark == NULL) md->mark = ecode + 2;
				else if(rrc == MATCH_SKIP_ARG && STRCMP_UC_UC_TEST(ecode + 2, md->start_match_ptr) == 0)
				{
					md->start_match_ptr = eptr;
					RRETURN(MATCH_SKIP);
				}
				RRETURN(rrc);
			case OP_FAIL:
				RRETURN(MATCH_NOMATCH);
			case OP_COMMIT:
				RMATCH(eptr, ecode + PRIV(OP_lengths)[*ecode], offset_top, md, eptrb, RM52);
				if(rrc != MATCH_NOMATCH) RRETURN(rrc);
				RRETURN(MATCH_COMMIT);
			case OP_PRUNE:
				RMATCH(eptr, ecode + PRIV(OP_lengths)[*ecode], offset_top, md, eptrb, RM51);
				if(rrc != MATCH_NOMATCH) RRETURN(rrc);
				RRETURN(MATCH_PRUNE);
			case OP_PRUNE_ARG:
				md->nomatch_mark = ecode + 2;
				md->mark = NULL;
				RMATCH(eptr, ecode + PRIV(OP_lengths)[*ecode] + ecode[1], offset_top, md, eptrb, RM56);
				if((rrc == MATCH_MATCH || rrc == MATCH_ACCEPT) &&
				   md->mark == NULL) md->mark = ecode + 2;
				if(rrc != MATCH_NOMATCH) RRETURN(rrc);
				RRETURN(MATCH_PRUNE);
			case OP_SKIP:
				RMATCH(eptr, ecode + PRIV(OP_lengths)[*ecode], offset_top, md, eptrb, RM53);
				if(rrc != MATCH_NOMATCH) RRETURN(rrc);
				md->start_match_ptr = eptr;
				RRETURN(MATCH_SKIP);
			case OP_SKIP_ARG:
				md->skip_arg_count++;
				if(md->skip_arg_count <= md->ignore_skip_arg)
				{
					ecode += PRIV(OP_lengths)[*ecode] + ecode[1];
					break;
				}
				RMATCH(eptr, ecode + PRIV(OP_lengths)[*ecode] + ecode[1], offset_top, md, eptrb, RM57);
				if(rrc != MATCH_NOMATCH) RRETURN(rrc);
				md->start_match_ptr = ecode + 2;
				RRETURN(MATCH_SKIP_ARG);
			case OP_THEN:
				RMATCH(eptr, ecode + PRIV(OP_lengths)[*ecode], offset_top, md, eptrb, RM54);
				if(rrc != MATCH_NOMATCH) RRETURN(rrc);
				md->start_match_ptr = ecode;
				RRETURN(MATCH_THEN);
			case OP_THEN_ARG:
				md->nomatch_mark = ecode + 2;
				md->mark = NULL;
				RMATCH(eptr, ecode + PRIV(OP_lengths)[*ecode] + ecode[1], offset_top, md, eptrb, RM58);
				if((rrc == MATCH_MATCH || rrc == MATCH_ACCEPT) && md->mark == NULL) md->mark = ecode + 2;
				if(rrc != MATCH_NOMATCH) RRETURN(rrc);
				md->start_match_ptr = ecode;
				RRETURN(MATCH_THEN);
			case OP_ONCE_NC:
				prev = ecode;
				saved_eptr = eptr;
				save_mark = md->mark;
				do
				{
					RMATCH(eptr, ecode + 1 + LINK_SIZE, offset_top, md, eptrb, RM64);
					if(rrc == MATCH_MATCH)
					{
						mstart = md->start_match_ptr;
						break;
					}
					if(rrc == MATCH_THEN)
					{
						next = ecode + GET(ecode, 1);
						if(md->start_match_ptr < next && (*ecode == OP_ALT || *next == OP_ALT))
						   rrc = MATCH_NOMATCH;
					}
					if(rrc != MATCH_NOMATCH) RRETURN(rrc);
					ecode += GET(ecode, 1);
					md->mark = save_mark;
				} while(*ecode == OP_ALT);
				if(*ecode != OP_ONCE_NC && *ecode != OP_ALT) RRETURN(MATCH_NOMATCH);
				do ecode += GET(ecode, 1); while(*ecode == OP_ALT);
				offset_top = md->end_offset_top;
				eptr = md->end_match_ptr;
				if(*ecode == OP_KET || eptr == saved_eptr)
				{
					ecode += 1 + LINK_SIZE;
					break;
				}
				if(*ecode == OP_KETRMIN)
				{
					RMATCH(eptr, ecode + 1 + LINK_SIZE, offset_top, md, eptrb, RM65);
					if(rrc != MATCH_NOMATCH) RRETURN(rrc);
					ecode = prev;
					goto TAIL_RECURSE;
				}
				else
				{
					RMATCH(eptr, prev, offset_top, md, eptrb, RM66);
					if(rrc != MATCH_NOMATCH) RRETURN(rrc);
					ecode += 1 + LINK_SIZE;
					goto TAIL_RECURSE;
				}
			case OP_CBRA:
			case OP_SCBRA:
				number = GET2(ecode, 1 + LINK_SIZE);
				offset = number << 1;
				if(offset < md->offset_max)
				{
					save_offset1 = md->offset_vector[offset];
					save_offset2 = md->offset_vector[offset + 1];
					save_offset3 = md->offset_vector[md->offset_end - number];
					save_capture_last = md->capture_last;
					save_mark = md->mark;
					md->offset_vector[md->offset_end - number] = (ssh_l)(eptr - md->start_subject);
					for(;;)
					{
						if(op >= OP_SBRA) md->match_function_type = MATCH_CBEGROUP;
						RMATCH(eptr, ecode + PRIV(OP_lengths)[*ecode], offset_top, md, eptrb, RM1);
						if(rrc == MATCH_ONCE) break;
						if(rrc == MATCH_THEN)
						{
							next = ecode + GET(ecode, 1);
							if(md->start_match_ptr < next && (*ecode == OP_ALT || *next == OP_ALT))
							   rrc = MATCH_NOMATCH;
						}
						if(rrc != MATCH_NOMATCH) RRETURN(rrc);
						md->capture_last = save_capture_last;
						ecode += GET(ecode, 1);
						md->mark = save_mark;
						if(*ecode != OP_ALT) break;
					}
					md->offset_vector[offset] = save_offset1;
					md->offset_vector[offset + 1] = save_offset2;
					md->offset_vector[md->offset_end - number] = save_offset3;
					RRETURN(rrc);
				}
			case OP_ONCE:
			case OP_BRA:
			case OP_SBRA:
				for(;;)
				{
					if(op >= OP_SBRA || op == OP_ONCE) md->match_function_type = MATCH_CBEGROUP;
					else if(!md->hasthen && ecode[GET(ecode, 1)] != OP_ALT)
					{
						ecode += PRIV(OP_lengths)[*ecode];
						goto TAIL_RECURSE;
					}
					save_mark = md->mark;
					save_capture_last = md->capture_last;
					RMATCH(eptr, ecode + PRIV(OP_lengths)[*ecode], offset_top, md, eptrb, RM2);
					if(rrc == MATCH_THEN)
					{
						next = ecode + GET(ecode, 1);
						if(md->start_match_ptr < next && (*ecode == OP_ALT || *next == OP_ALT))
						   rrc = MATCH_NOMATCH;
					}
					if(rrc != MATCH_NOMATCH)
					{
						if(rrc == MATCH_ONCE)
						{
							const ssh_ws *scode = ecode;
							if(*scode != OP_ONCE)
							{
								while(*scode == OP_ALT) scode += GET(scode, 1);
								scode -= GET(scode, 1);
							}
							if(md->once_target == scode) rrc = MATCH_NOMATCH;
						}
						RRETURN(rrc);
					}
					ecode += GET(ecode, 1);
					md->mark = save_mark;
					if(*ecode != OP_ALT) break;
					md->capture_last = save_capture_last;
				}

				RRETURN(MATCH_NOMATCH);
			case OP_CBRAPOS:
			case OP_SCBRAPOS:
				allow_zero = FALSE;

POSSESSIVE_CAPTURE:
				number = GET2(ecode, 1 + LINK_SIZE);
				offset = number << 1;
				if(offset >= md->offset_max) goto POSSESSIVE_NON_CAPTURE;
				matched_once = FALSE;
				code_offset = (ssh_l)(ecode - md->start_code);
				save_offset1 = md->offset_vector[offset];
				save_offset2 = md->offset_vector[offset + 1];
				save_offset3 = md->offset_vector[md->offset_end - number];
				save_capture_last = md->capture_last;
				for(;;)
				{
					md->offset_vector[md->offset_end - number] = (ssh_l)(eptr - md->start_subject);
					if(op >= OP_SBRA) md->match_function_type = MATCH_CBEGROUP; RMATCH(eptr, ecode + PRIV(OP_lengths)[*ecode], offset_top, md, eptrb, RM63);
					if(rrc == MATCH_KETRPOS)
					{
						offset_top = md->end_offset_top;
						ecode = md->start_code + code_offset;
						save_capture_last = md->capture_last;
						matched_once = TRUE;
						mstart = md->start_match_ptr;
						if(eptr == md->end_match_ptr)
						{
							do ecode += GET(ecode, 1); while(*ecode == OP_ALT);
							break;
						}
						eptr = md->end_match_ptr;
						continue;
					}
					if(rrc == MATCH_THEN)
					{
						next = ecode + GET(ecode, 1);
						if(md->start_match_ptr < next && (*ecode == OP_ALT || *next == OP_ALT))
						   rrc = MATCH_NOMATCH;
					}
					if(rrc != MATCH_NOMATCH) RRETURN(rrc);
					md->capture_last = save_capture_last;
					ecode += GET(ecode, 1);
					if(*ecode != OP_ALT) break;
				}
				if(!matched_once)
				{
					md->offset_vector[offset] = save_offset1;
					md->offset_vector[offset + 1] = save_offset2;
					md->offset_vector[md->offset_end - number] = save_offset3;
				}
				if(allow_zero || matched_once)
				{
					ecode += 1 + LINK_SIZE;
					break;
				}
				RRETURN(MATCH_NOMATCH);
			case OP_BRAPOS:
			case OP_SBRAPOS:
				allow_zero = FALSE;
POSSESSIVE_NON_CAPTURE:
				matched_once = FALSE;
				code_offset = (ssh_l)(ecode - md->start_code);
				save_capture_last = md->capture_last;
				for(;;)
				{
					if(op >= OP_SBRA) md->match_function_type = MATCH_CBEGROUP;
					RMATCH(eptr, ecode + PRIV(OP_lengths)[*ecode], offset_top, md, eptrb, RM48);
					if(rrc == MATCH_KETRPOS)
					{
						offset_top = md->end_offset_top;
						ecode = md->start_code + code_offset;
						matched_once = TRUE;
						mstart = md->start_match_ptr;
						if(eptr == md->end_match_ptr)
						{
							do ecode += GET(ecode, 1); while(*ecode == OP_ALT);
							break;
						}
						eptr = md->end_match_ptr;
						continue;
					}
					if(rrc == MATCH_THEN)
					{
						next = ecode + GET(ecode, 1);
						if(md->start_match_ptr < next && (*ecode == OP_ALT || *next == OP_ALT))
						   rrc = MATCH_NOMATCH;
					}

					if(rrc != MATCH_NOMATCH) RRETURN(rrc);
					ecode += GET(ecode, 1);
					if(*ecode != OP_ALT) break;
					md->capture_last = save_capture_last;
				}
				if(matched_once || allow_zero)
				{
					ecode += 1 + LINK_SIZE;
					break;
				}
				RRETURN(MATCH_NOMATCH);
			case OP_COND:
			case OP_SCOND:
				codelink = GET(ecode, 1);
				ecode += 1 + LINK_SIZE;
				if(*ecode == OP_CALLOUT)
				{
					ecode += PRIV(OP_lengths)[OP_CALLOUT];
					codelink -= PRIV(OP_lengths)[OP_CALLOUT];
				}
				condition = FALSE;
				switch(condcode = *ecode)
				{
					case OP_RREF:
						if(md->recursive != NULL)
						{
							ssh_u recno = GET2(ecode, 1);
							condition = (recno == RREF_ANY || recno == md->recursive->group_num);
						}
						break;
					case OP_DNRREF:
						if(md->recursive != NULL)
						{
							ssh_l count = GET2(ecode, 1 + IMM2_SIZE);
							ssh_ws *slot = md->name_table + GET2(ecode, 1) * md->name_entry_size;
							while(count-- > 0)
							{
								ssh_u recno = GET2(slot, 0);
								condition = recno == md->recursive->group_num;
								if(condition) break;
								slot += md->name_entry_size;
							}
						}
						break;
					case OP_CREF:
						offset = GET2(ecode, 1) << 1;
						condition = offset < offset_top && md->offset_vector[offset] >= 0;
						break;
					case OP_DNCREF:
					{
						ssh_l count = GET2(ecode, 1 + IMM2_SIZE);
						ssh_ws *slot = md->name_table + GET2(ecode, 1) * md->name_entry_size;
						while(count-- > 0)
						{
							offset = GET2(slot, 0) << 1;
							condition = offset < offset_top && md->offset_vector[offset] >= 0;
							if(condition) break;
							slot += md->name_entry_size;
						}
					}
					break;
					case OP_DEF:
					case OP_FAIL:
						break;
					default:
						md->match_function_type = MATCH_CONDASSERT;
						RMATCH(eptr, ecode, offset_top, md, NULL, RM3);
						if(rrc == MATCH_MATCH)
						{
							if(md->end_offset_top > offset_top) offset_top = md->end_offset_top;
							condition = TRUE;
							if(*ecode == OP_BRAZERO) ecode++;
							ecode += GET(ecode, 1);
							while(*ecode == OP_ALT) ecode += GET(ecode, 1);
							ecode += 1 + LINK_SIZE - PRIV(OP_lengths)[condcode];
						}
						else if(rrc != MATCH_NOMATCH && rrc != MATCH_THEN)
						{
							RRETURN(rrc);
						}
						break;
				}
				ecode += condition ? PRIV(OP_lengths)[condcode] : codelink;
				if(condition || ecode[-(1 + LINK_SIZE)] == OP_ALT)
				{
					if(op != OP_SCOND)
					{
						goto TAIL_RECURSE;
					}

					md->match_function_type = MATCH_CBEGROUP;
					RMATCH(eptr, ecode, offset_top, md, eptrb, RM49);
					RRETURN(rrc);
				}
				break;
			case OP_CLOSE:
				number = GET2(ecode, 1);
				offset = number << 1;
				md->capture_last = (md->capture_last & OVFLMASK) | number;
				if(offset >= md->offset_max) md->capture_last |= OVFLBIT; else
				{
					md->offset_vector[offset] = md->offset_vector[md->offset_end - number];
					md->offset_vector[offset + 1] = (ssh_l)(eptr - md->start_subject);
					if(offset >= offset_top)
					{
						register ssh_l *iptr = md->offset_vector + offset_top;
						register ssh_l *iend = md->offset_vector + offset;
						while(iptr < iend) *iptr++ = -1;
						offset_top = offset + 2;
					}
				}
				ecode += 1 + IMM2_SIZE;
				break;
			case OP_END:
			case OP_ACCEPT:
			case OP_ASSERT_ACCEPT:
				if(eptr == mstart && op != OP_ASSERT_ACCEPT && md->recursive == NULL && (md->notempty || (md->notempty_atstart && mstart == md->start_subject + md->start_offset)))
				   RRETURN(MATCH_NOMATCH);
				md->end_match_ptr = eptr;
				md->end_offset_top = offset_top;
				md->start_match_ptr = mstart;
				rrc = (op == OP_END) ? MATCH_MATCH : MATCH_ACCEPT;
				RRETURN(rrc);
			case OP_ASSERT:
			case OP_ASSERTBACK:
				save_mark = md->mark;
				if(md->match_function_type == MATCH_CONDASSERT)
				{
					condassert = TRUE;
					md->match_function_type = 0;
				}
				else condassert = FALSE;
				do
				{
					RMATCH(eptr, ecode + 1 + LINK_SIZE, offset_top, md, NULL, RM4);



					if(rrc == MATCH_MATCH || rrc == MATCH_ACCEPT)
					{
						mstart = md->start_match_ptr;
						break;
					}
					md->mark = save_mark;
					if(rrc == MATCH_THEN)
					{
						next = ecode + GET(ecode, 1);
						if(md->start_match_ptr < next && (*ecode == OP_ALT || *next == OP_ALT)) rrc = MATCH_NOMATCH;
					}
					if(rrc != MATCH_NOMATCH) RRETURN(rrc);
					ecode += GET(ecode, 1);
				} while(*ecode == OP_ALT);
				if(*ecode == OP_KET) RRETURN(MATCH_NOMATCH);
				if(condassert) RRETURN(MATCH_MATCH);
				do ecode += GET(ecode, 1); while(*ecode == OP_ALT);
				ecode += 1 + LINK_SIZE;
				offset_top = md->end_offset_top;
				continue;
			case OP_ASSERT_NOT:
			case OP_ASSERTBACK_NOT:
				save_mark = md->mark;
				if(md->match_function_type == MATCH_CONDASSERT)
				{
					condassert = TRUE;
					md->match_function_type = 0;
				}
				else condassert = FALSE;
				do
				{
					RMATCH(eptr, ecode + 1 + LINK_SIZE, offset_top, md, NULL, RM5);
					md->mark = save_mark;

					switch(rrc)
					{
						case MATCH_MATCH:
						case MATCH_ACCEPT:
							RRETURN(MATCH_NOMATCH);

						case MATCH_NOMATCH:
							break;
						case MATCH_THEN:
							next = ecode + GET(ecode, 1);
							if(md->start_match_ptr < next && (*ecode == OP_ALT || *next == OP_ALT))
							{
								rrc = MATCH_NOMATCH;
								break;
							}
						case MATCH_COMMIT:
						case MATCH_SKIP:
						case MATCH_SKIP_ARG:
						case MATCH_PRUNE:
							do ecode += GET(ecode, 1); while(*ecode == OP_ALT);
							goto NEG_ASSERT_TRUE;
						default:
							RRETURN(rrc);
					}
					ecode += GET(ecode, 1);
				} while(*ecode == OP_ALT);
NEG_ASSERT_TRUE:
				if(condassert) RRETURN(MATCH_MATCH);
				ecode += 1 + LINK_SIZE;
				continue;
			case OP_REVERSE:
				eptr -= GET(ecode, 1);
				if(eptr < md->start_subject) RRETURN(MATCH_NOMATCH);
				if(eptr < md->start_used_ptr) md->start_used_ptr = eptr;
				ecode += 1 + LINK_SIZE;
				break;
			case OP_CALLOUT:
				ecode += 2 + 2 * LINK_SIZE;
				break;
			case OP_RECURSE:
			{
				recursion_info *ri;
				ssh_u recno;
				callpat = md->start_code + GET(ecode, 1);
				recno = (callpat == md->start_code) ? 0 : GET2(callpat, 1 + LINK_SIZE);
				for(ri = md->recursive; ri != NULL; ri = ri->prevrec)
					if(recno == ri->group_num && eptr == ri->subject_position) RRETURN(REGEX_ERROR_RECURSELOOP);
				new_recursive.group_num = recno;
				new_recursive.saved_capture_last = md->capture_last;
				new_recursive.subject_position = eptr;
				new_recursive.prevrec = md->recursive;
				md->recursive = &new_recursive;
				ecode += 1 + LINK_SIZE;
				new_recursive.saved_max = md->offset_end;
				if(new_recursive.saved_max <= REC_STACK_SAVE_MAX) new_recursive.offset_save = stacksave;
				else
				{
					new_recursive.offset_save = (ssh_l *)malloc(new_recursive.saved_max * sizeof(ssh_l));
					if(new_recursive.offset_save == NULL) RRETURN(REGEX_ERROR_NOMEMORY);
				}
				memcpy(new_recursive.offset_save, md->offset_vector, new_recursive.saved_max * sizeof(ssh_l));
				cbegroup = (*callpat >= OP_SBRA);
				do
				{
					if(cbegroup) md->match_function_type = MATCH_CBEGROUP;
					RMATCH(eptr, callpat + PRIV(OP_lengths)[*callpat], offset_top, md, eptrb, RM6);
					memcpy(md->offset_vector, new_recursive.offset_save, new_recursive.saved_max * sizeof(ssh_l));
					md->capture_last = new_recursive.saved_capture_last;
					md->recursive = new_recursive.prevrec;
					if(rrc == MATCH_MATCH || rrc == MATCH_ACCEPT)
					{
						if(new_recursive.offset_save != stacksave) free(new_recursive.offset_save);
						eptr = md->end_match_ptr;
						mstart = md->start_match_ptr;
						goto RECURSION_MATCHED;
					}
					if(rrc >= MATCH_BACKTRACK_MIN && rrc <= MATCH_BACKTRACK_MAX)
					{
						if(new_recursive.offset_save != stacksave) free(new_recursive.offset_save);
						RRETURN(MATCH_NOMATCH);
					}
					if(rrc != MATCH_NOMATCH)
					{
						if(new_recursive.offset_save != stacksave) free(new_recursive.offset_save);
						RRETURN(rrc);
					}
					md->recursive = &new_recursive;
					callpat += GET(callpat, 1);
				} while(*callpat == OP_ALT);
				md->recursive = new_recursive.prevrec;
				if(new_recursive.offset_save != stacksave) free(new_recursive.offset_save);
				RRETURN(MATCH_NOMATCH);
			}
RECURSION_MATCHED:
			break;
			case OP_ALT:
				do ecode += GET(ecode, 1); while(*ecode == OP_ALT);
				break;
			case OP_BRAZERO:
				next = ecode + 1;
				RMATCH(eptr, next, offset_top, md, eptrb, RM10);
				if(rrc != MATCH_NOMATCH) RRETURN(rrc);
				do next += GET(next, 1); while(*next == OP_ALT);
				ecode = next + 1 + LINK_SIZE;
				break;

			case OP_BRAMINZERO:
				next = ecode + 1;
				do next += GET(next, 1); while(*next == OP_ALT);
				RMATCH(eptr, next + 1 + LINK_SIZE, offset_top, md, eptrb, RM11);
				if(rrc != MATCH_NOMATCH) RRETURN(rrc);
				ecode++;
				break;

			case OP_SKIPZERO:
				next = ecode + 1;
				do next += GET(next, 1); while(*next == OP_ALT);
				ecode = next + 1 + LINK_SIZE;
				break;
			case OP_BRAPOSZERO:
				op = *(++ecode);
				allow_zero = TRUE;
				if(op == OP_CBRAPOS || op == OP_SCBRAPOS) goto POSSESSIVE_CAPTURE;
				goto POSSESSIVE_NON_CAPTURE;
			case OP_KET:
			case OP_KETRMIN:
			case OP_KETRMAX:
			case OP_KETRPOS:
				prev = ecode - GET(ecode, 1);
				if(*prev >= OP_SBRA || *prev == OP_ONCE)
				{
					saved_eptr = eptrb->epb_saved_eptr;
					eptrb = eptrb->epb_prev;
				}
				else saved_eptr = NULL;
				if((*prev >= OP_ASSERT && *prev <= OP_ASSERTBACK_NOT) || *prev == OP_ONCE_NC)
				{
					md->end_match_ptr = eptr;
					md->end_offset_top = offset_top;
					md->start_match_ptr = mstart;
					RRETURN(MATCH_MATCH);
				}
				if(*prev == OP_CBRA || *prev == OP_SCBRA || *prev == OP_CBRAPOS || *prev == OP_SCBRAPOS)
				{
					number = GET2(prev, 1 + LINK_SIZE);
					offset = number << 1;
					if(md->recursive != NULL && md->recursive->group_num == number)
					{
						md->end_match_ptr = eptr;
						md->start_match_ptr = mstart;
						RRETURN(MATCH_MATCH);
					}
					md->capture_last = (md->capture_last & OVFLMASK) | number;
					if(offset >= md->offset_max) md->capture_last |= OVFLBIT; else
					{
						if(offset > offset_top)
						{
							register ssh_l *iptr = md->offset_vector + offset_top;
							register ssh_l *iend = md->offset_vector + offset;
							while(iptr < iend) *iptr++ = -1;
						}
						md->offset_vector[offset] = md->offset_vector[md->offset_end - number];
						md->offset_vector[offset + 1] = (ssh_l)(eptr - md->start_subject);
						if(offset_top <= offset) offset_top = offset + 2;
					}
				}
				if(*ecode == OP_KETRPOS)
				{
					md->start_match_ptr = mstart;
					md->end_match_ptr = eptr;
					md->end_offset_top = offset_top;
					RRETURN(MATCH_KETRPOS);
				}
				if(*ecode == OP_KET || eptr == saved_eptr)
				{
					if(*prev == OP_ONCE)
					{
						RMATCH(eptr, ecode + 1 + LINK_SIZE, offset_top, md, eptrb, RM12);
						if(rrc != MATCH_NOMATCH) RRETURN(rrc);
						md->once_target = prev;
						RRETURN(MATCH_ONCE);
					}
					ecode += 1 + LINK_SIZE;
					break;
				}
				if(*ecode == OP_KETRMIN)
				{
					RMATCH(eptr, ecode + 1 + LINK_SIZE, offset_top, md, eptrb, RM7);
					if(rrc != MATCH_NOMATCH) RRETURN(rrc);
					if(*prev == OP_ONCE)
					{
						RMATCH(eptr, prev, offset_top, md, eptrb, RM8);
						if(rrc != MATCH_NOMATCH) RRETURN(rrc);
						md->once_target = prev;
						RRETURN(MATCH_ONCE);
					}
					if(*prev >= OP_SBRA)
					{
						RMATCH(eptr, prev, offset_top, md, eptrb, RM50);
						RRETURN(rrc);
					}
					ecode = prev;
					goto TAIL_RECURSE;
				}
				else
				{
					RMATCH(eptr, prev, offset_top, md, eptrb, RM13);
					if(rrc == MATCH_ONCE && md->once_target == prev) rrc = MATCH_NOMATCH;
					if(rrc != MATCH_NOMATCH) RRETURN(rrc);
					if(*prev == OP_ONCE)
					{
						RMATCH(eptr, ecode + 1 + LINK_SIZE, offset_top, md, eptrb, RM9);
						if(rrc != MATCH_NOMATCH) RRETURN(rrc);
						md->once_target = prev;
						RRETURN(MATCH_ONCE);
					}
					ecode += 1 + LINK_SIZE;
					goto TAIL_RECURSE;
				}
			case OP_CIRC:
				if(md->notbol && eptr == md->start_subject) RRETURN(MATCH_NOMATCH);
			case OP_SOD:
				if(eptr != md->start_subject) RRETURN(MATCH_NOMATCH);
				ecode++;
				break;
			case OP_CIRCM:
				if(md->notbol && eptr == md->start_subject) RRETURN(MATCH_NOMATCH);
				if(eptr != md->start_subject && (eptr == md->end_subject || !WAS_NEWLINE(eptr)))
				   RRETURN(MATCH_NOMATCH);
				ecode++;
				break;
			case OP_SOM:
				if(eptr != md->start_subject + md->start_offset) RRETURN(MATCH_NOMATCH);
				ecode++;
				break;
			case OP_SET_SOM:
				mstart = eptr;
				ecode++;
				break;
			case OP_DOLLM:
				if(eptr < md->end_subject)
				{
					if(!IS_NEWLINE(eptr))
					{
						if(md->partial != 0 && eptr + 1 >= md->end_subject && NLBLOCK->nltype == NLTYPE_FIXED && NLBLOCK->nllen == 2 && UCHAR21TEST(eptr) == NLBLOCK->nl[0])
						{
							md->hitend = TRUE;
							if(md->partial > 1) RRETURN(REGEX_ERROR_PARTIAL);
						}
						RRETURN(MATCH_NOMATCH);
					}
				}
				else
				{
					if(md->noteol) RRETURN(MATCH_NOMATCH);
					SCHECK_PARTIAL();
				}
				ecode++;
				break;



			case OP_DOLL:
				if(md->noteol) RRETURN(MATCH_NOMATCH);
				if(!md->endonly) goto ASSERT_NL_OR_EOS;





			case OP_EOD:
				if(eptr < md->end_subject) RRETURN(MATCH_NOMATCH);
				SCHECK_PARTIAL();
				ecode++;
				break;



			case OP_EODN:
ASSERT_NL_OR_EOS :
				if(eptr < md->end_subject && (!IS_NEWLINE(eptr) || eptr != md->end_subject - md->nllen))
				{
					if(md->partial != 0 && eptr + 1 >= md->end_subject && NLBLOCK->nltype == NLTYPE_FIXED && NLBLOCK->nllen == 2 && UCHAR21TEST(eptr) == NLBLOCK->nl[0])
				{
				md->hitend = TRUE;
				if(md->partial > 1) RRETURN(REGEX_ERROR_PARTIAL);
			}
				RRETURN(MATCH_NOMATCH);
			}
				 SCHECK_PARTIAL();
				 ecode++;
				 break;
			case OP_NOT_WORD_BOUNDARY:
			case OP_WORD_BOUNDARY:
			{
					if(eptr == md->start_subject) prev_is_word = FALSE; else
					{
						if(eptr <= md->start_used_ptr) md->start_used_ptr = eptr - 1;
							prev_is_word = MAX_255(eptr[-1])
							&& ((md->ctypes[eptr[-1]] & ctype_word) != 0);
					}
					if(eptr >= md->end_subject)
					{
						SCHECK_PARTIAL();
						cur_is_word = FALSE;
					}
					else
						cur_is_word = MAX_255(*eptr) && ((md->ctypes[*eptr] & ctype_word) != 0);
				if((*ecode++ == OP_WORD_BOUNDARY) ? cur_is_word == prev_is_word : cur_is_word != prev_is_word)
				   RRETURN(MATCH_NOMATCH);
			}
			break;
			case OP_ANY:
				if(IS_NEWLINE(eptr)) RRETURN(MATCH_NOMATCH);
				if(md->partial != 0 && eptr + 1 >= md->end_subject && NLBLOCK->nltype == NLTYPE_FIXED && NLBLOCK->nllen == 2 && UCHAR21TEST(eptr) == NLBLOCK->nl[0])
				{
					md->hitend = TRUE;
					if(md->partial > 1) RRETURN(REGEX_ERROR_PARTIAL);
				}
			case OP_ALLANY:
				if(eptr >= md->end_subject)
				{
					SCHECK_PARTIAL();
					RRETURN(MATCH_NOMATCH);
				}
				eptr++;
				ecode++;
				break;
			case OP_ANYBYTE:
				if(eptr >= md->end_subject)
				{
					SCHECK_PARTIAL();
					RRETURN(MATCH_NOMATCH);
				}
				eptr++;
				ecode++;
				break;

			case OP_NOT_DIGIT:
				if(eptr >= md->end_subject)
				{
					SCHECK_PARTIAL();
					RRETURN(MATCH_NOMATCH);
				}
				GETCHARINCTEST(c, eptr);
				if(c < 256 && (md->ctypes[c] & ctype_digit) != 0)
					RRETURN(MATCH_NOMATCH);
				ecode++;
				break;

			case OP_DIGIT:
				if(eptr >= md->end_subject)
				{
					SCHECK_PARTIAL();
					RRETURN(MATCH_NOMATCH);
				}
				GETCHARINCTEST(c, eptr);
				if(c > 255 || (md->ctypes[c] & ctype_digit) == 0)
					RRETURN(MATCH_NOMATCH);
				ecode++;
				break;

			case OP_NOT_WHITESPACE:
				if(eptr >= md->end_subject)
				{
					SCHECK_PARTIAL();
					RRETURN(MATCH_NOMATCH);
				}
				GETCHARINCTEST(c, eptr);
				if(c < 256 && (md->ctypes[c] & ctype_space) != 0)
					RRETURN(MATCH_NOMATCH);
				ecode++;
				break;

			case OP_WHITESPACE:
				if(eptr >= md->end_subject)
				{
					SCHECK_PARTIAL();
					RRETURN(MATCH_NOMATCH);
				}
				GETCHARINCTEST(c, eptr);
				if(c > 255 || (md->ctypes[c] & ctype_space) == 0)
					RRETURN(MATCH_NOMATCH);
				ecode++;
				break;

			case OP_NOT_WORDCHAR:
				if(eptr >= md->end_subject)
				{
					SCHECK_PARTIAL();
					RRETURN(MATCH_NOMATCH);
				}
				GETCHARINCTEST(c, eptr);
				if(c < 256 && (md->ctypes[c] & ctype_word) != 0)
					RRETURN(MATCH_NOMATCH);
				ecode++;
				break;

			case OP_WORDCHAR:
				if(eptr >= md->end_subject)
				{
					SCHECK_PARTIAL();
					RRETURN(MATCH_NOMATCH);
				}
				GETCHARINCTEST(c, eptr);
				if(c > 255 || (md->ctypes[c] & ctype_word) == 0)
					RRETURN(MATCH_NOMATCH);
				ecode++;
				break;

			case OP_ANYNL:
				if(eptr >= md->end_subject)
				{
					SCHECK_PARTIAL();
					RRETURN(MATCH_NOMATCH);
				}
				GETCHARINCTEST(c, eptr);
				switch(c)
				{
					default: RRETURN(MATCH_NOMATCH);

					case CHAR_CR:
						if(eptr >= md->end_subject)
						{
							SCHECK_PARTIAL();
						}
						else if(UCHAR21TEST(eptr) == CHAR_LF) eptr++;
						break;

					case CHAR_LF:
						break;

					case CHAR_VT:
					case CHAR_FF:
					case CHAR_NEL:
					case 0x2028:
					case 0x2029:
						if(md->bsr_anycrlf) RRETURN(MATCH_NOMATCH);
						break;
				}
				ecode++;
				break;

			case OP_NOT_HSPACE:
				if(eptr >= md->end_subject)
				{
					SCHECK_PARTIAL();
					RRETURN(MATCH_NOMATCH);
				}
				GETCHARINCTEST(c, eptr);
				switch(c)
				{
HSPACE_CASES: RRETURN(MATCH_NOMATCH);
					default: break;
				}
				ecode++;
				break;

			case OP_HSPACE:
				if(eptr >= md->end_subject)
				{
					SCHECK_PARTIAL();
					RRETURN(MATCH_NOMATCH);
				}
				GETCHARINCTEST(c, eptr);
				switch(c)
				{
HSPACE_CASES: break;
					default: RRETURN(MATCH_NOMATCH);
				}
				ecode++;
				break;

			case OP_NOT_VSPACE:
				if(eptr >= md->end_subject)
				{
					SCHECK_PARTIAL();
					RRETURN(MATCH_NOMATCH);
				}
				GETCHARINCTEST(c, eptr);
				switch(c)
				{
VSPACE_CASES: RRETURN(MATCH_NOMATCH);
					default: break;
				}
				ecode++;
				break;

			case OP_VSPACE:
				if(eptr >= md->end_subject)
				{
					SCHECK_PARTIAL();
					RRETURN(MATCH_NOMATCH);
				}
				GETCHARINCTEST(c, eptr);
				switch(c)
				{
VSPACE_CASES: break;
					default: RRETURN(MATCH_NOMATCH);
				}
				ecode++;
				break;
			case OP_DNREF:
			case OP_DNREFI:
				caseless = op == OP_DNREFI;
				{
					ssh_l count = GET2(ecode, 1 + IMM2_SIZE);
					ssh_ws *slot = md->name_table + GET2(ecode, 1) * md->name_entry_size;
					ecode += 1 + 2 * IMM2_SIZE;
					length = (md->jscript_compat) ? 0 : -1;
					offset = 0;

					while(count-- > 0)
					{
						offset = GET2(slot, 0) << 1;
						if(offset < offset_top && md->offset_vector[offset] >= 0)
						{
							length = md->offset_vector[offset + 1] - md->offset_vector[offset];
							break;
						}
						slot += md->name_entry_size;
					}
				}
				goto REF_REPEAT;

			case OP_REF:
			case OP_REFI:
				caseless = op == OP_REFI;
				offset = GET2(ecode, 1) << 1;
				ecode += 1 + IMM2_SIZE;
				if(offset >= offset_top || md->offset_vector[offset] < 0)
					length = (md->jscript_compat) ? 0 : -1;
				else
					length = md->offset_vector[offset + 1] - md->offset_vector[offset];
REF_REPEAT:
				switch(*ecode)
				{
					case OP_CRSTAR:
					case OP_CRMINSTAR:
					case OP_CRPLUS:
					case OP_CRMINPLUS:
					case OP_CRQUERY:
					case OP_CRMINQUERY:
						c = *ecode++ - OP_CRSTAR;
						minimize = (c & 1) != 0;
						min = rep_min[c];
						max = rep_max[c];
						if(max == 0) max = INT_MAX;
						break;

					case OP_CRRANGE:
					case OP_CRMINRANGE:
						minimize = (*ecode == OP_CRMINRANGE);
						min = GET2(ecode, 1);
						max = GET2(ecode, 1 + IMM2_SIZE);
						if(max == 0) max = INT_MAX;
						ecode += 1 + 2 * IMM2_SIZE;
						break;

					default:
						if((length = match_ref(offset, eptr, length, md, caseless)) < 0)
						{
							if(length == -2) eptr = md->end_subject;
							CHECK_PARTIAL();
							RRETURN(MATCH_NOMATCH);
						}
						eptr += length;
						continue;
				}
				if(length == 0) continue;
				if(length < 0 && min == 0) continue;
				for(i = 1; i <= min; i++)
				{
					ssh_l slength;
					if((slength = match_ref(offset, eptr, length, md, caseless)) < 0)
					{
						if(slength == -2) eptr = md->end_subject;
						CHECK_PARTIAL();
						RRETURN(MATCH_NOMATCH);
					}
					eptr += slength;
				}
				if(min == max) continue;
				if(minimize)
				{
					for(fi = min;; fi++)
					{
						ssh_l slength;
						RMATCH(eptr, ecode, offset_top, md, eptrb, RM14);
						if(rrc != MATCH_NOMATCH) RRETURN(rrc);
						if(fi >= max) RRETURN(MATCH_NOMATCH);
						if((slength = match_ref(offset, eptr, length, md, caseless)) < 0)
						{
							if(slength == -2) eptr = md->end_subject;
							CHECK_PARTIAL();
							RRETURN(MATCH_NOMATCH);
						}
						eptr += slength;
					}

				}
				else
				{
					pp = eptr;
					for(i = min; i < max; i++)
					{
						ssh_l slength;
						if((slength = match_ref(offset, eptr, length, md, caseless)) < 0)
						{
							if(slength == -2 && md->partial != 0 && md->end_subject > md->start_used_ptr)
							{
								md->hitend = TRUE;
								if(md->partial > 1) RRETURN(REGEX_ERROR_PARTIAL);
							}
							break;
						}
						eptr += slength;
					}

					while(eptr >= pp)
					{
						RMATCH(eptr, ecode, offset_top, md, eptrb, RM15);
						if(rrc != MATCH_NOMATCH) RRETURN(rrc);
						eptr -= length;
					}
					RRETURN(MATCH_NOMATCH);
				}




			case OP_NCLASS:
			case OP_CLASS:
			{

#define BYTE_MAP ((ssh_b *)data)
				data = ecode + 1;
				ecode += 1 + (32 / sizeof(ssh_ws));

				switch(*ecode)
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
						c = *ecode++ - OP_CRSTAR;
						if(c < OP_CRPOSSTAR - OP_CRSTAR) minimize = (c & 1) != 0;
						else possessive = TRUE;
						min = rep_min[c];
						max = rep_max[c];
						if(max == 0) max = INT_MAX;
						break;

					case OP_CRRANGE:
					case OP_CRMINRANGE:
					case OP_CRPOSRANGE:
						minimize = (*ecode == OP_CRMINRANGE);
						possessive = (*ecode == OP_CRPOSRANGE);
						min = GET2(ecode, 1);
						max = GET2(ecode, 1 + IMM2_SIZE);
						if(max == 0) max = INT_MAX;
						ecode += 1 + 2 * IMM2_SIZE;
						break;

					default:
						min = max = 1;
						break;
				}
				for(i = 1; i <= min; i++)
				{
					if(eptr >= md->end_subject)
					{
						SCHECK_PARTIAL();
						RRETURN(MATCH_NOMATCH);
					}
					c = *eptr++;
					if(c > 255)
					{
						if(op == OP_CLASS) RRETURN(MATCH_NOMATCH);
					}
					else
						if((BYTE_MAP[c / 8] & (1 << (c & 7))) == 0) RRETURN(MATCH_NOMATCH);
				}
				if(min == max) continue;
				if(minimize)
				{
					for(fi = min;; fi++)
					{
						RMATCH(eptr, ecode, offset_top, md, eptrb, RM17);
						if(rrc != MATCH_NOMATCH) RRETURN(rrc);
						if(fi >= max) RRETURN(MATCH_NOMATCH);
						if(eptr >= md->end_subject)
						{
							SCHECK_PARTIAL();
							RRETURN(MATCH_NOMATCH);
						}
						c = *eptr++;
						if(c > 255)
						{
							if(op == OP_CLASS) RRETURN(MATCH_NOMATCH);
						}
						else
							if((BYTE_MAP[c / 8] & (1 << (c & 7))) == 0) RRETURN(MATCH_NOMATCH);
					}
				}
				else
				{
					pp = eptr;
					for(i = min; i < max; i++)
					{
						if(eptr >= md->end_subject)
						{
							SCHECK_PARTIAL();
							break;
						}
						c = *eptr;
						if(c > 255)
						{
							if(op == OP_CLASS) break;
						}
						else
							if((BYTE_MAP[c / 8] & (1 << (c & 7))) == 0) break;
						eptr++;
					}

					if(possessive) continue;

					while(eptr >= pp)
					{
						RMATCH(eptr, ecode, offset_top, md, eptrb, RM19);
						if(rrc != MATCH_NOMATCH) RRETURN(rrc);
						eptr--;
					}
					RRETURN(MATCH_NOMATCH);
				}
#undef BYTE_MAP
			}
			case OP_XCLASS:
			{
				data = ecode + 1 + LINK_SIZE;
				ecode += GET(ecode, 1);
				switch(*ecode)
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
						c = *ecode++ - OP_CRSTAR;
						if(c < OP_CRPOSSTAR - OP_CRSTAR) minimize = (c & 1) != 0;
						else possessive = TRUE;
						min = rep_min[c];
						max = rep_max[c];
						if(max == 0) max = INT_MAX;
						break;
					case OP_CRRANGE:
					case OP_CRMINRANGE:
					case OP_CRPOSRANGE:
						minimize = (*ecode == OP_CRMINRANGE);
						possessive = (*ecode == OP_CRPOSRANGE);
						min = GET2(ecode, 1);
						max = GET2(ecode, 1 + IMM2_SIZE);
						if(max == 0) max = INT_MAX;
						ecode += 1 + 2 * IMM2_SIZE;
						break;
					default:
						min = max = 1;
						break;
				}
				for(i = 1; i <= min; i++)
				{
					if(eptr >= md->end_subject)
					{
						SCHECK_PARTIAL();
						RRETURN(MATCH_NOMATCH);
					}
					GETCHARINCTEST(c, eptr);
					if(!PRIV(xclass)(c, data, utf)) RRETURN(MATCH_NOMATCH);
				}
				if(min == max) continue;
				if(minimize)
				{
					for(fi = min;; fi++)
					{
						RMATCH(eptr, ecode, offset_top, md, eptrb, RM20);
						if(rrc != MATCH_NOMATCH) RRETURN(rrc);
						if(fi >= max) RRETURN(MATCH_NOMATCH);
						if(eptr >= md->end_subject)
						{
							SCHECK_PARTIAL();
							RRETURN(MATCH_NOMATCH);
						}
						GETCHARINCTEST(c, eptr);
						if(!PRIV(xclass)(c, data, utf)) RRETURN(MATCH_NOMATCH);
					}
				}
				else
				{
					pp = eptr;
					for(i = min; i < max; i++)
					{
						ssh_l len = 1;
						if(eptr >= md->end_subject)
						{
							SCHECK_PARTIAL();
							break;
						}
						c = *eptr;
						if(!PRIV(xclass)(c, data, utf)) break;
						eptr += len;
					}
					if(possessive) continue;
					for(;;)
					{
						RMATCH(eptr, ecode, offset_top, md, eptrb, RM21);
						if(rrc != MATCH_NOMATCH) RRETURN(rrc);
						if(eptr-- == pp) break;
					}
					RRETURN(MATCH_NOMATCH);
				}
			}
			case OP_CHAR:
				if(md->end_subject - eptr < 1)
				{
					SCHECK_PARTIAL();
					RRETURN(MATCH_NOMATCH);
				}
				if(ecode[1] != *eptr++) RRETURN(MATCH_NOMATCH);
				ecode += 2;
				break;
			case OP_CHARI:
				if(eptr >= md->end_subject)
				{
					SCHECK_PARTIAL();
					RRETURN(MATCH_NOMATCH);
				}
				if(TABLE_GET(ecode[1], md->lcc, ecode[1]) != TABLE_GET(*eptr, md->lcc, *eptr)) RRETURN(MATCH_NOMATCH);
				eptr++;
				ecode += 2;
				break;
			case OP_EXACT:
			case OP_EXACTI:
				min = max = GET2(ecode, 1);
				ecode += 1 + IMM2_SIZE;
				goto REPEATCHAR;

			case OP_POSUPTO:
			case OP_POSUPTOI:
				possessive = TRUE;
			case OP_UPTO:
			case OP_UPTOI:
			case OP_MINUPTO:
			case OP_MINUPTOI:
				min = 0;
				max = GET2(ecode, 1);
				minimize = *ecode == OP_MINUPTO || *ecode == OP_MINUPTOI;
				ecode += 1 + IMM2_SIZE;
				goto REPEATCHAR;
			case OP_POSSTAR:
			case OP_POSSTARI:
				possessive = TRUE;
				min = 0;
				max = INT_MAX;
				ecode++;
				goto REPEATCHAR;
			case OP_POSPLUS:
			case OP_POSPLUSI:
				possessive = TRUE;
				min = 1;
				max = INT_MAX;
				ecode++;
				goto REPEATCHAR;
			case OP_POSQUERY:
			case OP_POSQUERYI:
				possessive = TRUE;
				min = 0;
				max = 1;
				ecode++;
				goto REPEATCHAR;
			case OP_STAR:
			case OP_STARI:
			case OP_MINSTAR:
			case OP_MINSTARI:
			case OP_PLUS:
			case OP_PLUSI:
			case OP_MINPLUS:
			case OP_MINPLUSI:
			case OP_QUERY:
			case OP_QUERYI:
			case OP_MINQUERY:
			case OP_MINQUERYI:
				c = *ecode++ - ((op < OP_STARI) ? OP_STAR : OP_STARI);
				minimize = (c & 1) != 0;
				min = rep_min[c];
				max = rep_max[c];
				if(max == 0) max = INT_MAX;
REPEATCHAR:
				fc = *ecode++;
				if(op >= OP_STARI)
				{
					foc = TABLE_GET(fc, md->fcc, fc);
					for(i = 1; i <= min; i++)
					{
						ssh_u cc;
						if(eptr >= md->end_subject)
						{
							SCHECK_PARTIAL();
							RRETURN(MATCH_NOMATCH);
						}
						cc = UCHAR21TEST(eptr);
						if(fc != cc && foc != cc) RRETURN(MATCH_NOMATCH);
						eptr++;
					}
					if(min == max) continue;
					if(minimize)
					{
						for(fi = min;; fi++)
						{
							ssh_u cc;
							RMATCH(eptr, ecode, offset_top, md, eptrb, RM24);
							if(rrc != MATCH_NOMATCH) RRETURN(rrc);
							if(fi >= max) RRETURN(MATCH_NOMATCH);
							if(eptr >= md->end_subject)
							{
								SCHECK_PARTIAL();
								RRETURN(MATCH_NOMATCH);
							}
							cc = UCHAR21TEST(eptr);
							if(fc != cc && foc != cc) RRETURN(MATCH_NOMATCH);
							eptr++;
						}

					}
					else
					{
						pp = eptr;
						for(i = min; i < max; i++)
						{
							ssh_u cc;
							if(eptr >= md->end_subject)
							{
								SCHECK_PARTIAL();
								break;
							}
							cc = UCHAR21TEST(eptr);
							if(fc != cc && foc != cc) break;
							eptr++;
						}
						if(possessive) continue;
						for(;;)
						{
							if(eptr == pp) goto TAIL_RECURSE;
							RMATCH(eptr, ecode, offset_top, md, eptrb, RM25);
							eptr--;
							if(rrc != MATCH_NOMATCH) RRETURN(rrc);
						}

					}
				}
				else
				{
					for(i = 1; i <= min; i++)
					{
						if(eptr >= md->end_subject)
						{
							SCHECK_PARTIAL();
							RRETURN(MATCH_NOMATCH);
						}
						if(fc != UCHAR21INCTEST(eptr)) RRETURN(MATCH_NOMATCH);
					}

					if(min == max) continue;

					if(minimize)
					{
						for(fi = min;; fi++)
						{
							RMATCH(eptr, ecode, offset_top, md, eptrb, RM26);
							if(rrc != MATCH_NOMATCH) RRETURN(rrc);
							if(fi >= max) RRETURN(MATCH_NOMATCH);
							if(eptr >= md->end_subject)
							{
								SCHECK_PARTIAL();
								RRETURN(MATCH_NOMATCH);
							}
							if(fc != UCHAR21INCTEST(eptr)) RRETURN(MATCH_NOMATCH);
						}

					}
					else
					{
						pp = eptr;
						for(i = min; i < max; i++)
						{
							if(eptr >= md->end_subject)
							{
								SCHECK_PARTIAL();
								break;
							}
							if(fc != UCHAR21TEST(eptr)) break;
							eptr++;
						}
						if(possessive) continue;
						for(;;)
						{
							if(eptr == pp) goto TAIL_RECURSE;
							RMATCH(eptr, ecode, offset_top, md, eptrb, RM27);
							eptr--;
							if(rrc != MATCH_NOMATCH) RRETURN(rrc);
						}

					}
				}
			case OP_NOT:
			case OP_NOTI:
				if(eptr >= md->end_subject)
				{
					SCHECK_PARTIAL();
					RRETURN(MATCH_NOMATCH);
				}
				{
					register ssh_u ch = ecode[1];
					c = *eptr++;
					if(ch == c || (op == OP_NOTI && TABLE_GET(ch, md->fcc, ch) == c))
						RRETURN(MATCH_NOMATCH);
					ecode += 2;
				}
				break;



			case OP_NOTEXACT:
			case OP_NOTEXACTI:
				min = max = GET2(ecode, 1);
				ecode += 1 + IMM2_SIZE;
				goto REPEATNOTCHAR;

			case OP_NOTUPTO:
			case OP_NOTUPTOI:
			case OP_NOTMINUPTO:
			case OP_NOTMINUPTOI:
				min = 0;
				max = GET2(ecode, 1);
				minimize = *ecode == OP_NOTMINUPTO || *ecode == OP_NOTMINUPTOI;
				ecode += 1 + IMM2_SIZE;
				goto REPEATNOTCHAR;

			case OP_NOTPOSSTAR:
			case OP_NOTPOSSTARI:
				possessive = TRUE;
				min = 0;
				max = INT_MAX;
				ecode++;
				goto REPEATNOTCHAR;

			case OP_NOTPOSPLUS:
			case OP_NOTPOSPLUSI:
				possessive = TRUE;
				min = 1;
				max = INT_MAX;
				ecode++;
				goto REPEATNOTCHAR;

			case OP_NOTPOSQUERY:
			case OP_NOTPOSQUERYI:
				possessive = TRUE;
				min = 0;
				max = 1;
				ecode++;
				goto REPEATNOTCHAR;

			case OP_NOTPOSUPTO:
			case OP_NOTPOSUPTOI:
				possessive = TRUE;
				min = 0;
				max = GET2(ecode, 1);
				ecode += 1 + IMM2_SIZE;
				goto REPEATNOTCHAR;

			case OP_NOTSTAR:
			case OP_NOTSTARI:
			case OP_NOTMINSTAR:
			case OP_NOTMINSTARI:
			case OP_NOTPLUS:
			case OP_NOTPLUSI:
			case OP_NOTMINPLUS:
			case OP_NOTMINPLUSI:
			case OP_NOTQUERY:
			case OP_NOTQUERYI:
			case OP_NOTMINQUERY:
			case OP_NOTMINQUERYI:
				c = *ecode++ - ((op >= OP_NOTSTARI) ? OP_NOTSTARI : OP_NOTSTAR);
				minimize = (c & 1) != 0;
				min = rep_min[c];
				max = rep_max[c];
				if(max == 0) max = INT_MAX;
REPEATNOTCHAR:
				GETCHARINCTEST(fc, ecode);
				if(op >= OP_NOTSTARI)
				{
					foc = TABLE_GET(fc, md->fcc, fc);
					for(i = 1; i <= min; i++)
					{
						if(eptr >= md->end_subject)
						{
							SCHECK_PARTIAL();
							RRETURN(MATCH_NOMATCH);
						}
						if(fc == *eptr || foc == *eptr) RRETURN(MATCH_NOMATCH);
						eptr++;
					}
					if(min == max) continue;

					if(minimize)
					{
						for(fi = min;; fi++)
						{
							RMATCH(eptr, ecode, offset_top, md, eptrb, RM29);
							if(rrc != MATCH_NOMATCH) RRETURN(rrc);
							if(fi >= max) RRETURN(MATCH_NOMATCH);
							if(eptr >= md->end_subject)
							{
								SCHECK_PARTIAL();
								RRETURN(MATCH_NOMATCH);
							}
							if(fc == *eptr || foc == *eptr) RRETURN(MATCH_NOMATCH);
							eptr++;
						}
					}
					else
					{
						pp = eptr;
						for(i = min; i < max; i++)
						{
							if(eptr >= md->end_subject)
							{
								SCHECK_PARTIAL();
								break;
							}
							if(fc == *eptr || foc == *eptr) break;
							eptr++;
						}
						if(possessive) continue;
						for(;;)
						{
							if(eptr == pp) goto TAIL_RECURSE;
							RMATCH(eptr, ecode, offset_top, md, eptrb, RM31);
							if(rrc != MATCH_NOMATCH) RRETURN(rrc);
							eptr--;
						}
					}
				}
				else
				{
					for(i = 1; i <= min; i++)
					{
						if(eptr >= md->end_subject)
						{
							SCHECK_PARTIAL();
							RRETURN(MATCH_NOMATCH);
						}
						if(fc == *eptr++) RRETURN(MATCH_NOMATCH);
					}
					if(min == max) continue;
					if(minimize)
					{
						for(fi = min;; fi++)
						{
							RMATCH(eptr, ecode, offset_top, md, eptrb, RM33);
							if(rrc != MATCH_NOMATCH) RRETURN(rrc);
							if(fi >= max) RRETURN(MATCH_NOMATCH);
							if(eptr >= md->end_subject)
							{
								SCHECK_PARTIAL();
								RRETURN(MATCH_NOMATCH);
							}
							if(fc == *eptr++) RRETURN(MATCH_NOMATCH);
						}
					}
					else
					{
						pp = eptr;
						for(i = min; i < max; i++)
						{
							if(eptr >= md->end_subject)
							{
								SCHECK_PARTIAL();
								break;
							}
							if(fc == *eptr) break;
							eptr++;
						}
						if(possessive) continue;
						for(;;)
						{
							if(eptr == pp) goto TAIL_RECURSE;
							RMATCH(eptr, ecode, offset_top, md, eptrb, RM35);
							if(rrc != MATCH_NOMATCH) RRETURN(rrc);
							eptr--;
						}
					}
				}
			case OP_TYPEEXACT:
				min = max = GET2(ecode, 1);
				minimize = TRUE;
				ecode += 1 + IMM2_SIZE;
				goto REPEATTYPE;
			case OP_TYPEUPTO:
			case OP_TYPEMINUPTO:
				min = 0;
				max = GET2(ecode, 1);
				minimize = *ecode == OP_TYPEMINUPTO;
				ecode += 1 + IMM2_SIZE;
				goto REPEATTYPE;
			case OP_TYPEPOSSTAR:
				possessive = TRUE;
				min = 0;
				max = INT_MAX;
				ecode++;
				goto REPEATTYPE;
			case OP_TYPEPOSPLUS:
				possessive = TRUE;
				min = 1;
				max = INT_MAX;
				ecode++;
				goto REPEATTYPE;

			case OP_TYPEPOSQUERY:
				possessive = TRUE;
				min = 0;
				max = 1;
				ecode++;
				goto REPEATTYPE;

			case OP_TYPEPOSUPTO:
				possessive = TRUE;
				min = 0;
				max = GET2(ecode, 1);
				ecode += 1 + IMM2_SIZE;
				goto REPEATTYPE;

			case OP_TYPESTAR:
			case OP_TYPEMINSTAR:
			case OP_TYPEPLUS:
			case OP_TYPEMINPLUS:
			case OP_TYPEQUERY:
			case OP_TYPEMINQUERY:
				c = *ecode++ - OP_TYPESTAR;
				minimize = (c & 1) != 0;
				min = rep_min[c];
				max = rep_max[c];
				if(max == 0) max = INT_MAX;
REPEATTYPE:
				ctype = *ecode++;
				if(min > 0)
				{
						switch(ctype)
						{
								case OP_ANY:
									for(i = 1; i <= min; i++)
									{
										if(eptr >= md->end_subject)
										{
											SCHECK_PARTIAL();
											RRETURN(MATCH_NOMATCH);
										}
										if(IS_NEWLINE(eptr)) RRETURN(MATCH_NOMATCH);
										if(md->partial != 0 &&
										   eptr + 1 >= md->end_subject &&
										   NLBLOCK->nltype == NLTYPE_FIXED &&
										   NLBLOCK->nllen == 2 &&
										   *eptr == NLBLOCK->nl[0])
										{
											md->hitend = TRUE;
											if(md->partial > 1) RRETURN(REGEX_ERROR_PARTIAL);
										}
										eptr++;
									}
									break;

								case OP_ALLANY:
									if(eptr > md->end_subject - min)
									{
										SCHECK_PARTIAL();
										RRETURN(MATCH_NOMATCH);
									}
									eptr += min;
									break;

								case OP_ANYBYTE:
									if(eptr > md->end_subject - min)
									{
										SCHECK_PARTIAL();
										RRETURN(MATCH_NOMATCH);
									}
									eptr += min;
									break;

								case OP_ANYNL:
									for(i = 1; i <= min; i++)
									{
										if(eptr >= md->end_subject)
										{
											SCHECK_PARTIAL();
											RRETURN(MATCH_NOMATCH);
										}
										switch(*eptr++)
										{
											default: RRETURN(MATCH_NOMATCH);

											case CHAR_CR:
												if(eptr < md->end_subject && *eptr == CHAR_LF) eptr++;
												break;

											case CHAR_LF:
												break;

											case CHAR_VT:
											case CHAR_FF:
											case CHAR_NEL:
											case 0x2028:
											case 0x2029:
												if(md->bsr_anycrlf) RRETURN(MATCH_NOMATCH);
												break;
										}
									}
									break;

								case OP_NOT_HSPACE:
									for(i = 1; i <= min; i++)
									{
										if(eptr >= md->end_subject)
										{
											SCHECK_PARTIAL();
											RRETURN(MATCH_NOMATCH);
										}
										switch(*eptr++)
										{
											default: break;
HSPACE_BYTE_CASES:
HSPACE_MULTIBYTE_CASES :
												RRETURN(MATCH_NOMATCH);
										}
									}
									break;

								case OP_HSPACE:
									for(i = 1; i <= min; i++)
									{
										if(eptr >= md->end_subject)
										{
											SCHECK_PARTIAL();
											RRETURN(MATCH_NOMATCH);
										}
										switch(*eptr++)
										{
											default: RRETURN(MATCH_NOMATCH);
HSPACE_BYTE_CASES:
HSPACE_MULTIBYTE_CASES :
												break;
										}
									}
									break;

								case OP_NOT_VSPACE:
									for(i = 1; i <= min; i++)
									{
										if(eptr >= md->end_subject)
										{
											SCHECK_PARTIAL();
											RRETURN(MATCH_NOMATCH);
										}
										switch(*eptr++)
										{
VSPACE_BYTE_CASES:
VSPACE_MULTIBYTE_CASES :
											RRETURN(MATCH_NOMATCH);
											default: break;
										}
									}
									break;

								case OP_VSPACE:
									for(i = 1; i <= min; i++)
									{
										if(eptr >= md->end_subject)
										{
											SCHECK_PARTIAL();
											RRETURN(MATCH_NOMATCH);
										}
										switch(*eptr++)
										{
											default: RRETURN(MATCH_NOMATCH);
VSPACE_BYTE_CASES:
VSPACE_MULTIBYTE_CASES :
												break;
										}
									}
									break;

								case OP_NOT_DIGIT:
									for(i = 1; i <= min; i++)
									{
										if(eptr >= md->end_subject)
										{
											SCHECK_PARTIAL();
											RRETURN(MATCH_NOMATCH);
										}
										if(MAX_255(*eptr) && (md->ctypes[*eptr] & ctype_digit) != 0)
											RRETURN(MATCH_NOMATCH);
										eptr++;
									}
									break;

								case OP_DIGIT:
									for(i = 1; i <= min; i++)
									{
										if(eptr >= md->end_subject)
										{
											SCHECK_PARTIAL();
											RRETURN(MATCH_NOMATCH);
										}
										if(!MAX_255(*eptr) || (md->ctypes[*eptr] & ctype_digit) == 0)
											RRETURN(MATCH_NOMATCH);
										eptr++;
									}
									break;

								case OP_NOT_WHITESPACE:
									for(i = 1; i <= min; i++)
									{
										if(eptr >= md->end_subject)
										{
											SCHECK_PARTIAL();
											RRETURN(MATCH_NOMATCH);
										}
										if(MAX_255(*eptr) && (md->ctypes[*eptr] & ctype_space) != 0)
											RRETURN(MATCH_NOMATCH);
										eptr++;
									}
									break;

								case OP_WHITESPACE:
									for(i = 1; i <= min; i++)
									{
										if(eptr >= md->end_subject)
										{
											SCHECK_PARTIAL();
											RRETURN(MATCH_NOMATCH);
										}
										if(!MAX_255(*eptr) || (md->ctypes[*eptr] & ctype_space) == 0)
											RRETURN(MATCH_NOMATCH);
										eptr++;
									}
									break;

								case OP_NOT_WORDCHAR:
									for(i = 1; i <= min; i++)
									{
										if(eptr >= md->end_subject)
										{
											SCHECK_PARTIAL();
											RRETURN(MATCH_NOMATCH);
										}
										if(MAX_255(*eptr) && (md->ctypes[*eptr] & ctype_word) != 0)
											RRETURN(MATCH_NOMATCH);
										eptr++;
									}
									break;

								case OP_WORDCHAR:
									for(i = 1; i <= min; i++)
									{
										if(eptr >= md->end_subject)
										{
											SCHECK_PARTIAL();
											RRETURN(MATCH_NOMATCH);
										}
										if(!MAX_255(*eptr) || (md->ctypes[*eptr] & ctype_word) == 0)
											RRETURN(MATCH_NOMATCH);
										eptr++;
									}
									break;

								default:
									RRETURN(REGEX_ERROR_INTERNAL);
						}
				}
				if(min == max) continue;
				if(minimize)
				{
					{
							for(fi = min;; fi++)
							{
								RMATCH(eptr, ecode, offset_top, md, eptrb, RM43);
								if(rrc != MATCH_NOMATCH) RRETURN(rrc);
								if(fi >= max) RRETURN(MATCH_NOMATCH);
								if(eptr >= md->end_subject)
								{
									SCHECK_PARTIAL();
									RRETURN(MATCH_NOMATCH);
								}
								if(ctype == OP_ANY && IS_NEWLINE(eptr))
									RRETURN(MATCH_NOMATCH);
								c = *eptr++;
								switch(ctype)
								{
									case OP_ANY:
										if(md->partial != 0 &&
										   eptr >= md->end_subject &&
										   NLBLOCK->nltype == NLTYPE_FIXED &&
										   NLBLOCK->nllen == 2 &&
										   c == NLBLOCK->nl[0])
										{
											md->hitend = TRUE;
											if(md->partial > 1) RRETURN(REGEX_ERROR_PARTIAL);
										}
										break;

									case OP_ALLANY:
									case OP_ANYBYTE:
										break;

									case OP_ANYNL:
										switch(c)
										{
											default: RRETURN(MATCH_NOMATCH);
											case CHAR_CR:
												if(eptr < md->end_subject && *eptr == CHAR_LF) eptr++;
												break;

											case CHAR_LF:
												break;

											case CHAR_VT:
											case CHAR_FF:
											case CHAR_NEL:
											case 0x2028:
											case 0x2029:
												if(md->bsr_anycrlf) RRETURN(MATCH_NOMATCH);
												break;
										}
										break;

									case OP_NOT_HSPACE:
										switch(c)
										{
											default: break;
HSPACE_BYTE_CASES:
HSPACE_MULTIBYTE_CASES :
												RRETURN(MATCH_NOMATCH);
										}
										break;

									case OP_HSPACE:
										switch(c)
										{
											default: RRETURN(MATCH_NOMATCH);
HSPACE_BYTE_CASES:
HSPACE_MULTIBYTE_CASES :
												break;
										}
										break;

									case OP_NOT_VSPACE:
										switch(c)
										{
											default: break;
VSPACE_BYTE_CASES:
VSPACE_MULTIBYTE_CASES :
												RRETURN(MATCH_NOMATCH);
										}
										break;

									case OP_VSPACE:
										switch(c)
										{
											default: RRETURN(MATCH_NOMATCH);
VSPACE_BYTE_CASES:
VSPACE_MULTIBYTE_CASES :
												break;
										}
										break;

									case OP_NOT_DIGIT:
										if(MAX_255(c) && (md->ctypes[c] & ctype_digit) != 0) RRETURN(MATCH_NOMATCH);
										break;

									case OP_DIGIT:
										if(!MAX_255(c) || (md->ctypes[c] & ctype_digit) == 0) RRETURN(MATCH_NOMATCH);
										break;

									case OP_NOT_WHITESPACE:
										if(MAX_255(c) && (md->ctypes[c] & ctype_space) != 0) RRETURN(MATCH_NOMATCH);
										break;

									case OP_WHITESPACE:
										if(!MAX_255(c) || (md->ctypes[c] & ctype_space) == 0) RRETURN(MATCH_NOMATCH);
										break;

									case OP_NOT_WORDCHAR:
										if(MAX_255(c) && (md->ctypes[c] & ctype_word) != 0) RRETURN(MATCH_NOMATCH);
										break;

									case OP_WORDCHAR:
										if(!MAX_255(c) || (md->ctypes[c] & ctype_word) == 0) RRETURN(MATCH_NOMATCH);
										break;

									default:
										RRETURN(REGEX_ERROR_INTERNAL);
								}
							}
						}
				}
				else
				{
					pp = eptr;
						{
							switch(ctype)
							{
								case OP_ANY:
									for(i = min; i < max; i++)
									{
										if(eptr >= md->end_subject)
										{
											SCHECK_PARTIAL();
											break;
										}
										if(IS_NEWLINE(eptr)) break;
										if(md->partial != 0 &&
										   eptr + 1 >= md->end_subject &&
										   NLBLOCK->nltype == NLTYPE_FIXED &&
										   NLBLOCK->nllen == 2 &&
										   *eptr == NLBLOCK->nl[0])
										{
											md->hitend = TRUE;
											if(md->partial > 1) RRETURN(REGEX_ERROR_PARTIAL);
										}
										eptr++;
									}
									break;

								case OP_ALLANY:
								case OP_ANYBYTE:
									c = max - min;
									if(c > (ssh_u)(md->end_subject - eptr))
									{
										eptr = md->end_subject;
										SCHECK_PARTIAL();
									}
									else eptr += c;
									break;

								case OP_ANYNL:
									for(i = min; i < max; i++)
									{
										if(eptr >= md->end_subject)
										{
											SCHECK_PARTIAL();
											break;
										}
										c = *eptr;
										if(c == CHAR_CR)
										{
											if(++eptr >= md->end_subject) break;
											if(*eptr == CHAR_LF) eptr++;
										}
										else
										{
											if(c != CHAR_LF && (md->bsr_anycrlf || (c != CHAR_VT && c != CHAR_FF && c != CHAR_NEL && c != 0x2028 && c != 0x2029))) break;
											eptr++;
										}
									}
									break;

								case OP_NOT_HSPACE:
									for(i = min; i < max; i++)
									{
										if(eptr >= md->end_subject)
										{
											SCHECK_PARTIAL();
											break;
										}
										switch(*eptr)
										{
											default: eptr++; break;
HSPACE_BYTE_CASES:
HSPACE_MULTIBYTE_CASES :
												goto ENDLOOP00;
										}
									}
ENDLOOP00:
									break;

								case OP_HSPACE:
									for(i = min; i < max; i++)
									{
										if(eptr >= md->end_subject)
										{
											SCHECK_PARTIAL();
											break;
										}
										switch(*eptr)
										{
											default: goto ENDLOOP01;
HSPACE_BYTE_CASES:
HSPACE_MULTIBYTE_CASES :
												eptr++; break;
										}
									}
ENDLOOP01:
									break;

								case OP_NOT_VSPACE:
									for(i = min; i < max; i++)
									{
										if(eptr >= md->end_subject)
										{
											SCHECK_PARTIAL();
											break;
										}
										switch(*eptr)
										{
											default: eptr++; break;
VSPACE_BYTE_CASES:
VSPACE_MULTIBYTE_CASES :
												goto ENDLOOP02;
										}
									}
ENDLOOP02:
									break;

								case OP_VSPACE:
									for(i = min; i < max; i++)
									{
										if(eptr >= md->end_subject)
										{
											SCHECK_PARTIAL();
											break;
										}
										switch(*eptr)
										{
											default: goto ENDLOOP03;
VSPACE_BYTE_CASES:
VSPACE_MULTIBYTE_CASES :
												eptr++; break;
										}
									}
ENDLOOP03:
									break;

								case OP_NOT_DIGIT:
									for(i = min; i < max; i++)
									{
										if(eptr >= md->end_subject)
										{
											SCHECK_PARTIAL();
											break;
										}
										if(MAX_255(*eptr) && (md->ctypes[*eptr] & ctype_digit) != 0) break;
										eptr++;
									}
									break;

								case OP_DIGIT:
									for(i = min; i < max; i++)
									{
										if(eptr >= md->end_subject)
										{
											SCHECK_PARTIAL();
											break;
										}
										if(!MAX_255(*eptr) || (md->ctypes[*eptr] & ctype_digit) == 0) break;
										eptr++;
									}
									break;

								case OP_NOT_WHITESPACE:
									for(i = min; i < max; i++)
									{
										if(eptr >= md->end_subject)
										{
											SCHECK_PARTIAL();
											break;
										}
										if(MAX_255(*eptr) && (md->ctypes[*eptr] & ctype_space) != 0) break;
										eptr++;
									}
									break;

								case OP_WHITESPACE:
									for(i = min; i < max; i++)
									{
										if(eptr >= md->end_subject)
										{
											SCHECK_PARTIAL();
											break;
										}
										if(!MAX_255(*eptr) || (md->ctypes[*eptr] & ctype_space) == 0) break;
										eptr++;
									}
									break;

								case OP_NOT_WORDCHAR:
									for(i = min; i < max; i++)
									{
										if(eptr >= md->end_subject)
										{
											SCHECK_PARTIAL();
											break;
										}
										if(MAX_255(*eptr) && (md->ctypes[*eptr] & ctype_word) != 0) break;
										eptr++;
									}
									break;

								case OP_WORDCHAR:
									for(i = min; i < max; i++)
									{
										if(eptr >= md->end_subject)
										{
											SCHECK_PARTIAL();
											break;
										}
										if(!MAX_255(*eptr) || (md->ctypes[*eptr] & ctype_word) == 0) break;
										eptr++;
									}
									break;

								default:
									RRETURN(REGEX_ERROR_INTERNAL);
							}
							if(possessive) continue;
							for(;;)
							{
								if(eptr == pp) goto TAIL_RECURSE;
								RMATCH(eptr, ecode, offset_top, md, eptrb, RM47);
								if(rrc != MATCH_NOMATCH) RRETURN(rrc);
								eptr--;
								if(ctype == OP_ANYNL && eptr > pp  && *eptr == CHAR_LF && eptr[-1] == CHAR_CR) eptr--;
							}
						}
				}
			default:
				RRETURN(REGEX_ERROR_UNKNOWN_OPCODE);
		}
	}
}


#undef fc
#undef fi

REGEX_EXP_DEFN ssh_l regex16_exec(const regex16 *argument_re, REGEX_SPTR16 subject, ssh_l length, ssh_l start_offset, ssh_l options, ssh_l* offsets, ssh_l offsetcount)
{
	ssh_l rc, ocount, arg_offset_max;
	ssh_l newline;
	BOOL using_temporary_offsets = FALSE;
	BOOL anchored;
	BOOL startline;
	BOOL firstline;
	BOOL utf;
	BOOL has_first_char = FALSE;
	BOOL has_req_char = FALSE;
	ssh_ws first_char = 0;
	ssh_ws first_char2 = 0;
	ssh_ws req_char = 0;
	ssh_ws req_char2 = 0;
	match_data match_block;
	match_data *md = &match_block;
	const ssh_b *tables;
	const ssh_b *start_bits = NULL;
	ssh_wcs start_match = (ssh_wcs)subject + start_offset;
	ssh_wcs end_subject;
	ssh_wcs start_partial = NULL;
	ssh_wcs match_partial = NULL;
	ssh_wcs req_char_ptr = start_match - 1;
	const regex_study_data *study;
	const REAL_PCRE *re = (const REAL_PCRE *)argument_re;
	if((options & ~PUBLIC_EXEC_OPTIONS) != 0) return REGEX_ERROR_BADOPTION;
	if(re == NULL || subject == NULL || (offsets == NULL && offsetcount > 0)) return REGEX_ERROR_NULL;
	if(offsetcount < 0) return REGEX_ERROR_BADCOUNT;
	if(length < 0) return REGEX_ERROR_BADLENGTH;
	if(start_offset < 0 || start_offset > length) return REGEX_ERROR_BADOFFSET;
	if(re->magic_number != MAGIC_NUMBER) return re->magic_number == REVERSED_MAGIC_NUMBER ? REGEX_ERROR_BADENDIANNESS : REGEX_ERROR_BADMAGIC;
	if((re->flags & REGEX_MODE) == 0) return REGEX_ERROR_BADMODE;
	utf = md->utf = (re->options & REGEX_UTF8) != 0;
	md->partial = ((options & REGEX_PARTIAL_HARD) != 0) ? 2 : ((options & REGEX_PARTIAL_SOFT) != 0) ? 1 : 0;
	md->name_table = (ssh_ws *)re + re->name_table_offset;
	md->name_count = re->name_count;
	md->name_entry_size = re->name_entry_size;
	study = NULL;
	md->match_limit = MATCH_LIMIT;
	md->match_limit_recursion = MATCH_LIMIT_RECURSION;
	md->callout_data = NULL;
	tables = re->tables;
	if((re->flags & REGEX_MLSET) != 0 && re->limit_match < md->match_limit) md->match_limit = re->limit_match;
	if((re->flags & REGEX_RLSET) != 0 && re->limit_recursion < md->match_limit_recursion) md->match_limit_recursion = re->limit_recursion;
	if(tables == NULL) tables = PRIV(default_tables);
	anchored = ((re->options | options) & REGEX_ANCHORED) != 0;
	startline = (re->flags & REGEX_STARTLINE) != 0;
	firstline = (re->options & REGEX_FIRSTLINE) != 0;
	md->start_code = (const ssh_ws *)re + re->name_table_offset + re->name_count * re->name_entry_size;
	md->start_subject = (REGEX_PUCHAR)subject;
	md->start_offset = start_offset;
	md->end_subject = md->start_subject + length;
	end_subject = md->end_subject;
	md->endonly = (re->options & REGEX_DOLLAR_ENDONLY) != 0;
	md->use_ucp = (re->options & REGEX_UCP) != 0;
	md->jscript_compat = (re->options & REGEX_JAVASCRIPT_COMPAT) != 0;
	md->ignore_skip_arg = 0;
	md->notbol = (options & REGEX_NOTBOL) != 0;
	md->noteol = (options & REGEX_NOTEOL) != 0;
	md->notempty = (options & REGEX_NOTEMPTY) != 0;
	md->notempty_atstart = (options & REGEX_NOTEMPTY_ATSTART) != 0;
	md->hitend = FALSE;
	md->mark = md->nomatch_mark = NULL;
	md->recursive = NULL;
	md->hasthen = (re->flags & REGEX_HASTHEN) != 0;
	md->lcc = tables + lcc_offset;
	md->fcc = tables + fcc_offset;
	md->ctypes = tables + ctypes_offset;
	switch(options & (REGEX_BSR_ANYCRLF | REGEX_BSR_UNICODE))
	{
		case 0:
			md->bsr_anycrlf = (((re->options & (REGEX_BSR_ANYCRLF | REGEX_BSR_UNICODE)) != 0) ? ((re->options & REGEX_BSR_ANYCRLF) != 0) : FALSE);
			break;
		case REGEX_BSR_ANYCRLF:
			md->bsr_anycrlf = TRUE;
			break;
		case REGEX_BSR_UNICODE:
			md->bsr_anycrlf = FALSE;
			break;
		default: return REGEX_ERROR_BADNEWLINE;
	}
	switch((((options & REGEX_NEWLINE_BITS) == 0) ? re->options : (ssh_u)options) & REGEX_NEWLINE_BITS)
	{
		case 0: newline = NEWLINE; break;
		case REGEX_NEWLINE_CR: newline = CHAR_CR; break;
		case REGEX_NEWLINE_LF: newline = CHAR_NL; break;
		case REGEX_NEWLINE_CR + REGEX_NEWLINE_LF: newline = (CHAR_CR << 8) | CHAR_NL; break;
		case REGEX_NEWLINE_ANY: newline = -1; break;
		case REGEX_NEWLINE_ANYCRLF: newline = -2; break;
		default: return REGEX_ERROR_BADNEWLINE;
	}

	if(newline == -2) md->nltype = NLTYPE_ANYCRLF;
	else if(newline < 0) md->nltype = NLTYPE_ANY;
	else
	{
		md->nltype = NLTYPE_FIXED;
		if(newline > 255)
		{
			md->nllen = 2;
			md->nl[0] = (newline >> 8) & 255;
			md->nl[1] = newline & 255;
		}
		else
		{
			md->nllen = 1;
			md->nl[0] = (ssh_ws)newline;
		}
	}
	if(md->partial && (re->flags & REGEX_NOPARTIAL) != 0) return REGEX_ERROR_BADPARTIAL;
	ocount = offsetcount - (offsetcount % 3);
	arg_offset_max = (2 * ocount) / 3;
	if(re->top_backref > 0 && re->top_backref >= ocount / 3)
	{
		ocount = re->top_backref * 3 + 3;
		md->offset_vector = (ssh_l*)malloc(ocount * sizeof(ssh_l));
		if(md->offset_vector == NULL) return REGEX_ERROR_NOMEMORY;
		using_temporary_offsets = TRUE;
	}
	else md->offset_vector = offsets;
	md->offset_end = ocount;
	md->offset_max = (2 * ocount) / 3;
	md->capture_last = 0;
	if(md->offset_vector != NULL)
	{
		register ssh_l* iptr = md->offset_vector + ocount;
		register ssh_l* iend = iptr - re->top_bracket;
		if(iend < md->offset_vector + 2) iend = md->offset_vector + 2;
		while(--iptr >= iend) *iptr = -1;
		md->offset_vector[0] = md->offset_vector[1] = -1;
	}
	if(!anchored)
	{
		if((re->flags & REGEX_FIRSTSET) != 0)
		{
			has_first_char = TRUE;
			first_char = first_char2 = (ssh_ws)(re->first_char);
			if((re->flags & REGEX_FCH_CASELESS) != 0)
			{
				first_char2 = TABLE_GET(first_char, md->fcc, first_char);
			}
		}
		else
			if(!startline && study != NULL && (study->flags & REGEX_STUDY_MAPPED) != 0)
			   start_bits = study->start_bits;
	}
	if((re->flags & REGEX_REQCHSET) != 0)
	{
		has_req_char = TRUE;
		req_char = req_char2 = (ssh_ws)(re->req_char);
		if((re->flags & REGEX_RCH_CASELESS) != 0)
		{
			req_char2 = TABLE_GET(req_char, md->fcc, req_char);
		}
	}
	for(;;)
	{
		REGEX_PUCHAR save_end_subject = end_subject;
		REGEX_PUCHAR new_start_match;
		if(firstline)
		{
			REGEX_PUCHAR t = start_match;
			while(t < md->end_subject && !IS_NEWLINE(t)) t++;
			end_subject = t;
		}
		if(((options | re->options) & REGEX_NO_START_OPTIMIZE) == 0)
		{
			if(has_first_char)
			{
				ssh_ws smc;
				if(first_char != first_char2)
					while(start_match < end_subject && (smc = UCHAR21TEST(start_match)) != first_char && smc != first_char2) start_match++;
				else
					while(start_match < end_subject && UCHAR21TEST(start_match) != first_char)
						start_match++;
			}
			else if(startline)
			{
				if(start_match > md->start_subject + start_offset)
				{
					while(start_match < end_subject && !WAS_NEWLINE(start_match)) start_match++;
					if(start_match[-1] == CHAR_CR && (md->nltype == NLTYPE_ANY || md->nltype == NLTYPE_ANYCRLF) && start_match < end_subject && UCHAR21TEST(start_match) == CHAR_NL)
					   start_match++;
				}
			}
			else if(start_bits != NULL)
			{
				while(start_match < end_subject)
				{
					register ssh_u c = UCHAR21TEST(start_match);
					if(c > 255) c = 255;
					if((start_bits[c / 8] & (1 << (c & 7))) != 0) break;
					start_match++;
				}
			}
		}
		end_subject = save_end_subject;
		if(((options | re->options) & REGEX_NO_START_OPTIMIZE) == 0 && !md->partial)
		{
			if(study != NULL && (study->flags & REGEX_STUDY_MINLEN) != 0 && (ssh_u)(end_subject - start_match) < study->minlength)
			{
				rc = MATCH_NOMATCH;
				break;
			}
			if(has_req_char && end_subject - start_match < REQ_BYTE_MAX)
			{
				register REGEX_PUCHAR p = start_match + (has_first_char ? 1 : 0);
				if(p > req_char_ptr)
				{
					if(req_char != req_char2)
					{
						while(p < end_subject)
						{
							register ssh_u pp = UCHAR21INCTEST(p);
							if(pp == req_char || pp == req_char2) { p--; break; }
						}
					}
					else
					{
						while(p < end_subject)
						{
							if(UCHAR21INCTEST(p) == req_char) { p--; break; }
						}
					}
					if(p >= end_subject)
					{
						rc = MATCH_NOMATCH;
						break;
					}
					req_char_ptr = p;
				}
			}
		}
		md->start_match_ptr = start_match;
		md->start_used_ptr = start_match;
		md->match_call_count = 0;
		md->match_function_type = 0;
		md->end_offset_top = 0;
		md->skip_arg_count = 0;
		rc = match(start_match, md->start_code, start_match, 2, md, NULL, 0);
		if(md->hitend && start_partial == NULL)
		{
			start_partial = md->start_used_ptr;
			match_partial = start_match;
		}
		switch(rc)
		{
			case MATCH_SKIP_ARG:
				new_start_match = start_match;
				md->ignore_skip_arg = md->skip_arg_count;
				break;
			case MATCH_SKIP:
				if(md->start_match_ptr > start_match)
				{
					new_start_match = md->start_match_ptr;
					break;
				}
			case MATCH_NOMATCH:
			case MATCH_PRUNE:
			case MATCH_THEN:
				md->ignore_skip_arg = 0;
				new_start_match = start_match + 1;
				break;
			case MATCH_COMMIT:
				rc = MATCH_NOMATCH;
				goto ENDLOOP;
			default:
				goto ENDLOOP;
		}
		rc = MATCH_NOMATCH;
		if(firstline && IS_NEWLINE(start_match)) break;
		start_match = new_start_match;
		if(anchored || start_match > end_subject) break;
		if(start_match > (REGEX_PUCHAR)subject + start_offset && start_match[-1] == CHAR_CR && start_match < end_subject && *start_match == CHAR_NL && (re->flags & REGEX_HASCRORLF) == 0 && (md->nltype == NLTYPE_ANY || md->nltype == NLTYPE_ANYCRLF || md->nllen == 2))
		   start_match++;
		md->mark = NULL;
	}
ENDLOOP:
	if(rc == MATCH_MATCH || rc == MATCH_ACCEPT)
	{
		if(using_temporary_offsets)
		{
			if(arg_offset_max >= 4) memcpy(offsets + 2, md->offset_vector + 2, (arg_offset_max - 2) * sizeof(ssh_l));
			if(md->end_offset_top > arg_offset_max) md->capture_last |= OVFLBIT;
			free(md->offset_vector);
		}
		rc = ((md->capture_last & OVFLBIT) != 0 && md->end_offset_top >= arg_offset_max) ? 0 : md->end_offset_top / 2;
		if(md->end_offset_top / 2 <= re->top_bracket && offsets != NULL)
		{
			ssh_l* iptr, *iend;
			ssh_l resetcount = 2 + re->top_bracket * 2;
			if(resetcount > offsetcount) resetcount = offsetcount;
			iptr = offsets + md->end_offset_top;
			iend = offsets + resetcount;
			while(iptr < iend) *iptr++ = -1;
		}
		if(offsetcount < 2) rc = 0; else
		{
			offsets[0] = (ssh_l)(md->start_match_ptr - md->start_subject);
			offsets[1] = (ssh_l)(md->end_match_ptr - md->start_subject);
		}
		return rc;
	}
	if(using_temporary_offsets) free(md->offset_vector);
	if(rc != MATCH_NOMATCH && rc != REGEX_ERROR_PARTIAL) return rc;
	if(match_partial != NULL)
	{
		md->mark = NULL;
		if(offsetcount > 1)
		{
			offsets[0] = (ssh_l)(start_partial - (REGEX_PUCHAR)subject);
			offsets[1] = (ssh_l)(end_subject - (REGEX_PUCHAR)subject);
			if(offsetcount > 2) offsets[2] = (ssh_l)(match_partial - (REGEX_PUCHAR)subject);
		}
		rc = REGEX_ERROR_PARTIAL;
	}
	else
	{
		rc = REGEX_ERROR_NOMATCH;
	}
	return rc;
}
