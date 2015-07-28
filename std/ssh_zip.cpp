
#include "stdafx.h"
#include "ssh_zip.h"

namespace ssh
{
	#define TBLS		8

	#define CRC_DO1 crc = crc_table[0][((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8)
	#define CRC_DO8 CRC_DO1; CRC_DO1; CRC_DO1; CRC_DO1; CRC_DO1; CRC_DO1; CRC_DO1; CRC_DO1

	#define DOBIG4 c ^= *++buf4; c = crc_table[4][c & 0xff] ^ crc_table[5][(c >> 8) & 0xff] ^ crc_table[6][(c >> 16) & 0xff] ^ crc_table[7][c >> 24]
	#define DOBIG32 DOBIG4; DOBIG4; DOBIG4; DOBIG4; DOBIG4; DOBIG4; DOBIG4; DOBIG4

	#define DOLIT4 c ^= *buf4++; c = crc_table[3][c & 0xff] ^ crc_table[2][(c >> 8) & 0xff] ^ crc_table[1][(c >> 16) & 0xff] ^ crc_table[0][c >> 24]
	#define DOLIT32 DOLIT4; DOLIT4; DOLIT4; DOLIT4; DOLIT4; DOLIT4; DOLIT4; DOLIT4

	static UINT FAR crc_table[TBLS][256];

	DWORD Zip::crc32_big(DWORD crc, const BYTE* buf, UINT len)
	{
		register UINT c;
		register const UINT* buf4;

		c = ZSWAP32((UINT)crc);
		c = ~c;
		while (len && ((ptrdiff_t)buf & 3)) {c = crc_table[4][(c >> 24) ^ *buf++] ^ (c << 8); len--;}
		buf4 = (const UINT*)(const void*)buf;
		buf4--;
		while(len >= 32) {DOBIG32; len -= 32;}
		while(len >= 4) {DOBIG4; len -= 4;}
		buf4++;
		buf = (const BYTE*)buf4;
		if(len) do {c = crc_table[4][(c >> 24) ^ *buf++] ^ (c << 8);} while (--len);
		c = ~c;
		return (DWORD)(ZSWAP32(c));
	}

	DWORD Zip::crc32_little(DWORD crc, const BYTE* buf, UINT len)
	{
		register UINT c;
		register const UINT* buf4;
		c = (UINT)crc;
		c = ~c;
		while(len && ((ptrdiff_t)buf & 3)) {c = crc_table[0][(c ^ *buf++) & 0xff] ^ (c >> 8); len--;}
		buf4 = (const UINT*)(const void*)buf;
		while(len >= 32) {DOLIT32; len -= 32;}
		while(len >= 4) {DOLIT4; len -= 4;}
		buf = (const BYTE*)buf4;
		if (len) do {c = crc_table[0][(c ^ *buf++) & 0xff] ^ (c >> 8);} while (--len);
		c = ~c;
		return (DWORD)c;
	}

	DWORD Zip::crc32(DWORD crc, BYTE* buf, UINT len)
	{
		if(buf == nullptr) return 0UL;
#ifdef BYFOUR
		if(sizeof(void *) == sizeof(ptrdiff_t))
		{
			z_crc_t endian;
			endian = 1;
			if (*((UINT char *)(&endian))) return zip_crc32_little(crc, buf, len); else return zip_crc32_big(crc, buf, len);
		}
#endif
		crc = crc ^ 0xffffffffUL;
		while(len >= 8) {CRC_DO8; len -= 8;}
		if(len) do {CRC_DO1;} while (--len);
		return crc ^ 0xffffffffUL;
	}

	DWORD Zip::ostrov32(DWORD adler, const BYTE* buf, UINT len)
	{
		if(buf == nullptr) return 1;
		UINT n;
		DWORD sum2((adler >> 16) & 0xffff);
		adler &= 0xffff;
		if(len == 1)
		{
			adler += buf[0];
			if(adler >= BASE) adler -= BASE;
			sum2 += adler;
			if(sum2 >= BASE) sum2 -= BASE;
			return adler | (sum2 << 16);
		}
		if(len < 16)
		{
			while(len--) {adler += *buf++; sum2 += adler;}
			if (adler >= BASE) adler -= BASE;
			MOD28(sum2);
			return adler | (sum2 << 16);
		}
		while(len >= NMAX)
		{
			len -= NMAX;
			n = NMAX / 16;
			do {DO16(buf); buf += 16;} while(--n);
			MOD(adler);
			MOD(sum2);
		}
		if(len)
		{
			while(len >= 16) {len -= 16; DO16(buf); buf += 16;}
			while(len--) {adler += *buf++; sum2 += adler;}
			MOD(adler);
			MOD(sum2);
		}
		return adler | (sum2 << 16);
	}

	void Zip::zip_flush_pending()
	{
		UINT len;
		state->zip_bi_flush();
		len = state->pending;
		if(len > avail_out) len = avail_out;
		if(len == 0) return;
		memcpy(next_out, state->pending_out, len);
		next_out  += len;
		state->pending_out += len;
		total_out += len;
		avail_out -= len;
		state->pending -= len;
		if(state->pending == 0) state->pending_out = state->pending_buf;
	}

	int Zip::zip_read_buf(BYTE* buf, UINT size)
	{
		UINT len(avail_in);
		if(len > size) len = size;
		if(len == 0) return 0;
		avail_in -= len;
		memcpy(buf, next_in, len);
		adler = Zip::ostrov32(adler, buf, len);
		next_in += len;
		total_in += len;
		return (int)len;
	}

