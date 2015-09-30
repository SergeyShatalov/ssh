
#include "stdafx.h"
#include "ssh_zip.h"

namespace ssh
{
	void ZipDeflate::pqdownheap(zip_ct_data* tree, int k)
	{
		int v(heap[k]), j(k << 1);
		while(j <= heap_len)
		{
			if(j < heap_len && smaller(tree, heap[j+1], heap[j], depth)) j++;
			if(smaller(tree, v, heap[j], depth)) break;
			heap[k] = heap[j];  k = j;
			j <<= 1;
		}
		heap[k] = v;
	}

	void ZipDeflate::compress_block(zip_ct_data* ltree, zip_ct_data* dtree)
	{
		UINT dist, lx(0), code;
		int lc, extra;
		if(last_lit) do
		{
			dist = d_buf[lx];
			lc = l_buf[lx++];
			if(dist == 0)
			{
				send_code(lc, ltree);
			}
			else
			{
				code = _length_code[lc];
				send_code(code + LITERALS + 1, ltree);
				extra = extra_lbits[code];
				if(extra) {lc -= base_length[code]; send_bits(lc, extra);}
				dist--;
				code = d_code(dist);
				send_code(code, dtree);
				extra = extra_dbits[code];
				if(extra) {dist -= base_dist[code]; send_bits(dist, extra);}
			}
		} while(lx < last_lit);
		send_code(END_BLOCK, ltree);
	}

	void ZipDeflate::build_tree(zip_tree_desc* desc)
	{
		zip_ct_data* tree(desc->dyn_tree);
		const zip_ct_data* stree(desc->stat_desc->static_tree);
		const int* extra(desc->stat_desc->extra_bits);
		int elems(desc->stat_desc->elems), base(desc->stat_desc->extra_base), max_length(desc->stat_desc->max_length), n, m, max_code = -1, node, h, bits, xbits, overflow(0);
		WORD next_code[MAX_BITS + 1], code(0), f;

		heap_len = 0, heap_max = HEAP_SIZE;
		for(n = 0; n < elems; n++) {if(tree[n].fc.freq != 0) {heap[++(heap_len)] = max_code = n; depth[n] = 0;} else tree[n].dl.len = 0;}
		while(heap_len < 2)
		{
			node = heap[++(heap_len)] = (max_code < 2 ? ++max_code : 0);
			tree[node].fc.freq = 1;
			depth[node] = 0;
			opt_len--;
			if(stree) static_len -= stree[node].dl.len;
		}
		desc->max_code = max_code;
		for(n = heap_len / 2; n >= 1; n--) pqdownheap(tree, n);
		node = elems;
		do
		{
			n = heap[1];
			heap[1] = heap[heap_len--];
			pqdownheap(tree, 1);
			m = heap[1];
			heap[--(heap_max)] = n;
			heap[--(heap_max)] = m;
			tree[node].fc.freq = tree[n].fc.freq + tree[m].fc.freq;
			depth[node] = (BYTE)((depth[n] >= depth[m] ? depth[n] : depth[m]) + 1);
			tree[n].dl.dad = tree[m].dl.dad = (WORD)node;
			heap[1] = node++;
			pqdownheap(tree, 1);

		} while (heap_len >= 2);
		heap[--(heap_max)] = heap[1];
		
		max_code = desc->max_code;

		for(bits = 0; bits <= MAX_BITS; bits++) bl_count[bits] = 0;
		tree[heap[heap_max]].dl.len = 0;
		for(h = heap_max + 1; h < HEAP_SIZE; h++)
		{
			n = heap[h];
			bits = tree[tree[n].dl.dad].dl.len + 1;
			if(bits > max_length) bits = max_length, overflow++;
			tree[n].dl.len = (WORD)bits;
			if(n > max_code) continue;
			bl_count[bits]++;
			xbits = 0;
			if(n >= base) xbits = extra[n-base];
			f = tree[n].fc.freq;
			opt_len += (DWORD)f * (bits + xbits);
			if(stree) static_len += (DWORD)f * (stree[n].dl.len + xbits);
		}
		if(overflow == 0) return;
		do
		{
			bits = max_length-1;
			while(bl_count[bits] == 0) bits--;
			bl_count[bits]--;
			bl_count[bits + 1] += 2;
			bl_count[max_length]--;
			overflow -= 2;
		} while(overflow > 0);
		for(bits = max_length; bits != 0; bits--)
		{
			n = bl_count[bits];
			while(n)
			{
				m = heap[--h];
				if(m > max_code) continue;
				if((UINT)tree[m].dl.len != (UINT)bits)
				{
					opt_len += ((long)bits - (long)tree[m].dl.len) *(long)tree[m].fc.freq;
					tree[m].dl.len = (WORD)bits;
				}
				n--;
			}
		}		
		for(bits = 1; bits <= MAX_BITS; bits++) { next_code[bits] = code = (code + bl_count[bits - 1]) << 1; }
		for(n = 0; n <= max_code; n++)
		{
			int len = tree[n].dl.len;
			if(len == 0) continue;
			UINT code(next_code[len]++), res = 0;
			do { res |= code & 1; code >>= 1, res <<= 1; } while(--len > 0);
			tree[n].fc.code = (res >> 1);
		}
	}

