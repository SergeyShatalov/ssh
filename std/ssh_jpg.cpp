
#include "stdafx.h"
#include "ssh_jpg.h"

namespace Jpeg
{
	void Decoder::_DecodeLength()
	{
		if(ctx.size < 2) throw;
		ctx.length = _Decode16(ctx.pos);
		if(ctx.length > ctx.size) throw;
		_Skip(2);
	}

	void Decoder::_DecodeDRI()
	{
		_DecodeLength();
		if(ctx.length < 2) throw;
		ctx.rstinterval = _Decode16(ctx.pos);
		_Skip(ctx.length);
	}

	void Decoder::_RowIDCT(int* blk)
	{
		int x0, x1, x2, x3, x4, x5, x6, x7, x8;
		if(!((x1 = blk[4] << 11) | (x2 = blk[6]) | (x3 = blk[2]) | (x4 = blk[1]) | (x5 = blk[7]) | (x6 = blk[5]) | (x7 = blk[3])))
		{
			blk[0] = blk[1] = blk[2] = blk[3] = blk[4] = blk[5] = blk[6] = blk[7] = blk[0] << 3;
			return;
		}
		x0 = (blk[0] << 11) + 128;
		x8 = W7 * (x4 + x5);
		x4 = x8 + (W1 - W7) * x4;
		x5 = x8 - (W1 + W7) * x5;
		x8 = W3 * (x6 + x7);
		x6 = x8 - (W3 - W5) * x6;
		x7 = x8 - (W3 + W5) * x7;
		x8 = x0 + x1;
		x0 -= x1;
		x1 = W6 * (x3 + x2);
		x2 = x1 - (W2 + W6) * x2;
		x3 = x1 + (W2 - W6) * x3;
		x1 = x4 + x6;
		x4 -= x6;
		x6 = x5 + x7;
		x5 -= x7;
		x7 = x8 + x3;
		x8 -= x3;
		x3 = x0 + x2;
		x0 -= x2;
		x2 = (181 * (x4 + x5) + 128) >> 8;
		x4 = (181 * (x4 - x5) + 128) >> 8;
		blk[0] = (x7 + x1) >> 8;
		blk[1] = (x3 + x2) >> 8;
		blk[2] = (x0 + x4) >> 8;
		blk[3] = (x8 + x6) >> 8;
		blk[4] = (x8 - x6) >> 8;
		blk[5] = (x0 - x4) >> 8;
		blk[6] = (x3 - x2) >> 8;
		blk[7] = (x7 - x1) >> 8;
	}

	void Decoder::_ColIDCT(const int* blk, unsigned char *out, int stride)
	{
		int x0, x1, x2, x3, x4, x5, x6, x7, x8;
		if(!((x1 = blk[8 * 4] << 8) | (x2 = blk[8 * 6]) | (x3 = blk[8 * 2]) | (x4 = blk[8 * 1]) | (x5 = blk[8 * 7]) | (x6 = blk[8 * 5]) | (x7 = blk[8 * 3])))
		{
			x1 = _Clip(((blk[0] + 32) >> 6) + 128);
			for(x0 = 8; x0; --x0) {*out = (unsigned char)x1; out += stride;}
			return;
		}
		x0 = (blk[0] << 8) + 8192;
		x8 = W7 * (x4 + x5) + 4;
		x4 = (x8 + (W1 - W7) * x4) >> 3;
		x5 = (x8 - (W1 + W7) * x5) >> 3;
		x8 = W3 * (x6 + x7) + 4;
		x6 = (x8 - (W3 - W5) * x6) >> 3;
		x7 = (x8 - (W3 + W5) * x7) >> 3;
		x8 = x0 + x1;
		x0 -= x1;
		x1 = W6 * (x3 + x2) + 4;
		x2 = (x1 - (W2 + W6) * x2) >> 3;
		x3 = (x1 + (W2 - W6) * x3) >> 3;
		x1 = x4 + x6;
		x4 -= x6;
		x6 = x5 + x7;
		x5 -= x7;
		x7 = x8 + x3;
		x8 -= x3;
		x3 = x0 + x2;
		x0 -= x2;
		x2 = (181 * (x4 + x5) + 128) >> 8;
		x4 = (181 * (x4 - x5) + 128) >> 8;
		*out = _Clip(((x7 + x1) >> 14) + 128);  out += stride;
		*out = _Clip(((x3 + x2) >> 14) + 128);  out += stride;
		*out = _Clip(((x0 + x4) >> 14) + 128);  out += stride;
		*out = _Clip(((x8 + x6) >> 14) + 128);  out += stride;
		*out = _Clip(((x8 - x6) >> 14) + 128);  out += stride;
		*out = _Clip(((x0 - x4) >> 14) + 128);  out += stride;
		*out = _Clip(((x3 - x2) >> 14) + 128);  out += stride;
		*out = _Clip(((x7 - x1) >> 14) + 128);
	}

