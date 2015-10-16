
/*
*	�����:		������� �. �.
*	�������:	�����������, 20 ������� 2015, 8:00
*	�����������:--
*	��������:	����� �������� �� �������� � ����������� �����������
*/

#pragma once

#include "ssh_arch.h"
#include "ssh_map.h"
#include "ssh_math.h"

namespace ssh
{
	enum class FormatsMap : int
	{
		bc1, bc2, bc3, bgra8, a8, l8, rgba8, rgb8, bgr8, r5g6b5, rgb5a1, rgba4, la8, font, undef
	};

	struct QUAD
	{
		vec4 xy;
		vec4 uv;
		color diff;
	};

	class Image;
	class ImgMap;

	class SSH ImgMod
	{
	public:
		// ���� �������������
		enum class Types : int
		{
			undef,
			flip,			// ����������
			copy,			// �����������
			border,			// ������� �����
			resize,			// �������� ������
			noise,			// ������������ ���
			correct,		// ��������� �� ������ �����������
			//mosaik,			// ������������ �������
			figure,			// ��������� ������
			gradient,		// ����������� �������
			replace,		// ������ �����
			histogramm		// �����������
		};
		// ������� ���������
		enum class Coord : int
		{
			absolute,		// ���������� ��������
			percent			// �������� � ���������
		};
		// ���� ���������
		enum class Addr : int
		{
			lclamp,		// 
			lmirror,	// �������
			lrepeat,	// ������
			nclamp,
			nmirror,
			nrepeat
		};
		// ���� ���������
		enum class Terrain : int
		{
			mountain,		// ��������
			hill,			// ���������
			plain,			// ���������
			valley,			// ������
			island,			// ������
			pit,			// ������(���� ��� � ��� ���)
			pass,			// �������
			raid			// ������
		};
		// ���� ��������
		enum class Ops : int
		{
			none,					// ��� ��������
			perlin,					// ��� �������
			terrain,				// ��� ������������ ���������
			brd_o3d,				// ������� 3� �����
			brd_i3d,				// ��������� 3� �����
			grp,					// ����� ��� ������ ���������
			h_flip,					// �������������� �������
			v_flip,					// ������������ �������
			flip_90,				// ������� �� 90 ��������
			tbl_2d,					// ��������� �����
			tbl_3d,					// ����� 3� �������
			tbl_grp					// ����� ������� ��� ������ ���������
		};
		// �������� �������� ��� ���������
		enum class Pix : int
		{
			add,					// ����������
			sub,					// ���������
			set,					// ���������
			xor,					// ����������� ���
			and,					// ���������� �
			or,						// ���������� ���
			lum,					// ������� ������
			not,					// ���������
			var_alpha,				// ������ �����
			fix_alpha,				// ������������� �����(��������)
			mul,					// ���������
			lum_add,				// ���������� �������� ������
			lum_sub,				// ��������� �������� ������
			norm,					// ������������
			pow2					// ������ �������� �� �������� ������� ������� 2
		};
		// ���� ��������
		enum class Flt : int
		{
			none,			// ��� �������
			sobel,			// ������
			laplacian,		// ������
			prewit,			// ������
			emboss,			// ������
			normal,			// ������������
			hi,				// ��������� ��������
			low,			// ����������
			median,			// ������ ���
			roberts,		// �������
			max,			// ������������
			min,			// �����������
			contrast,		// ��������
			binary,			// �����������
			gamma,			// ����� ���������
			scale_bias		// 
		};
		// ���� �����
		enum class Figures : int
		{
			ellipse,		// ������
			region,			// ������ �����
			rectangle,		// �������������
			tri_u,			// ����������� �����
			tri_d,			// ����������� ����
			tri_r,			// ����������� ������
			tri_l,			// ����������� �����
			pyramid,		// ��������
			sixangle,		// �������������
			eightangle,		// ��������������
			romb,			// ����
			star1,			// ������
			star2,			// ������ ������
			arrow_r,		// ������� ������
			arrow_l,		// ������� �����
			arrow_d,		// ������� ����
			arrow_u,		// ������� �����
			cross_diag,		// ����� �� ���������
			checked,		// ������
			vplz,			// ������������ �������
			hplz,			// �������������� �������
			plus			// ����
		};
		// ���� ������
		enum class Borders : int
		{
			left	= 1,	// �����
			right	= 2,	// ������
			top		= 4,	// ������
			bottom	= 8,	// �����
			all		= 15	// ���
		};
		// ������������
		enum class Histogramms : int
		{
			rgb,
			red,
			green,
			blue,
			rgb_v,
			red_v,
			green_v,
			blue_v
		};
		ImgMod() {}
		// ���������������� ����������� �� xml
		ImgMod(Xml* xml, HXML hroot, Image* img = nullptr) { make(xml, hroot, img); }
		// ������� �����������
		void make(Xml* xml, HXML hroot, Image* img = nullptr);
		// ��������� ����������� ��� �����
		void apply(ImgMap* map);
		// �������������� �� ����������� ������� ��������� � ����������
		static Bar<int> absolute_bar(const Bar<int>& bar, const Range<int>& clip, Coord coord)
		{
			if(coord != Coord::percent) return bar;
			float x(bar.x / 100.0f), y(bar.y / 100.0f), w(bar.w / 100.0f), h(bar.h / 100.0f);
			return Bar<int>((int)(x * clip.w), (int)(y * clip.h), (int)(w * clip.w), (int)(h * clip.h));
		}
		// ���
		Types type = Types::undef;
		// ���� ��������
		Ops type_ops = Ops::none;
		// ���������
		Addr type_address = Addr::lclamp;
		// ������
		Figures type_figure = Figures::star1;
		// ������
		Flt type_filter = Flt::none;
		// ��� ���������
		Terrain type_terrain = Terrain::island;
		// ��� �����������
		Histogramms type_histogramm = Histogramms::rgb;
		// ��� ���������
		Coord type_coord = Coord::absolute;
		// �������� � ���������
		Range<Pix> ops = Range<Pix>(Pix::set, Pix::set);
		// �����
		Range<int> msks = Range<int>(0xff000000, 0xff000000);
		// �������� �����
		Range<int> vals;
		// ����� �������
		Borders sides = Borders::all;
		// ������ �������
		int w_border = 1;
		// ������
		int radius = 1;
		// ����� ����
		int shadow = 0xff000000;
		// ��������� ����������� � �������
		int aspect = 1;
		// �������� �������
		int w_mtx = 3;
		// �������
		float scale = 1.0f;
		// �������� �����
		float alpha = 1.0f;
		// ������ ��������
		Buffer<ssh_cs> rgba;
		// ������ ��� �������
		vec4 flt_vec = vec4(1.0f, 1.0f, 1.0f, 1.0f);
		// ������� �������� ������������
		Bar<int> bar;
		// ��������
		Range<int> rn;
		// ��������
		Range<int> wh = Range<int>(10, 10);
		// �������� ������
		Range<int> wh_cell = Range<int>(1, 1);
		// ���������� ����������
		Range<float> wh_rep = Range<float>(1.0f, 1.0f);
		// �������� ��������� � ��������
		Range<int> array_count = Range<int>(1, 1);
		// ����� ��� ������������ �����������
		Range<int> cols_histogramm;
	};

