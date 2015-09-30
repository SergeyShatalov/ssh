
#include "stdafx.h"
#include "ssh_zip.h"

namespace ssh
{
	void ZipInflate::inflateFast(UINT start)
	{
		BYTE* in, *last, *out, *beg, *end, *window, *from;
		UINT wsize, whave, wnext, bits, lmask, dmask, op, len, dist;
		DWORD hold;
		code const* lcode;
		code const* dcode;
		code here;

		in = strm->next_in - OFF;
		last = in + (strm->avail_in - 5);
		out = strm->next_out - OFF;
		beg = out - (start - strm->avail_out);
		end = out + (strm->avail_out - 257);
		wsize = this->wsize;
		whave = this->whave;
		wnext = this->wnext;
		window = this->window;
		hold = this->hold;
		bits = this->bits;
		lcode = this->lencode;
		dcode = this->distcode;
		lmask = (1U << this->lenbits) - 1;
		dmask = (1U << this->distbits) - 1;
		do
		{
			if(bits < 15)
			{
				hold += (DWORD)(PUP(in)) << bits;
				bits += 8;
				hold += (DWORD)(PUP(in)) << bits;
				bits += 8;
			}
			here = lcode[hold & lmask];
dolen:
			op = (UINT)(here.bits);
			hold >>= op;
			bits -= op;
			op = (UINT)(here.op);
			if(op == 0) {PUP(out) = (BYTE)(here.val);}
			else if (op & 16)
			{
				len = (UINT)(here.val);
				op &= 15;
				if(op)
				{
					if(bits < op) {hold += (DWORD)(PUP(in)) << bits; bits += 8;}
					len += (UINT)hold & ((1U << op) - 1);
					hold >>= op;
					bits -= op;
				}
				if(bits < 15)
				{
					hold += (DWORD)(PUP(in)) << bits; bits += 8;
					hold += (DWORD)(PUP(in)) << bits; bits += 8;
				}
				here = dcode[hold & dmask];
dodist:
				op = (UINT)(here.bits);
				hold >>= op;
				bits -= op;
				op = (UINT)(here.op);
				if(op & 16)
				{
					dist = (UINT)(here.val);
					op &= 15;
					if (bits < op)
					{
						hold += (DWORD)(PUP(in)) << bits;
						bits += 8;
						if(bits < op) {hold += (DWORD)(PUP(in)) << bits; bits += 8;}
					}
					dist += (UINT)hold & ((1U << op) - 1);
					hold >>= op;
					bits -= op;
					op = (UINT)(out - beg);
					if(dist > op)
					{
						op = dist - op;
						if(op > whave) {if(sane) {mode = BAD; break;}}
						from = window - OFF;
						if(wnext == 0)
						{
							from += wsize - op;
							if(op < len)
							{
								len -= op;
								do {PUP(out) = PUP(from);} while (--op);
								from = out - dist;
							}
						}
						else if(wnext < op)
						{
							from += wsize + wnext - op;
							op -= wnext;
							if(op < len)
							{
								len -= op; do {PUP(out) = PUP(from);} while (--op);
								from = window - OFF;
								if(wnext < len) {op = wnext; len -= op; do {PUP(out) = PUP(from);} while (--op); from = out - dist;}
							}
						}
						else
						{
							from += wnext - op;
							if(op < len) {len -= op; do {PUP(out) = PUP(from);} while (--op); from = out - dist;}
						}
						while(len > 2)
						{
							PUP(out) = PUP(from);
							PUP(out) = PUP(from);
							PUP(out) = PUP(from);
							len -= 3;
						}
						if(len) {PUP(out) = PUP(from); if(len > 1) PUP(out) = PUP(from);}
					}
					else
					{
						from = out - dist;
						do
						{
							PUP(out) = PUP(from);
							PUP(out) = PUP(from);
							PUP(out) = PUP(from);
							len -= 3;
						} while (len > 2);
						if(len) {PUP(out) = PUP(from); if(len > 1) PUP(out) = PUP(from);}
					}
				}
				else if((op & 64) == 0) {here = dcode[here.val + (hold & ((1U << op) - 1))]; goto dodist;}
				else {mode = BAD; break;}
			}
			else if((op & 64) == 0) {here = lcode[here.val + (hold & ((1U << op) - 1))]; goto dolen;}
			else if(op & 32) {mode = TYPE; break;}
			else {mode = BAD; break;}
		} while(in < last && out < end);
		len = bits >> 3;
		in -= len;
		bits -= len << 3;
		hold &= (1U << bits) - 1;
		strm->next_in = in + OFF;
		strm->next_out = out + OFF;
		strm->avail_in = (UINT)(in < last ? 5 + (last - in) : 5 - (in - last));
		strm->avail_out = (UINT)(out < end ? 257 + (end - out) : 257 - (out - end));
		this->hold = hold;
		this->bits = bits;
	}