	int Decoder::_ShowBits(int bits)
	{
		unsigned char newbyte;
		if(!bits) return 0;
		while(ctx.bufbits < bits)
		{
			if(ctx.size <= 0)
			{
				ctx.buf = (ctx.buf << 8) | 0xFF;
				ctx.bufbits += 8;
				continue;
			}
			newbyte = *ctx.pos++;
			ctx.size--;
			ctx.bufbits += 8;
			ctx.buf = (ctx.buf << 8) | newbyte;
			if(newbyte == 0xFF)
			{
				if(ctx.size)
				{
					unsigned char marker = *ctx.pos++;
					ctx.size--;
					switch(marker)
					{
						case 0:    break;
						case 0xD9: ctx.size = 0; break;
						default: if((marker & 0xF8) != 0xD0) throw; else {ctx.buf = (ctx.buf << 8) | marker; ctx.bufbits += 8;}
					}
				}
				else throw;
			}
		}
		return (ctx.buf >> (ctx.bufbits - bits)) & ((1 << bits) - 1);
	}

	void Decoder::_DecodeSOF()
	{
		int i, ssxmax = 0, ssymax = 0;
		Component* c;
		_DecodeLength();
		if(ctx.length < 9) throw;
		if(ctx.pos[0] != 8) throw;
		ctx.height = _Decode16(ctx.pos + 1);
		ctx.width = _Decode16(ctx.pos + 3);
		ctx.ncomp = ctx.pos[5];
		_Skip(6);
		if(ctx.ncomp != 1 && ctx.ncomp != 3) throw;
		if(ctx.length < (ctx.ncomp * 3)) throw;
		for(i = 0, c = ctx.comp; i < ctx.ncomp; ++i, ++c)
		{
			c->cid = ctx.pos[0];
			if(!(c->ssx = ctx.pos[1] >> 4)) throw;
			if(c->ssx & (c->ssx - 1)) throw;
			if(!(c->ssy = ctx.pos[1] & 15)) throw;
			if(c->ssy & (c->ssy - 1)) throw;
			if((c->qtsel = ctx.pos[2]) & 0xFC) throw;
			_Skip(3);
			ctx.qtused |= 1 << c->qtsel;
			if(c->ssx > ssxmax) ssxmax = c->ssx;
			if(c->ssy > ssymax) ssymax = c->ssy;
		}
		ctx.mbsizex = ssxmax << 3;
		ctx.mbsizey = ssymax << 3;
		ctx.mbwidth = (ctx.width + ctx.mbsizex - 1) / ctx.mbsizex;
		ctx.mbheight = (ctx.height + ctx.mbsizey - 1) / ctx.mbsizey;
		for(i = 0, c = ctx.comp; i < ctx.ncomp; ++i, ++c)
		{
			c->width = (ctx.width * c->ssx + ssxmax - 1) / ssxmax;
			c->stride = (c->width + 7) & 0x7FFFFFF8;
			c->height = (ctx.height * c->ssy + ssymax - 1) / ssymax;
			c->stride = ctx.mbwidth * ctx.mbsizex * c->ssx / ssxmax;
			if(((c->width < 3) && (c->ssx != ssxmax)) || ((c->height < 3) && (c->ssy != ssymax))) throw;
			c->pixels = (unsigned char*)malloc(c->stride * (ctx.mbheight * ctx.mbsizey * c->ssy / ssymax));
		}
		if(ctx.ncomp == 3) ctx.rgb = (unsigned char*)malloc(ctx.width * ctx.height * ctx.ncomp);
		_Skip(ctx.length);
	}