	class ImgMap
	{
		friend Image;
		friend ImgMod;
	public:
		// �����������
		ImgMap() {}
		ImgMap(const Range<int>& _wh, const Buffer<ssh_cs>& _pix) : ixywh(_wh), pix(_pix), fxywh(0.0f, 0.0f, 1.0f, 1.0f) {}
		// ������� ����, ��� ����� �������� �������
		virtual bool is_font() const { return false; }
		// ��������/�������� ���
		void set_mip(int level, ImgMap* mip) { mips[level] = mip; }
		// ������� ���
		void del_mip(int level) { mips[level] = nullptr; }
		// ���������� ������� ����� � �����������(��� ������)
		void set_bar(const Bar<int>& bar, const Range<int>& r)
		{
			ixywh = bar;
			fxywh.x = (float)bar.x / (float)r.w;
			fxywh.y = (float)bar.y / (float)r.h;
			fxywh.w = (float)bar.w / (float)r.w;
			fxywh.h = (float)bar.h / (float)r.h;
		}
		// ������� ���
		ImgMap* get_mip(int level) { return mips[level]; }
		// ������� ��������
		const Bar<int>& bar() const { return ixywh; }
		// ������� ����� ��������
		const Buffer<ssh_cs>& pixels() const { return pix; }
	protected:
		// �������
		Bar<int> ixywh;
		Bar<float> fxywh;
		// �����
		Buffer<ssh_cs> pix;
		// ��� ������
		Map<ImgMap*, int> mips;
	};