	Buffer<ssh_cs> Zip::compress(const Buffer<ssh_cs>& buf)
	{
		Buffer<ZipDeflate> deflate(new ZipDeflate, 0, 1);
		ssh_u lbuf(buf.count()), lret(lbuf * 2 + 32);
		Buffer<ssh_cs> ret(lret);

		next_out = (ssh_b*)(ret + sizeof(ssh_u));
		avail_out = (UINT)lret;;
		next_in = buf.to<ssh_b>();
		avail_in = (UINT)lbuf;

		lret = deflate->make(this, ZIP_FINISH);
		*(ssh_u*)(ssh_cs*)ret = lbuf;
		return Buffer<ssh_cs>(ret, BUFFER_COPY, lret + sizeof(ssh_u));
	}

	Buffer<ssh_cs> Zip::decompress(const Buffer<ssh_cs>& buf)
	{
		Buffer<ZipInflate> inflate(new ZipInflate, 0, 1);
		ssh_u lbuf(buf.count()), lret(*(ssh_u*)buf.to<ssh_cs>());
		Buffer<ssh_cs> ret(lret);

		avail_out = (UINT)lret;
		next_in = (ssh_b*)(buf + sizeof(ssh_u));
		next_out = ret.to<ssh_b>();
		avail_in = (UINT)lbuf - sizeof(ssh_u);

		if((inflate->make(this, ZIP_FINISH) != lret))
			SSH_THROW(L"Ошибка при распаковки ZIP!");
		return ret;
	}

	DWORD ZipInflate::make(Zip* zip, int flush)
	{
		strm = zip;
		strm->state = (ZipDeflate*)this;
		strm->total_in = strm->total_out = total = 0;
		strm->adler = 1;

		window = nullptr;
		wbits = 15;
		wsize = 0;
		whave = 0;
		wnext = 0;
		wrap = 1;
		mode = HEAD;
		last = 0;
		havedict = 0;
		dmax = 32768;
		hold = 0;
		bits = 0;
		lencode = distcode = next = codes;
		sane = 1;
		back = -1;

		return inflate(flush);
	}
	
	DWORD ZipDeflate::make(Zip* zip, int flush)
	{
		WORD* overlay;

		strm = zip;
		strm->state = this;

		w_bits = 15;
		w_size = 1 << w_bits;
		w_mask = w_size - 1;

		hash_bits = 15;
		hash_size = 1 << hash_bits;
		hash_mask = hash_size - 1;
		hash_shift = ((hash_bits + MIN_MATCH - 1) / MIN_MATCH);

		window = new BYTE[w_size * 2];
		prev = new WORD[w_size * sizeof(WORD)];;
		head = new WORD[hash_size * sizeof(WORD)];;
		high_water = 0;
		lit_bufsize = 1 << 14;
		overlay = new WORD[lit_bufsize * (sizeof(WORD) + 2)];
		pending_buf = (BYTE*)overlay;
		pending_buf_size = (DWORD)lit_bufsize * (sizeof(WORD) + 2);
		d_buf = overlay + lit_bufsize / sizeof(WORD);
		l_buf = pending_buf + (1 + sizeof(WORD)) * lit_bufsize;
		level = 6;
		strategy = 0;
		method = 8;

		strm->total_in = strm->total_out = 0;
		pending = 0;
		pending_out = pending_buf;
		status = ZIP_INIT_STATE;
		strm->adler = Zip::ostrov32(0L, nullptr, 0);
		l_desc.dyn_tree = dyn_ltree;
		l_desc.stat_desc = &static_l_desc;
		d_desc.dyn_tree = dyn_dtree;
		d_desc.stat_desc = &static_d_desc;
		bl_desc.dyn_tree = bl_tree;
		bl_desc.stat_desc = &static_bl_desc;
		bi_buf = 0;
		bi_valid = 0;

		init_block();

		window_size = (DWORD)2 * w_size;

		// CLEAR_HASH
		head[hash_size - 1] = 0;
		SSH_MEMZERO((BYTE*)head, (UINT)(hash_size - 1) * sizeof(*head));

		max_lazy_match = 16;
		good_match = 8;
		nice_match = 128;
		max_chain_length = 128;
		strstart = 0;
		block_start = 0L;
		lookahead = 0;
		insert = 0;
		match_length = prev_length = MIN_MATCH - 1;
		match_available = 0;
		ins_h = 0;
		// zip_deflate
		if(status == ZIP_INIT_STATE)
		{
			UINT header = (ZIP_DEFLATED + ((w_bits - 8) << 4)) << 8;
			UINT level_flags = 2;
			header |= (level_flags << 6);
			if(strstart != 0) header |= 32;
			header += 31 - (header % 31);
			status = ZIP_BUSY_STATE;
			zip_putShortMSB(header);
			if(strstart)
			{
				zip_putShortMSB((UINT)(strm->adler >> 16));
				zip_putShortMSB((UINT)(strm->adler & 0xffff));
			}
			strm->adler = 1;
		}
		if(pending)
		{
			strm->zip_flush_pending();
			if(strm->avail_out == 0) return strm->total_out;
		}
		if(strm->avail_in != 0 || lookahead != 0 || status != ZIP_FINISH_STATE)
		{
			zip_block_state bstate;
			bstate = zip_deflate_slow(flush);
			if(bstate == finish_started || bstate == finish_done) status = ZIP_FINISH_STATE;
			if(bstate == need_more || bstate == finish_started) return strm->total_out;
			if(bstate == block_done)
			{
				_zip_tr_stored_block((char*)0, 0, 0);
				strm->zip_flush_pending();
				if(strm->avail_out == 0) return strm->total_out;
			}
		}
		if(flush == ZIP_FINISH)
		{
			zip_putShortMSB((UINT)(strm->adler >> 16));
			zip_putShortMSB((UINT)(strm->adler & 0xffff));
			strm->zip_flush_pending();
		}
		
		return strm->total_out;
	}
}