	void ZipDeflate::init_block()
	{
		int n;
		for(n = 0; n < L_CODES;  n++) dyn_ltree[n].fc.freq = 0;
		for(n = 0; n < D_CODES;  n++) dyn_dtree[n].fc.freq = 0;
		for(n = 0; n < BL_CODES; n++) bl_tree[n].fc.freq = 0;
		dyn_ltree[END_BLOCK].fc.freq = 1;
		opt_len = static_len = 0L;
		last_lit = matches = 0;
	}

	void ZipDeflate::zip_putShortMSB(UINT b)
	{
		put_byte((BYTE)(b >> 8));
		put_byte((BYTE)(b & 0xff));
	}

	void ZipDeflate::zip_bi_flush()
	{
		if(bi_valid == 16) {put_short(bi_buf); bi_buf = 0; bi_valid = 0;}
		else if(bi_valid >= 8) {put_byte((BYTE)bi_buf); bi_buf >>= 8; bi_valid -= 8;}
	}

	void ZipDeflate::_zip_tr_stored_block(char* buf, DWORD stored_len, int last)
	{
		send_bits(last, 3);

		if(bi_valid > 8) { put_short(bi_buf); }
		else if(bi_valid > 0) { put_byte((BYTE)bi_buf); }
		bi_buf = 0; bi_valid = 0;

		put_short((WORD)stored_len);
		put_short((WORD)~stored_len);
		while(stored_len--) { put_byte(*buf++); }
	}

	void ZipDeflate::send_tree(zip_ct_data* tree, int max_code)
	{
		int n, prevlen = -1, curlen, nextlen(tree[0].dl.len), count(0), max_count(7), min_count(4);
		if(nextlen == 0) max_count = 138, min_count = 3;
		for(n = 0; n <= max_code; n++)
		{
			curlen = nextlen; nextlen = tree[n + 1].dl.len;
			if(++count < max_count && curlen == nextlen) continue;
			else if(count < min_count) {do {send_code(curlen, bl_tree);} while(--count != 0);}
			else if(curlen)
			{
				if(curlen != prevlen) {send_code(curlen, bl_tree); count--;}
				send_code(REP_3_6, bl_tree); send_bits(count - 3, 2);
			}
			else if(count <= 10) {send_code(REPZ_3_10, bl_tree); send_bits(count - 3, 3);}
			else {send_code(REPZ_11_138, bl_tree); send_bits(count - 11, 7);}
			count = 0; prevlen = curlen;
			if(nextlen == 0) max_count = 138, min_count = 3;
			else if(curlen == nextlen) max_count = 6, min_count = 3; else max_count = 7, min_count = 4;
		}
	}

	void ZipDeflate::scan_tree(zip_ct_data* tree, int max_code)
	{
		int n, prevlen(-1), curlen, nextlen(tree[0].dl.len), count(0), max_count(7), min_count(4);
		if(nextlen == 0) max_count = 138, min_count = 3;
		tree[max_code+1].dl.len = (WORD)0xffff;
		for(n = 0; n <= max_code; n++)
		{
			curlen = nextlen; nextlen = tree[n+1].dl.len;
			if(++count < max_count && curlen == nextlen) continue;
			else if(count < min_count) bl_tree[curlen].fc.freq += count;
			else if(curlen) {if(curlen != prevlen) bl_tree[curlen].fc.freq++; bl_tree[REP_3_6].fc.freq++;}
			else if(count <= 10) bl_tree[REPZ_3_10].fc.freq++;
			else bl_tree[REPZ_11_138].fc.freq++;
			count = 0; prevlen = curlen;
			if (nextlen == 0) { max_count = 138, min_count = 3;}
			else if (curlen == nextlen) {max_count = 6, min_count = 3;} else {max_count = 7, min_count = 4;}
		}
	}