	class ImgTxt : public ImgMap
	{
		friend class Image;
	public:
		enum class Styles : int
		{
			normal, bold, italic
		};
		enum class Aligned : int
		{
			left	= 0x00000000,// ����������� �� ������ ���� (�� ���������)
			right	= 0x00000001,// ����������� �� ������� ����
			h_center= 0x00000002,// ����������� �� ��������������� ������
			top		= 0x00000000,// ����������� �� �������� ���� (�� ���������)
			bottom	= 0x00000004,// ����������� �� ������� ����
			v_center= 0x00000008,// ����������� �� ������������� ������
			brk		= 0x00000010 // ������������ �������� �� ����� � �������� �����
		};
		// �����������
		ImgTxt(ssh_wcs name, ssh_wcs face, int height, int min_ws, const Buffer<Bar<float>>& pos, const Buffer<ssh_w>& remap);
		ImgTxt(ssh_wcs name, ssh_wcs face, int height, int min_ws, const Range<int>& wh, const Buffer<Bar<int>>& pos, const Buffer<ssh_w>& remap, const Buffer<ssh_cs>& pix);
		// ������� ����, ��� ����� �������� �������
		virtual bool is_font() const override { return true; }
		// ���������� ����� �� ����������������� ������
		void draw(ssh_cs* dst, const Pts<int>& pt, const Bar<int>& clip, const Bar<int>& screen) const;
		// ����������� - ������������ ������(���������, ������������)
		int install(Image* img, ssh_wcs text, const Range<int>& clip, Aligned align, ImgMod::Coord coords);
		// ������� ���� ����������������� ������
		const Range<int>& get_clip() const { return clip; }
		// ������� ���������������� �����
		String get_text() const { return text; }
		// ������� ����� ������������ ����������������� ������
		Aligned get_aligned() const { return align; }
		// ������������ ������ ������ �� ����������������� ������
		Buffer<QUAD> make_quads(const Pts<int>& pt, const Bar<int>& clip, const Bar<int>& screen);
		// ������� �����
		Buffer<QUAD> get_quads() const { return quads; }
	protected:
		// ������ ��������
		int height = 0;
		// ����������� ������
		int min_ws = 0;
		// ����� ������ - ��� ���
		Buffer<QUAD> quads;
		// ������ ������� �������� ������
		Buffer<Bar<float>> pos;
		// ������ ������ ����� ��������
		Buffer<ssh_w> remap;
		// ����� ���������� ������
		Buffer<ssh_cs> install_buf;
		// ���������� ��� �������
		String name;
		// �������� ������
		String face;
		// ���������������� �����
		String text;
		// ���� �������� ������
		Range<int> clip;
		// ������������ ������
		Aligned align;
	};

	class SSH ImgCnv
	{
		friend class Image;
	public:
		enum class Flags : int
		{
			upper = 32, right = 16, grey = 8, rgb = 4, idx = 2, rle = 1, null = 0
		};
		enum class Types : int
		{
			tga, bmp, fse, bfs, dds, gif, jpg, psd, png, undef
		};
		struct IMAGE
		{
			IMAGE(const Range<int>& _wh) : wh(_wh), pix(Buffer<ssh_cs>(_wh.w * _wh.h * 4)) {}
			// ��������
			Range<int> wh;
			// ����� ��������
			Buffer<ssh_cs> pix;
		};
		// �����������
		ImgCnv(ssh_wcs path);
		// ����������� ��� �����������
		const IMAGE* enumerate(bool is_begin) const
		{
			static List<IMAGE*>::Node* n(nullptr);
			IMAGE* img(nullptr);
			if(is_begin) n = imgs.root();
			if(n) { img = n->value; n = n->next; }
			return img;
		}
	protected:
		// ������������ ���� ������� BMP
		void makeBMP(ssh_cs* buf);
		// ������������ ���� ������� TGA
		void makeTGA(ssh_cs* buf);
		// ������������ ���� ������� JPG
		void makeJPG(ssh_cs* buf, ssh_u sz);
		// ������������ ���� ������� DDS
		void makeDDS(ssh_cs* buf, int layer, int mip);
		// ������������ ���� ������� FSE
		void makeFSE(ssh_cs* buf);
		// ������������ ���� ������� BFS (R-TYPE)
		void makeBFS(ssh_cs* buf);
		// ������������ ���� ������� GIF
		void makeGIF(ssh_cs* buf);
		// ������������ ���� ������� PSD
		void makePSD(ssh_cs* buf);
		// ������������ ���� ������� PNG
		void makePNG(ssh_cs* buf) {}
		// ������ �����������
		List<IMAGE*> imgs;
	};