	void Decoder::_DecodeDHT()
	{
		int codelen, currcnt, remain, spread, i, j;
		VlcCode *vlc;
		unsigned char counts[16];
		_DecodeLength();
		while(ctx.length >= 17)
		{
			i = ctx.pos[0];
			if(i & 0xEC) throw;
			if(i & 0x02) throw;
			i = (i | (i >> 3)) & 3;  // combined DC/AC + tableid value
			for(codelen = 1; codelen <= 16; ++codelen) counts[codelen - 1] = ctx.pos[codelen];
			_Skip(17);
			vlc = &ctx.vlctab[i][0];
			remain = spread = 65536;
			for(codelen = 1; codelen <= 16; ++codelen)
			{
				spread >>= 1;
				currcnt = counts[codelen - 1];
				if(!currcnt) continue;
				if(ctx.length < currcnt) throw;
				remain -= currcnt << (16 - codelen);
				if(remain < 0) throw;
				for(i = 0; i < currcnt; ++i)
				{
					register unsigned char code = ctx.pos[i];
					for(j = spread; j; --j)
					{
						vlc->bits = (unsigned char)codelen;
						vlc->code = code;
						++vlc;
					}
				}
				_Skip(currcnt);
			}
			while(remain--)
			{
				vlc->bits = 0;
				++vlc;
			}
		}
		if(ctx.length) throw;
	}

	void Decoder::_DecodeDQT(void)
	{
		int i;
		unsigned char *t;
		_DecodeLength();
		while(ctx.length >= 65)
		{
			i = ctx.pos[0];
			if(i & 0xFC) throw;
			ctx.qtavail |= 1 << i;
			t = &ctx.qtab[i][0];
			for(i = 0; i < 64; ++i) t[i] = ctx.pos[i + 1];
			_Skip(65);
		}
		if(ctx.length) throw;
	}

	int Decoder::_GetVLC(VlcCode* vlc, unsigned char* code)
	{
		int value = _ShowBits(16);
		int bits = vlc[value].bits;
		if(!bits) throw;
		_SkipBits(bits);
		value = vlc[value].code;
		if(code) *code = (unsigned char)value;
		bits = value & 15;
		if(!bits) return 0;
		value = _GetBits(bits);
		if(value < (1 << (bits - 1))) value += ((-1) << bits) + 1;
		return value;
	}

	void Decoder::_DecodeBlock(Component* c, unsigned char* out)
	{
		unsigned char code;
		int value, coef = 0;
		memset(ctx.block, 0, sizeof(ctx.block));
		c->dcpred += _GetVLC(&ctx.vlctab[c->dctabsel][0], NULL);
		ctx.block[0] = (c->dcpred) * ctx.qtab[c->qtsel][0];
		do
		{
			value = _GetVLC(&ctx.vlctab[c->actabsel][0], &code);
			if(!code) break;  // EOB
			if(!(code & 0x0F) && (code != 0xF0)) throw;
			coef += (code >> 4) + 1;
			if(coef > 63) throw;
			ctx.block[(int)ZZ[coef]] = value * ctx.qtab[c->qtsel][coef];
		} while(coef < 63);
		for(coef = 0; coef < 64; coef += 8) _RowIDCT(&ctx.block[coef]);
		for(coef = 0; coef < 8; ++coef) _ColIDCT(&ctx.block[coef], &out[coef], c->stride);
	}