	void ZipDeflate::_zip_tr_flush_block(char* buf, DWORD stored_len, int last)
	{
		DWORD opt_lenb, static_lenb;
		int max_blindex(0);
		if(level > 0)
		{
			build_tree(&l_desc);
			build_tree(&d_desc);
			scan_tree(dyn_ltree, l_desc.max_code);
			scan_tree(dyn_dtree, d_desc.max_code);
			build_tree(&bl_desc);
			for(max_blindex = BL_CODES - 1; max_blindex >= 3; max_blindex--) { if(bl_tree[bl_order[max_blindex]].dl.len != 0) break; }
			opt_len += 3 * (max_blindex + 1) + 5 + 5 + 4;
			opt_lenb = (opt_len + 10) >> 3;
			static_lenb = (static_len + 10) >> 3;
			if(static_lenb <= opt_lenb) opt_lenb = static_lenb;
		}
		else opt_lenb = static_lenb = stored_len + 5;
		if(stored_len+4 <= opt_lenb && buf != (char*)0) _zip_tr_stored_block(buf, stored_len, last);
		else if(static_lenb == opt_lenb)
		{
			send_bits(last + 2, 3);
			compress_block((zip_ct_data*)static_ltree, (zip_ct_data*)static_dtree);
		}
		else
		{
			send_bits(last + 4, 3);
			send_bits(l_desc.max_code - 256, 5);
			send_bits(d_desc.max_code, 5);
			send_bits(max_blindex - 3, 4);
			for(int rank = 0; rank <= max_blindex; rank++) { send_bits(bl_tree[bl_order[rank]].dl.len, 3); }
			send_tree(dyn_ltree, l_desc.max_code);
			send_tree(dyn_dtree, d_desc.max_code);
			compress_block(dyn_ltree, dyn_dtree);
		}
		init_block();
		if(last)
		{
			if(bi_valid > 8) { put_short(bi_buf); }
			else if(bi_valid > 0) { put_byte((BYTE)bi_buf); }
			bi_buf = 0; bi_valid = 0;
		}
	}