	class SSH Image : public Resource
	{
		SSH_DYNCREATE(Image);
	public:
		// �������� ��� ��������
		enum class Cmds : int
		{
			none, modify, open, save, duplicate, font, empty, remove, draw, make
		};

		enum class TypesMap : int
		{
			CubeMap, VolumeMap, TextureMap, ArrayMap, AtlasMap
		};
		enum class LayersCube : int
		{
			PositiveX, PositiveY, PositiveZ,
			NegativeX, NegativeY, NegativeZ
		};
		struct HEADER
		{
			int sign = 0x01020304;
			// ��������
			Range<int> wh;
			// ������
			FormatsMap fmt = FormatsMap::bgra8;
			// ���
			TypesMap tp = TypesMap::TextureMap;
			// ���������� ��� �������
			int mip;
			// ���������� �������
			int count;
			// �������������� ��������
			buf_cs tex;
		};
		// ������������
		Image() {}
		Image(ssh_wcs path) { open(path); }
		Image(TypesMap _tp, FormatsMap _fmt, int width = 0, int height = 0);
		// ��������/���������� �����
		int set_map(ssh_wcs path, int layer, int mip = 0);
		// ��������/���������� ������ �����
		ImgMap* set_empty(const Range<int>& wh, int layer, int mip = 0);
		// ��������/���������� �����
		ImgTxt* set_font(ssh_wcs name, ssh_wcs face, ssh_w* groups, int height, int layer);
		// ������� �����
		void del_map(int layer, int mip = 0);
		// ����� ���������� �������
		void flush();
		// ������� �����
		ImgMap* get_map(int layer, int mip = 0);
		// ������� �������� �����
		ImgMap* duplicate(int nlayer, int nmip, int olayer, int omip);
		// ������� �����������
		Buffer<ssh_cs> histogramm(int layer, int mip, const Range<int>& wh, ImgMod::Histogramms type, const color& bkg, const color& frg);
		// ������������
		Buffer<ssh_cs> make(ImgMap* map = nullptr);
		// ������� ������ ����
		auto root() const { return maps.root(); }
		// ������� ���������� �����
		int layers() const { return (int)maps.count(); }
		// ������� �������� ���������� ��� �������
		int get_mips(const ImgMap* map = nullptr, Range<int>* wh = nullptr);
		// ������� ������
		FormatsMap format() const { return head.fmt; }
		// ������� ���
		TypesMap type() const { return head.tp; }
		// ������� �������� ��������
		Range<int> range(const ImgMap* map = nullptr);
		// ���������
		virtual void save(ssh_wcs path, bool is_xml) override;
		// �������� ����� � ���� � ������������ �������
		void save(ssh_wcs path, ImgCnv::Types type, FormatsMap fmt, int layer, int mip = 0);
		// ������� ����� ��� �������������
		virtual SCHEME* get_scheme() const override { return nullptr; }
	protected:
		// ����������
		virtual ~Image() {}
		// ��������� �����
		void set_map(ImgMap* map, int layer, int mip);
		// �������� �������
		bool packed_atlas(Range<int>& rn, int offsXY = 1);
		// ������� ���������� �����, � ����������� �� ���� �����������
		int count_layers_from_type() const;
		// ������������
		virtual void make(const Buffer<ssh_cs>& buf) override;
		// �������� ���� ������� BMP
		void saveBMP(File* f, const Range<int>& wh, const Buffer<ssh_cs>& pix, FormatsMap fmt);
		// �������� ���� ������� TGA
		void saveTGA(File* f, const Range<int>& wh, const Buffer<ssh_cs>& pix, FormatsMap fmt);
		// �������� ���� ������� FSE
		void saveFSE(File* f, const Range<int>& wh, const Buffer<ssh_cs>& pix, FormatsMap fmt);
		// �������� ���� ������� BFS
		void saveBFS(File* f, const Range<int>& wh, ssh_cs* pix, FormatsMap fmt);
		// �������� ���� ������� DDS
		void saveDDS(File* f, ImgMap* map, FormatsMap fmt);
		// �����
		Map<ImgMap*, int> maps;
		// ���������
		HEADER head;
		// ������������ ���� �����
		//QUAD quad(int layer, const Pts<int>& pt, const Bar<int>& clip, const Range<int>& screen, const color& col);
	};