	void Decoder::_DecodeScan()
	{
		int i, mbx, mby, sbx, sby;
		int rstcount = ctx.rstinterval, nextrst = 0;
		Component* c;
		_DecodeLength();
		if(ctx.length < (4 + 2 * ctx.ncomp)) throw;
		if(ctx.pos[0] != ctx.ncomp) throw;
		_Skip(1);
		for(i = 0, c = ctx.comp; i < ctx.ncomp; ++i, ++c)
		{
			if(ctx.pos[0] != c->cid) throw;
			if(ctx.pos[1] & 0xEE) throw;
			c->dctabsel = ctx.pos[1] >> 4;
			c->actabsel = (ctx.pos[1] & 1) | 2;
			_Skip(2);
		}
		if(ctx.pos[0] || (ctx.pos[1] != 63) || ctx.pos[2]) throw;
		_Skip(ctx.length);
		for(mby = 0; mby < ctx.mbheight; ++mby)
		{
			for(mbx = 0; mbx < ctx.mbwidth; ++mbx)
			{
				for(i = 0, c = ctx.comp; i < ctx.ncomp; ++i, ++c)
				{
					for(sby = 0; sby < c->ssy; ++sby)
					{
						for(sbx = 0; sbx < c->ssx; ++sbx) _DecodeBlock(c, &c->pixels[((mby * c->ssy + sby) * c->stride + mbx * c->ssx + sbx) << 3]);
					}
				}
				if(ctx.rstinterval && !(--rstcount))
				{
					_ByteAlign();
					i = _GetBits(16);
					nextrst = (nextrst + 1) & 7;
					rstcount = ctx.rstinterval;
					for(i = 0; i < 3; ++i) ctx.comp[i].dcpred = 0;
				}
			}
		}
	}

	void Decoder::_UpsampleH(Component* c)
	{
		const int xmax = c->width - 3;
		unsigned char *out, *lin, *lout;
		int x, y;
		out = (unsigned char*)malloc((c->width * c->height) << 1);
		lin = c->pixels;
		lout = out;
		for(y = c->height; y; --y)
		{
			lout[0] = CF(CF2A * lin[0] + CF2B * lin[1]);
			lout[1] = CF(CF3X * lin[0] + CF3Y * lin[1] + CF3Z * lin[2]);
			lout[2] = CF(CF3A * lin[0] + CF3B * lin[1] + CF3C * lin[2]);
			for(x = 0; x < xmax; ++x)
			{
				lout[(x << 1) + 3] = CF(CF4A * lin[x] + CF4B * lin[x + 1] + CF4C * lin[x + 2] + CF4D * lin[x + 3]);
				lout[(x << 1) + 4] = CF(CF4D * lin[x] + CF4C * lin[x + 1] + CF4B * lin[x + 2] + CF4A * lin[x + 3]);
			}
			lin += c->stride;
			lout += c->width << 1;
			lout[-3] = CF(CF3A * lin[-1] + CF3B * lin[-2] + CF3C * lin[-3]);
			lout[-2] = CF(CF3X * lin[-1] + CF3Y * lin[-2] + CF3Z * lin[-3]);
			lout[-1] = CF(CF2A * lin[-1] + CF2B * lin[-2]);
		}
		c->width <<= 1;
		c->stride = c->width;
		free(c->pixels);
		c->pixels = out;
	}