	ZipDeflate::zip_block_state ZipDeflate::zip_deflate_slow(int flush)
	{
		DWORD hash_head;
		int bflush;
		while(true)
		{
			if(lookahead < MIN_LOOKAHEAD)
			{
				UINT n, m, more;
				WORD *p;
				UINT wsize(w_size);

				do
				{
					more = (UINT)(window_size - (DWORD)lookahead - (DWORD)strstart);
					if(sizeof(int) <= 2)
					{
						if(more == 0 && strstart == 0 && lookahead == 0) more = wsize;
						else if(more == (UINT)(-1)) more--;
					}
					if(strstart >= wsize + MAX_DIST())
					{
						memcpy(window, window + wsize, (UINT)wsize);
						match_start -= wsize;
						strstart -= wsize;
						block_start -= (long)wsize;
						n = hash_size;
						p = &head[n];
						do { m = *--p; *p = (WORD)(m >= wsize ? m - wsize : 0); } while(--n);
						n = wsize;
						p = &prev[n];
						do { m = *--p; *p = (WORD)(m >= wsize ? m - wsize : 0); } while(--n);
						more += wsize;
					}
					if(strm->avail_in == 0) break;
					n = strm->zip_read_buf(window + strstart + lookahead, more);
					lookahead += n;
					if(lookahead + insert >= MIN_MATCH)
					{
						UINT str = strstart - insert;
						ins_h = window[str];
						UPDATE_HASH(ins_h, window[str + 1]);
						while(insert)
						{
							UPDATE_HASH(ins_h, window[str + MIN_MATCH - 1]);
							prev[str & w_mask] = head[ins_h];
							head[ins_h] = (WORD)str;
							str++;
							insert--;
							if(lookahead + insert < MIN_MATCH) break;
						}
					}
				} while(lookahead < MIN_LOOKAHEAD && strm->avail_in != 0);
				if(high_water < window_size)
				{
					DWORD curr = strstart + (DWORD)(lookahead);
					DWORD init;
					if(high_water < curr)
					{
						init = window_size - curr;
						if(init > WIN_INIT) init = WIN_INIT;
						SSH_MEMZERO(window + curr, (UINT)init);
						high_water = curr + init;
					}
					else if(high_water < (DWORD)curr + WIN_INIT)
					{
						init = (DWORD)curr + WIN_INIT - high_water;
						if(init > window_size - high_water) init = window_size - high_water;
						SSH_MEMZERO(window + high_water, (UINT)init);
						high_water += init;
					}
				}
				if(lookahead == 0) break;
			}
			hash_head = 0;
			if(lookahead >= MIN_MATCH) {INSERT_STRING(strstart, hash_head);}
			prev_length = match_length, prev_match = match_start;
			match_length = MIN_MATCH - 1;
			if(hash_head && prev_length < max_lazy_match && strstart - hash_head <= (UINT)MAX_DIST())
			{
				UINT chain_length(max_chain_length);
				BYTE* scan(window + strstart), *match;
				int len, best_len(prev_length), nice_match(nice_match);
				DWORD cur_match(hash_head), limit(strstart >(DWORD)MAX_DIST() ? strstart - (DWORD)MAX_DIST() : 0);
				WORD* prev(this->prev);
				UINT wmask(this->w_mask);
				BYTE* strend(window + strstart + MAX_MATCH);
				BYTE scan_end1(scan[best_len - 1]), scan_end(scan[best_len]);

				if(prev_length >= good_match) chain_length >>= 2;
				if((UINT)nice_match > lookahead) nice_match = lookahead;
				do
				{
					match = window + cur_match;
					if(match[best_len] != scan_end || match[best_len - 1] != scan_end1 || *match != *scan || *++match != scan[1]) continue;
					scan += 2, match++;
					do {} while(*++scan == *++match && *++scan == *++match && *++scan == *++match && *++scan == *++match && *++scan == *++match && *++scan == *++match && *++scan == *++match && *++scan == *++match && scan < strend);
					len = MAX_MATCH - (int)(strend - scan);
					scan = strend - MAX_MATCH;
					if(len > best_len)
					{
						match_start = cur_match;
						best_len = len;
						if(len >= nice_match) break;
						scan_end1 = scan[best_len - 1];
						scan_end = scan[best_len];
					}
				} while((cur_match = prev[cur_match & wmask]) > limit && --chain_length != 0);
				match_length = ((UINT)best_len <= lookahead ? (UINT)best_len : lookahead);
				if(match_length <= 5 && (match_length == MIN_MATCH && strstart - match_start > 4096)) match_length = MIN_MATCH - 1;
			}
			if(prev_length >= MIN_MATCH && match_length <= prev_length)
			{
				UINT max_insert(strstart + lookahead - MIN_MATCH);
				_zip_tr_tally_dist(strstart - 1 - prev_match, prev_length - MIN_MATCH, bflush);
				lookahead -= prev_length - 1;
				prev_length -= 2;
				do {if(++strstart <= max_insert) {INSERT_STRING(strstart, hash_head);}} while (--prev_length != 0);
				match_available = 0;
				match_length = MIN_MATCH - 1;
				strstart++;
				if(bflush) FLUSH_BLOCK(0);
			}
			else if(match_available)
			{
				_zip_tr_tally_lit(window[strstart - 1], bflush);
				if(bflush) {FLUSH_BLOCK_ONLY(0);}
				strstart++;
				lookahead--;
				if(strm->avail_out == 0) return need_more;
			}
			else
			{
				match_available = 1;
				strstart++;
				lookahead--;
			}
		}
		if(match_available)
		{
			_zip_tr_tally_lit(window[strstart - 1], bflush);
			match_available = 0;
		}
		insert = strstart < MIN_MATCH - 1 ? strstart : MIN_MATCH - 1;
		if(flush == ZIP_FINISH) {FLUSH_BLOCK(1); return finish_done;}
		if(last_lit) FLUSH_BLOCK(0);
		return block_done;
	}
}