	extern "C"
	{
		int asm_ssh_compute_width_wchar(void* ptex);
		int asm_ssh_compute_fmt_size(int width, int height, FormatsMap fmt, int* is_limit = nullptr);
		void asm_ssh_copy_wchar(const Bar<int>& pos, ssh_u height, void* dst, void* src, ssh_u pitch_dst);
		void asm_ssh_cnv(FormatsMap fmt, const Range<int>& wh, void* dst, void* src, int is);
		void asm_ssh_h_flip(const Bar<int>& bar, const Range<int>& wh, void* buf);
		void asm_ssh_v_flip(const Bar<int>& bar, const Range<int>& wh, void* buf);
		void asm_ssh_unpack_bmp(ssh_u pitch, void* pal, void* dst, void* src, ssh_u _4bit);
		void asm_ssh_unpack_psd(ssh_u w, ssh_u h, void* dst, void* src, ssh_u channel_count, ssh_u compression, ssh_u pal);
		void asm_ssh_indexed_psd(ssh_u sz, void* pal, void* src);
		void asm_ssh_unpack_tga(const Range<int>& wh, void* pal, void* dst, void* src, int bpp, int flags);
		void asm_ssh_unpack_gif(int iTrans, void* pal, void* dst, void* src, void* stk);
		void asm_ssh_copy(const Bar<int>& src_bar, const Range<int>& src_wh, void* src, void* dst, const Bar<int>& dst_bar, const Range<int>& dst_wh, ImgMod* modify);
		void asm_ssh_replace(const Range<int>& vals, const Range<int>& msks, void* pix, const Range<int>& clip);
		void asm_ssh_figure(const Bar<int>& bar, const Range<int>& clip, void* pix, ImgMod* modify);
		void asm_ssh_gradient(const Bar<int>& bar, const Range<int>& clip, void* pix, ImgMod* modify);
		void asm_ssh_histogramm(const Range<int>& tmp, ImgMod* modify, void* buf);
		void asm_ssh_correct(const Bar<int>& bar, const Range<int>& clip, void* pix, ImgMod::Histogramms type);
		void asm_ssh_noise_perlin(const Range<int>& clip, int vals, void* pix, float scale);
		void asm_ssh_noise_terrain(const Bar<int>& bar, const Range<int>& clip, void* pix, ImgMod* modify);
		void asm_ssh_flip_90(const Range<int>& clip, void* dst, void* pix);
		void asm_ssh_border3d(const Bar<int>& bar, const Range<int>& clip, void* pix, ImgMod* modify);
		void asm_ssh_group(const Bar<int>& bar, const Range<int>& clip, void* pix, ImgMod* modify);
		void asm_ssh_table(const Bar<int>& bar, const Range<int>& clip, void* pix, ImgMod* modify);
		void asm_ssh_border2d(const Bar<int>& bar, const Range<int>& clip, void* pix, ImgMod* modify);
		Bar<int>* asm_ssh_get_clipbar(const Bar<int>& bar, Bar<int>* out, const Bar<int>& clip);
		QUAD* asm_ssh_make_quad(const Bar<float>& tex, const Bar<int>& pos, const Bar<int>& clip, const Range<int>& screen, int bgra);
	}
}


/*
extern "C"
{
void	__stdcall	asmMakeQuadsText(const Bar<uint_t>& pos, const Bar<uint_t>& clip, QUAD* quads, BYTE* buffer, const Range<uint_t>& screen, ImgTexture* fnt);
uint_t	__stdcall	asmInstallText(PCC font, const Range<uint_t>& clip, BYTE* buffer, PCC msg, ImgText::Aligned flags, void* cells);
void	__stdcall	asmDrawText(const Bar<uint_t>& bar, const Range<uint_t>& clip, BYTE* pixels, BYTE* text, const Pts<uint_t>& pt);
void	__stdcall	asmMakeMipmapLevel(uint_t width, uint_t height, BYTE* dst);
uint_t	__stdcall	asmGetWidthPictureSymvol(BYTE* src);
uint_t	__stdcall	asmGetHeightPictureSymvol(BYTE* src, uint_t w, uint_t h);
void	__stdcall	asmNoiseTerrain(const Bar<uint_t>& bar, const Range<uint_t>& clip, BYTE* src, const Range<uint_t>& vals, const Range<uint_t>& wh, ImgModify::PixelOperations pixOps, uint_t nRepeat);
void	__stdcall	asmMosaik(const Range<uint_t>& wh, void* cells, BYTE* bgra, String* src, const Range<uint_t>& count, const Range<uint_t>& wh_cells, uint_t rel);
Range<uint_t>*	__stdcall	asmGetRangeText();
};
*/