	void Decoder::_UpsampleV(Component* c)
	{
		const int w = c->width, s1 = c->stride, s2 = s1 + s1;
		unsigned char *out, *cin, *cout;
		int x, y;
		out = (unsigned char*)malloc((c->width * c->height) << 1);
		for(x = 0; x < w; ++x)
		{
			cin = &c->pixels[x];
			cout = &out[x];
			*cout = CF(CF2A * cin[0] + CF2B * cin[s1]);  cout += w;
			*cout = CF(CF3X * cin[0] + CF3Y * cin[s1] + CF3Z * cin[s2]);  cout += w;
			*cout = CF(CF3A * cin[0] + CF3B * cin[s1] + CF3C * cin[s2]);  cout += w;
			cin += s1;
			for(y = c->height - 3; y; --y)
			{
				*cout = CF(CF4A * cin[-s1] + CF4B * cin[0] + CF4C * cin[s1] + CF4D * cin[s2]);  cout += w;
				*cout = CF(CF4D * cin[-s1] + CF4C * cin[0] + CF4B * cin[s1] + CF4A * cin[s2]);  cout += w;
				cin += s1;
			}
			cin += s1;
			*cout = CF(CF3A * cin[0] + CF3B * cin[-s1] + CF3C * cin[-s2]);  cout += w;
			*cout = CF(CF3X * cin[0] + CF3Y * cin[-s1] + CF3Z * cin[-s2]);  cout += w;
			*cout = CF(CF2A * cin[0] + CF2B * cin[-s1]);
		}
		c->height <<= 1;
		c->stride = c->width;
		free(c->pixels);
		c->pixels = out;
	}

	void Decoder::_Convert()
	{
		int i;
		Component* c;
		for(i = 0, c = ctx.comp; i < ctx.ncomp; ++i, ++c)
		{
			while((c->width < ctx.width) || (c->height < ctx.height))
			{
				if(c->width < ctx.width) _UpsampleH(c);
				if(c->height < ctx.height) _UpsampleV(c);
			}
			if((c->width < ctx.width) || (c->height < ctx.height)) throw;
		}
		if(ctx.ncomp == 3)
		{
			// convert to RGB
			int x, yy;
			unsigned char *prgb = ctx.rgb;
			const unsigned char *py = ctx.comp[0].pixels;
			const unsigned char *pcb = ctx.comp[1].pixels;
			const unsigned char *pcr = ctx.comp[2].pixels;
			for(yy = ctx.height; yy; --yy)
			{
				for(x = 0; x < ctx.width; ++x)
				{
					register int y = py[x] << 8;
					register int cb = pcb[x] - 128;
					register int cr = pcr[x] - 128;
					*prgb++ = _Clip((y + 359 * cr + 128) >> 8);
					*prgb++ = _Clip((y - 88 * cb - 183 * cr + 128) >> 8);
					*prgb++ = _Clip((y + 454 * cb + 128) >> 8);
				}
				py += ctx.comp[0].stride;
				pcb += ctx.comp[1].stride;
				pcr += ctx.comp[2].stride;
			}
		}
		else if(ctx.comp[0].width != ctx.comp[0].stride)
		{
			// grayscale -> only remove stride
			unsigned char *pin = &ctx.comp[0].pixels[ctx.comp[0].stride];
			unsigned char *pout = &ctx.comp[0].pixels[ctx.comp[0].width];
			int y;
			for(y = ctx.comp[0].height - 1; y; --y)
			{
				memcpy(pout, pin, ctx.comp[0].width);
				pin += ctx.comp[0].stride;
				pout += ctx.comp[0].width;
			}
			ctx.comp[0].stride = ctx.comp[0].width;
		}
	}

	void Decoder::_Decode(const unsigned char* jpeg, const size_t size)
	{
		ctx.pos = (const unsigned char*)jpeg;
		ctx.size = size & 0x7FFFFFFF;
		if(ctx.size < 2) throw;
		if((ctx.pos[0] ^ 0xFF) | (ctx.pos[1] ^ 0xD8)) throw;
		_Skip(2);
		while(true)
		{
			if((ctx.size < 2) || (ctx.pos[0] != 0xFF)) throw;
			_Skip(2);
			switch(ctx.pos[-1])
			{
				case 0xC0: _DecodeSOF();  break;
				case 0xC4: _DecodeDHT();  break;
				case 0xDB: _DecodeDQT();  break;
				case 0xDD: _DecodeDRI();  break;
				case 0xDA: _DecodeScan(); _Convert(); return;
				case 0xFE: _SkipMarker(); break;
				default: if((ctx.pos[-1] & 0xF0) == 0xE0) _SkipMarker(); else throw;
			}
		}
		throw;
	}
}