	void ZipInflate::updateWindow(UINT out)
	{
		UINT copy, dist;

		if(window == nullptr) {window = new BYTE[65536];}
		if(wsize == 0) {wsize = 1U << wbits; wnext = 0; whave = 0;}
		copy = out - strm->avail_out;
		if(copy >= wsize) {memcpy(window, strm->next_out - wsize, wsize); wnext = 0; whave = wsize;}
		else
		{
			dist = wsize - wnext;
			if(dist > copy) dist = copy;
			memcpy(window + wnext, strm->next_out - copy, dist);
			copy -= dist;
			if(copy) {memcpy(window, strm->next_out - copy, copy); wnext = copy; whave = wsize;}
			else
			{
				wnext += dist;
				if(wnext == wsize) wnext = 0;
				if(whave < wsize) whave += dist;
			}
		}
	}

	int ZipInflate::inflateTable(codetype type, WORD* lens, UINT codes, code** table, UINT* bits, WORD* work)
	{
		int left, end;
		code* next, here;
		UINT len, sym, min, max, root, curr, drop, used, huff, incr, fill, low, mask;
		const WORD* base, *extra;
		WORD count[MAXBITS + 1], offs[MAXBITS + 1];
		static const WORD lbase[31] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0};
		static const WORD lext[31] = {16, 16, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17, 18, 18, 18, 18, 19, 19, 19, 19, 20, 20, 20, 20, 21, 21, 21, 21, 16, 78, 68};
		static const WORD dbase[32] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577, 0, 0};
		static const WORD dext[32] = {16, 16, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 64, 64};

		for(len = 0; len <= MAXBITS; len++) count[len] = 0;
		for(sym = 0; sym < codes; sym++) count[lens[sym]]++;
		root = *bits;
		for(max = MAXBITS; max >= 1; max--) {if(count[max] != 0) break;}
		if(root > max) root = max;
		if(max == 0)
		{
			here.op = 64;
			here.bits = 1;
			here.val = 0;
			*(*table)++ = here;
			*(*table)++ = here;
			*bits = 1;
			return 0;
		}
		for(min = 1; min < max; min++) {if(count[min] != 0) break;}
		if(root < min) root = min;
		left = 1;
		for(len = 1; len <= MAXBITS; len++)
		{
			left <<= 1;
			left -= count[len];
			if (left < 0) return -1;
		}
		if(left > 0 && (type == CODES || max != 1)) return -1;
		offs[1] = 0;
		for(len = 1; len < MAXBITS; len++) offs[len + 1] = offs[len] + count[len];
		for(sym = 0; sym < codes; sym++) {if (lens[sym] != 0) work[offs[lens[sym]]++] = (WORD)sym;}
		switch(type)
		{
			case CODES: base = extra = work; end = 19; break;
			case LENS: base = lbase; base -= 257; extra = lext; extra -= 257; end = 256; break;
			default: base = dbase; extra = dext; end = -1;
		}
		huff = 0;
		sym = 0;
		len = min;
		next = *table;
		curr = root;
		drop = 0;
		low = (UINT)(-1);
		used = 1U << root;
		mask = used - 1;
		if((type == LENS && used >= ENOUGH_LENS) || (type == DISTS && used >= ENOUGH_DISTS)) return 1;
		for(;;)
		{
			here.bits = (BYTE)(len - drop);
			if((int)(work[sym]) < end) {here.op = (BYTE)0; here.val = work[sym];}
			else if((int)(work[sym]) > end) {here.op = (BYTE)(extra[work[sym]]); here.val = base[work[sym]];}
			else {here.op = (BYTE)(32 + 64); here.val = 0;}
			incr = 1U << (len - drop);
			fill = 1U << curr;
			min = fill;
			do {fill -= incr; next[(huff >> drop) + fill] = here; } while (fill != 0);
			incr = 1U << (len - 1);
			while (huff & incr) incr >>= 1;
			if(incr != 0) {huff &= incr - 1; huff += incr;} else huff = 0;
			sym++;
			if(--(count[len]) == 0) {if (len == max) break; len = lens[work[sym]];}
			if(len > root && (huff & mask) != low)
			{
				if(drop == 0) drop = root;
				next += min;
				curr = len - drop;
				left = (int)(1 << curr);
				while(curr + drop < max)
				{
					left -= count[curr + drop];
					if(left <= 0) break;
					curr++;
					left <<= 1;
				}
				used += 1U << curr;
				if ((type == LENS && used >= ENOUGH_LENS) || (type == DISTS && used >= ENOUGH_DISTS)) return 1;
				low = huff & mask;
				(*table)[low].op = (BYTE)curr;
				(*table)[low].bits = (BYTE)root;
				(*table)[low].val = (WORD)(next - *table);
			}
		}
		if(huff != 0)
		{
			here.op = 64;
			here.bits = (len - drop);
			here.val = 0;
			next[huff] = here;
		}
		*table += used;
		*bits = root;
		return 0;
	}

	void ZipInflate::fixedtables()
	{
		static const code lenfix[512] =
		{
			{96,7,0},{0,8,80},{0,8,16},{20,8,115},{18,7,31},{0,8,112},{0,8,48},
			{0,9,192},{16,7,10},{0,8,96},{0,8,32},{0,9,160},{0,8,0},{0,8,128},
			{0,8,64},{0,9,224},{16,7,6},{0,8,88},{0,8,24},{0,9,144},{19,7,59},
			{0,8,120},{0,8,56},{0,9,208},{17,7,17},{0,8,104},{0,8,40},{0,9,176},
			{0,8,8},{0,8,136},{0,8,72},{0,9,240},{16,7,4},{0,8,84},{0,8,20},
			{21,8,227},{19,7,43},{0,8,116},{0,8,52},{0,9,200},{17,7,13},{0,8,100},
			{0,8,36},{0,9,168},{0,8,4},{0,8,132},{0,8,68},{0,9,232},{16,7,8},
			{0,8,92},{0,8,28},{0,9,152},{20,7,83},{0,8,124},{0,8,60},{0,9,216},
			{18,7,23},{0,8,108},{0,8,44},{0,9,184},{0,8,12},{0,8,140},{0,8,76},
			{0,9,248},{16,7,3},{0,8,82},{0,8,18},{21,8,163},{19,7,35},{0,8,114},
			{0,8,50},{0,9,196},{17,7,11},{0,8,98},{0,8,34},{0,9,164},{0,8,2},
			{0,8,130},{0,8,66},{0,9,228},{16,7,7},{0,8,90},{0,8,26},{0,9,148},
			{20,7,67},{0,8,122},{0,8,58},{0,9,212},{18,7,19},{0,8,106},{0,8,42},
			{0,9,180},{0,8,10},{0,8,138},{0,8,74},{0,9,244},{16,7,5},{0,8,86},
			{0,8,22},{64,8,0},{19,7,51},{0,8,118},{0,8,54},{0,9,204},{17,7,15},
			{0,8,102},{0,8,38},{0,9,172},{0,8,6},{0,8,134},{0,8,70},{0,9,236},
			{16,7,9},{0,8,94},{0,8,30},{0,9,156},{20,7,99},{0,8,126},{0,8,62},
			{0,9,220},{18,7,27},{0,8,110},{0,8,46},{0,9,188},{0,8,14},{0,8,142},
			{0,8,78},{0,9,252},{96,7,0},{0,8,81},{0,8,17},{21,8,131},{18,7,31},
			{0,8,113},{0,8,49},{0,9,194},{16,7,10},{0,8,97},{0,8,33},{0,9,162},
			{0,8,1},{0,8,129},{0,8,65},{0,9,226},{16,7,6},{0,8,89},{0,8,25},
			{0,9,146},{19,7,59},{0,8,121},{0,8,57},{0,9,210},{17,7,17},{0,8,105},
			{0,8,41},{0,9,178},{0,8,9},{0,8,137},{0,8,73},{0,9,242},{16,7,4},
			{0,8,85},{0,8,21},{16,8,258},{19,7,43},{0,8,117},{0,8,53},{0,9,202},
			{17,7,13},{0,8,101},{0,8,37},{0,9,170},{0,8,5},{0,8,133},{0,8,69},
			{0,9,234},{16,7,8},{0,8,93},{0,8,29},{0,9,154},{20,7,83},{0,8,125},
			{0,8,61},{0,9,218},{18,7,23},{0,8,109},{0,8,45},{0,9,186},{0,8,13},
			{0,8,141},{0,8,77},{0,9,250},{16,7,3},{0,8,83},{0,8,19},{21,8,195},
			{19,7,35},{0,8,115},{0,8,51},{0,9,198},{17,7,11},{0,8,99},{0,8,35},
			{0,9,166},{0,8,3},{0,8,131},{0,8,67},{0,9,230},{16,7,7},{0,8,91},
			{0,8,27},{0,9,150},{20,7,67},{0,8,123},{0,8,59},{0,9,214},{18,7,19},
			{0,8,107},{0,8,43},{0,9,182},{0,8,11},{0,8,139},{0,8,75},{0,9,246},
			{16,7,5},{0,8,87},{0,8,23},{64,8,0},{19,7,51},{0,8,119},{0,8,55},
			{0,9,206},{17,7,15},{0,8,103},{0,8,39},{0,9,174},{0,8,7},{0,8,135},
			{0,8,71},{0,9,238},{16,7,9},{0,8,95},{0,8,31},{0,9,158},{20,7,99},
			{0,8,127},{0,8,63},{0,9,222},{18,7,27},{0,8,111},{0,8,47},{0,9,190},
			{0,8,15},{0,8,143},{0,8,79},{0,9,254},{96,7,0},{0,8,80},{0,8,16},
			{20,8,115},{18,7,31},{0,8,112},{0,8,48},{0,9,193},{16,7,10},{0,8,96},
			{0,8,32},{0,9,161},{0,8,0},{0,8,128},{0,8,64},{0,9,225},{16,7,6},
			{0,8,88},{0,8,24},{0,9,145},{19,7,59},{0,8,120},{0,8,56},{0,9,209},
			{17,7,17},{0,8,104},{0,8,40},{0,9,177},{0,8,8},{0,8,136},{0,8,72},
			{0,9,241},{16,7,4},{0,8,84},{0,8,20},{21,8,227},{19,7,43},{0,8,116},
			{0,8,52},{0,9,201},{17,7,13},{0,8,100},{0,8,36},{0,9,169},{0,8,4},
			{0,8,132},{0,8,68},{0,9,233},{16,7,8},{0,8,92},{0,8,28},{0,9,153},
			{20,7,83},{0,8,124},{0,8,60},{0,9,217},{18,7,23},{0,8,108},{0,8,44},
			{0,9,185},{0,8,12},{0,8,140},{0,8,76},{0,9,249},{16,7,3},{0,8,82},
			{0,8,18},{21,8,163},{19,7,35},{0,8,114},{0,8,50},{0,9,197},{17,7,11},
			{0,8,98},{0,8,34},{0,9,165},{0,8,2},{0,8,130},{0,8,66},{0,9,229},
			{16,7,7},{0,8,90},{0,8,26},{0,9,149},{20,7,67},{0,8,122},{0,8,58},
			{0,9,213},{18,7,19},{0,8,106},{0,8,42},{0,9,181},{0,8,10},{0,8,138},
			{0,8,74},{0,9,245},{16,7,5},{0,8,86},{0,8,22},{64,8,0},{19,7,51},
			{0,8,118},{0,8,54},{0,9,205},{17,7,15},{0,8,102},{0,8,38},{0,9,173},
			{0,8,6},{0,8,134},{0,8,70},{0,9,237},{16,7,9},{0,8,94},{0,8,30},
			{0,9,157},{20,7,99},{0,8,126},{0,8,62},{0,9,221},{18,7,27},{0,8,110},
			{0,8,46},{0,9,189},{0,8,14},{0,8,142},{0,8,78},{0,9,253},{96,7,0},
			{0,8,81},{0,8,17},{21,8,131},{18,7,31},{0,8,113},{0,8,49},{0,9,195},
			{16,7,10},{0,8,97},{0,8,33},{0,9,163},{0,8,1},{0,8,129},{0,8,65},
			{0,9,227},{16,7,6},{0,8,89},{0,8,25},{0,9,147},{19,7,59},{0,8,121},
			{0,8,57},{0,9,211},{17,7,17},{0,8,105},{0,8,41},{0,9,179},{0,8,9},
			{0,8,137},{0,8,73},{0,9,243},{16,7,4},{0,8,85},{0,8,21},{16,8,258},
			{19,7,43},{0,8,117},{0,8,53},{0,9,203},{17,7,13},{0,8,101},{0,8,37},
			{0,9,171},{0,8,5},{0,8,133},{0,8,69},{0,9,235},{16,7,8},{0,8,93},
			{0,8,29},{0,9,155},{20,7,83},{0,8,125},{0,8,61},{0,9,219},{18,7,23},
			{0,8,109},{0,8,45},{0,9,187},{0,8,13},{0,8,141},{0,8,77},{0,9,251},
			{16,7,3},{0,8,83},{0,8,19},{21,8,195},{19,7,35},{0,8,115},{0,8,51},
			{0,9,199},{17,7,11},{0,8,99},{0,8,35},{0,9,167},{0,8,3},{0,8,131},
			{0,8,67},{0,9,231},{16,7,7},{0,8,91},{0,8,27},{0,9,151},{20,7,67},
			{0,8,123},{0,8,59},{0,9,215},{18,7,19},{0,8,107},{0,8,43},{0,9,183},
			{0,8,11},{0,8,139},{0,8,75},{0,9,247},{16,7,5},{0,8,87},{0,8,23},
			{64,8,0},{19,7,51},{0,8,119},{0,8,55},{0,9,207},{17,7,15},{0,8,103},
			{0,8,39},{0,9,175},{0,8,7},{0,8,135},{0,8,71},{0,9,239},{16,7,9},
			{0,8,95},{0,8,31},{0,9,159},{20,7,99},{0,8,127},{0,8,63},{0,9,223},
			{18,7,27},{0,8,111},{0,8,47},{0,9,191},{0,8,15},{0,8,143},{0,8,79},
			{0,9,255}
		};

		static const code distfix[32] =
		{
			{16,5,1},{23,5,257},{19,5,17},{27,5,4097},{17,5,5},{25,5,1025},
			{21,5,65},{29,5,16385},{16,5,3},{24,5,513},{20,5,33},{28,5,8193},
			{18,5,9},{26,5,2049},{22,5,129},{64,5,0},{16,5,2},{23,5,385},
			{19,5,25},{27,5,6145},{17,5,7},{25,5,1537},{21,5,97},{29,5,24577},
			{16,5,4},{24,5,769},{20,5,49},{28,5,12289},{18,5,13},{26,5,3073},
			{22,5,193},{64,5,0}
		};
		lencode = lenfix;
		lenbits = 9;
		distcode = distfix;
		distbits = 5;
	}

	DWORD ZipInflate::inflate(int flush)
	{
		DWORD _hold;
		BYTE* _next, *_put, *_from;
		UINT _have, _left, _bits, _in, _out, _copy, _len;
		code _here, _last;
		static const WORD order[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
		if(mode == TYPE) mode = TYPEDO;
		LOAD();
		_in = _have; _out = _left;
		while(true)
		{
			switch(mode)
			{
			case HEAD:
				if(wrap == 0) {mode = TYPEDO; break;}
				NEEDBITS(16);
				flags = 0;
				if(!(wrap & 1) || ((BITS(8) << 8) + (_hold >> 8)) % 31) return 0;
				if(BITS(4) != ZIP_DEFLATED) return 0;
				DROPBITS(4);
				_len = BITS(4) + 8;
				if(wbits == 0) wbits = _len; else if(_len > wbits) return 0;
				dmax = 1U << _len;
				strm->adler = check = Zip::ostrov32(0L, nullptr, 0);
				mode = _hold & 0x200 ? DICTID : TYPE;
				INITBITS();
				break;
			case DICTID:
				NEEDBITS(32);
				strm->adler = check = ZSWAP32(_hold);
				INITBITS();
				mode = DICT;
			case DICT:
				if(havedict == 0) {RESTORE(); return 0;}
				strm->adler = check = Zip::ostrov32(0L, nullptr, 0);
				mode = TYPE;
			case TYPE:
				if(flush == ZIP_BLOCK || flush == ZIP_TREES) goto inf_leave;
			case TYPEDO:
				if(last) {BYTEBITS(); mode = CHECK; break;}
				NEEDBITS(3);
				last = BITS(1);
				DROPBITS(1);
				switch(BITS(2))
				{
					case 0: mode = STORED; break;
					case 1: fixedtables(); mode = LEN_; if (flush == ZIP_TREES) {DROPBITS(2); goto inf_leave;} break;
					case 2: mode = TABLE; break;
					case 3: mode = BAD;
				}
				DROPBITS(2);
				break;
			case STORED:
				BYTEBITS();
				NEEDBITS(32);
				if((_hold & 0xffff) != ((_hold >> 16) ^ 0xffff)) return 0;
				length = (UINT)_hold & 0xffff;
				INITBITS();
				mode = COPY_;
				if(flush == ZIP_TREES) goto inf_leave;
			case COPY_:
				mode = COPY;
			case COPY:
				_copy = length;
				if(_copy)
				{
					if(_copy > _have) _copy = _have;
					if(_copy > _left) _copy = _left;
					if(_copy == 0) goto inf_leave;
					memcpy(_put, _next, _copy);
					_have -= _copy; _next += _copy; _left -= _copy; _put += _copy; length -= _copy;
					break;
				}
				mode = TYPE;
				break;
			case TABLE:
				NEEDBITS(14);
				nlen = BITS(5) + 257;
				DROPBITS(5);
				ndist = BITS(5) + 1;
				DROPBITS(5);
				ncode = BITS(4) + 4;
				DROPBITS(4);
				if(nlen > 286 || ndist > 30) return 0;
				have = 0;
				mode = LENLENS;
			case LENLENS:
				while(have < ncode)
				{
					NEEDBITS(3);
					lens[order[have++]] = (WORD)BITS(3);
					DROPBITS(3);
				}
				while(have < 19) lens[order[have++]] = 0;
				next = codes;
				lencode = (code const*)(next);
				lenbits = 7;
				if(inflateTable(CODES, lens, 19, &next, &lenbits, work)) return 0;
				have = 0;
				mode = CODELENS;
			case CODELENS:
				while(have < nlen + ndist)
				{
					while(true)
					{
						_here = lencode[BITS(lenbits)];
						if((UINT)(_here.bits) <= _bits) break;
						PULLBYTE();
					}
					if(_here.val < 16)
					{
						DROPBITS(_here.bits);
						lens[have++] = _here.val;
					}
					else
					{
						if(_here.val == 16)
						{
							NEEDBITS(_here.bits + 2);
							DROPBITS(_here.bits);
							if(have == 0) return 0;
							_len = lens[have - 1];
							_copy = 3 + BITS(2);
							DROPBITS(2);
						}
						else if(_here.val == 17)
						{
							NEEDBITS(_here.bits + 3);
							DROPBITS(_here.bits);
							_len = 0;
							_copy = 3 + BITS(3);
							DROPBITS(3);
						}
						else
						{
							NEEDBITS(_here.bits + 7);
							DROPBITS(_here.bits);
							_len = 0;
							_copy = 11 + BITS(7);
							DROPBITS(7);
						}
						if(have + _copy > nlen + ndist) return 0;
						while(_copy--) lens[have++] = (WORD)_len;
					}
				}
				if(lens[256] == 0) return 0;
				next = codes;
				lencode = (code const*)(next);
				lenbits = 9;
				if(inflateTable(LENS, lens, nlen, &next, &lenbits, work)) return 0;
				distcode = (code const*)next;
				distbits = 6;
				if(inflateTable(DISTS, lens + nlen, ndist, &next, &distbits, work)) return 0;
				mode = LEN_;
				if(flush == ZIP_TREES) goto inf_leave;
			case LEN_:
				mode = LEN;
			case LEN:
				if(_have >= 6 && _left >= 258)
				{
					RESTORE();
					inflateFast(_out);
					LOAD();
					if(mode == TYPE) back = -1;
					break;
				}
				back = 0;
				while(true)
				{
					_here = lencode[BITS(lenbits)];
					if((UINT)(_here.bits) <= _bits) break;
					PULLBYTE();
				}
				if(_here.op && (_here.op & 0xf0) == 0)
				{
					_last = _here;
					while(true)
					{
						_here = lencode[_last.val + (BITS(_last.bits + _last.op) >> _last.bits)];
						if((UINT)(_last.bits + _here.bits) <= _bits) break;
						PULLBYTE();
					}
					DROPBITS(_last.bits);
					back += _last.bits;
				}
				DROPBITS(_here.bits);
				back += _here.bits;
				length = (UINT)_here.val;
				if((int)(_here.op) == 0) {mode = LIT; break;}
				if(_here.op & 32) {back = -1; mode = TYPE; break;}
				if(_here.op & 64) return 0;
				extra = (UINT)(_here.op) & 15;
				mode = LENEXT;
			case LENEXT:
				if(extra)
				{
					NEEDBITS(extra);
					length += BITS(extra);
					DROPBITS(extra);
					back += extra;
				}
				was = length;
				mode = DIST;
			case DIST:
				while(true)
				{
					_here = distcode[BITS(distbits)];
					if((UINT)(_here.bits) <= _bits) break;
					PULLBYTE();
				}
				if((_here.op & 0xf0) == 0)
				{
					_last = _here;
					while(true)
					{
						_here = distcode[_last.val + (BITS(_last.bits + _last.op) >> _last.bits)];
						if((UINT)(_last.bits + _here.bits) <= _bits) break;
						PULLBYTE();
					}
					DROPBITS(_last.bits);
					back += _last.bits;
				}
				DROPBITS(_here.bits);
				back += _here.bits;
				if(_here.op & 64) return 0;
				offset = (UINT)_here.val;
				extra = (UINT)(_here.op) & 15;
				mode = DISTEXT;
			case DISTEXT:
				if(extra)
				{
					NEEDBITS(extra);
					offset += BITS(extra);
					DROPBITS(extra);
					back += extra;
				}
				mode = MATCH;
			case MATCH:
				if(_left == 0) goto inf_leave;
				_copy = _out - _left;
				if(offset > _copy)
				{
					_copy = offset - _copy;
					if(_copy > whave && sane) return 0;
					if(_copy > wnext)
					{
						_copy -= wnext;
						_from = window + (wsize - _copy);
					}
					else _from = window + (wnext - _copy);
					if(_copy > length) _copy = length;
				}
				else
				{
					_from = _put - offset;
					_copy = length;
				}
				if(_copy > _left) _copy = _left;
				_left -= _copy;
				length -= _copy;
				do {*_put++ = *_from++;} while (--_copy);
				if(length == 0) mode = LEN;
				break;
			case LIT:
				if(_left == 0) goto inf_leave;
				*_put++ = (BYTE)(length);
				_left--;
				mode = LEN;
				break;
			case CHECK:
				if(wrap)
				{
					NEEDBITS(32);
					_out -= _left;
					strm->total_out += _out;
					total += _out;
					if(_out) strm->adler = check = UPDATE(check, _put - _out, _out);
					_out = _left;
					if((flags ? _hold : ZSWAP32(_hold)) != check) return 0;
					INITBITS();
				}
				mode = LENGTH;
			case LENGTH:
				if(wrap && flags) {NEEDBITS(32); if(_hold != (total & 0xffffffffUL)) return 0; INITBITS();}
				mode = DONE;
			case DONE:goto inf_leave;
			case BAD:
			default:
				return 0;
			}
		}
inf_leave:
		RESTORE();
		if(wsize || (_out != strm->avail_out && mode < BAD && (mode < CHECK || flush != ZIP_FINISH))) updateWindow(_out);
		_in -= strm->avail_in;
		_out -= strm->avail_out;
		strm->total_in += _in;
		strm->total_out += _out;
		total += _out;
		return (_in == 0 && _out == 0) ? 0 : strm->total_out;
	}
}
