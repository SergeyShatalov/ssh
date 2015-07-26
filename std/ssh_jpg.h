
#pragma once

namespace Jpeg
{
	class Decoder
	{
	public:
		Decoder(const unsigned char* data, size_t size)
		{
			char temp[64] = {0, 1, 8, 16, 9, 2, 3, 10, 17, 24, 32, 25, 18, 11, 4, 5, 12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6, 7, 14, 21, 28, 35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51, 58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63};
			memcpy(ZZ, temp, sizeof(ZZ));
			memset(&ctx, 0, sizeof(Context));
			_Decode(data, size);
		}
		Decoder::~Decoder()
		{
			for(int i = 0; i < 3; ++i) if(ctx.comp[i].pixels) free((void*)ctx.comp[i].pixels);
			if(ctx.rgb) free((void*)ctx.rgb);
		}
		// ширина
		int width() const {return ctx.width;}
		// высота
		int height() const {return ctx.height;}
		// признак цветности RGB или Ч/Б
		bool IsColor() const {return ctx.ncomp != 1;}
		// буфер изображения
		ssh_b* GetImage() const {return (ctx.ncomp == 1) ? ctx.comp[0].pixels : ctx.rgb;}
		// размер буфера
		ssh_u size() const {return ctx.width * ctx.height * ctx.ncomp;}
	private:
		enum
		{
			W1 = 2841,
			W2 = 2676,
			W3 = 2408,
			W5 = 1609,
			W6 = 1108,
			W7 = 565,
		};

		enum
		{
			CF4A = (-9),
			CF4B = (111),
			CF4C = (29),
			CF4D = (-3),
			CF3A = (28),
			CF3B = (109),
			CF3C = (-9),
			CF3X = (104),
			CF3Y = (27),
			CF3Z = (-3),
			CF2A = (139),
			CF2B = (-11),
		};

		struct VlcCode
		{
			unsigned char bits, code;
		};

		struct Component
		{
			int cid;
			int ssx, ssy;
			int width, height;
			int stride;
			int qtsel;
			int actabsel, dctabsel;
			int dcpred;
			unsigned char *pixels;
		};

		struct Context
		{
			const unsigned char *pos;
			int size;
			int length;
			int width, height;
			int mbwidth, mbheight;
			int mbsizex, mbsizey;
			int ncomp;
			Component comp[3];
			int qtused, qtavail;
			unsigned char qtab[4][64];
			VlcCode vlctab[4][65536];
			int buf, bufbits;
			int block[64];
			int rstinterval;
			unsigned char *rgb;
		};

		Context ctx;
		char ZZ[64];
		void _RowIDCT(int* blk);
		void _ColIDCT(const int* blk, unsigned char *out, int stride);
		void _DecodeDQT();
		void _DecodeDHT();
		void _DecodeSOF();
		void _DecodeScan();
		void _DecodeBlock(Component* c, unsigned char* out);
		int _GetVLC(VlcCode* vlc, unsigned char* code);
		void _UpsampleH(Component* c);
		void _UpsampleV(Component* c);
		void _Convert();
		void _Decode(const unsigned char* jpeg, const size_t size);
		void _DecodeLength();
		void _DecodeDRI();

		void _SkipBits(int bits) { (ctx.bufbits < bits) ? _ShowBits(bits) : ctx.bufbits -= bits; }
		void _ByteAlign(void) {ctx.bufbits &= 0xF8;}
		void _Skip(int count) { ctx.pos += count; ctx.size -= count; ctx.length -= count; if(ctx.size < 0) throw;}
		void _SkipMarker(void) { _DecodeLength(); _Skip(ctx.length); }
		int _ShowBits(int bits);
		int _GetBits(int bits) { int res = _ShowBits(bits); _SkipBits(bits); return res; }
		unsigned short _Decode16(const unsigned char *pos) {return (pos[0] << 8) | pos[1];}
		unsigned char _Clip(const int x) { return (x < 0) ? 0 : ((x > 0xFF) ? 0xFF : (unsigned char)x); }
		unsigned char CF(const int x) {return _Clip((x + 64) >> 7);}
	};
}

