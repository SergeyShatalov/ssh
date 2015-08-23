
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
		bc1, bc2, bc3, bgra8, a8, l8, rgba8, rgb8, bgr8, b5g6r5, bgr5a1, bgra4, font, undef
	};

	struct QUAD
	{
		vec4 xy;
		vec4 uv;
		color diff;
	};

	class SSH Image : public Resource
	{
		SSH_DYNCREATE(Image);
	public:
		class ImgMod
		{
		public:
			// ���� �������������
			enum class Types : int
			{
				flip,			// ����������
				copy,			// �����������
				border,			// ������� �����
				resize,			// �������� ������
				noise,			// ������������ ���
				correct,		// ��������� �� ������ �����������
				mosaik,			// ������������ �������
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
			enum class TerrainTypes : int
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
				brdOut3d,				// ������� 3� �����
				rrdIn3d,				// ��������� 3� �����
				groupBox,				// ����� ��� ������ ���������
				flipH,					// �������������� �������
				flipV,					// ������������ �������
				flip90,					// ������� �� 90 ��������
				table,					// ��������� �����
				table3d,				// ����� 3� �������
				tableGrp				// ����� ������� ��� ������ ���������
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
				alpha,					// ������ �����
				fixed,					// ������������� �����(��������)
				mull,					// ���������
				lumAdd,					// ���������� �������� ������
				lumSub,					// ��������� �������� ������
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
				hi,				// ��������
				low,			// ����������
				median,			// ���������
				roberts,		// �������
				max,			// ������������
				min,			// �����������
				contrast,		// ��������
				binary,			// �����������
				gamma,			// ����� ���������
				scaleBias		// 
			};
			// ���� �����
			enum class Figures : int
			{
				ellipse,		// ������
				region,			// ������ �����
				rectangle,		// �������������
				triangleUp,		// ����������� �����
				triangleDown,	// ����������� ����
				triangleRight,	// ����������� ������
				triangleLeft,	// ����������� �����
				pyramid,		// ��������
				sixangle,		// �������������
				eightangle,		// ��������������
				romb,			// ����
				star1,			// ������
				star2,			// ������ ������
				arrowRight,		// ������� ������
				arrowLeft,		// ������� �����
				arrowDown,		// ������� ����
				arrowUp,		// ������� �����
				cross45,		// ����� �� ���������
				checked,		// ������
				vPlzSlider,		// ������������ �������
				hPlzSlider,		// �������������� �������
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
				valRgb,
				valRed,
				valGreen,
				valBlue
			};
			ImgMod() {}
			~ImgMod() {}
		protected:
		};

		class ImgMap
		{
		public:
			// �����������
			ImgMap(const Range<int>& _wh, const Buffer<ssh_cs>& _pix) : wh(_wh), pix(_pix) { }
			// ��������/�������� ���
			void set_mip(int level, ImgMap* mip) { mips[level] = mip; }
			// ������� ���
			void del_mip(int level) { mips[level] = nullptr; }
			// ������� ���
			const ImgMap* get_mip(int level) { return mips[level]; }
			// ������� ��������
			const Range<int>& range() const { return wh; }
			// ������� ����� ��������
			const Buffer<ssh_cs>& pixels() const { return pix; }
		protected:
			// ��������
			Range<int> wh;
			// �����
			Buffer<ssh_cs> pix;
			// ��� ������
			Map<ImgMap*, int> mips;
		};

		class ImgTxt : public ImgMap
		{
		public:
			enum class Styles : int
			{
				normal, bold, italic
			};
			enum class Aligned : int
			{
				left	= 0x00000000,// ����������� �� ������ ���� (�� ���������)
				right	= 0x00000001,// ����������� �� ������� ����
				hcenter = 0x00000002,// ����������� �� ��������������� ������
				top		= 0x00000000,// ����������� �� �������� ���� (�� ���������)
				bottom	= 0x00000004,// ����������� �� ������� ����
				vcenter = 0x00000008,// ����������� �� ������������� ������
				wbreak	= 0x00000010 // ������������ �������� �� ����� � �������� �����
			};
			// �����������
			ImgTxt(const Range<int>& _wh, const Buffer<ssh_cs>& _pix, int _height, const Buffer<Bar<int>>& _pos);
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
			ssh_w height = 0;
			// ����� ������ - ��� ���
			Buffer<QUAD> quads;
			// ������ ������� �������� ������
			Buffer<Bar<float>> pos;
			// ����� ���������� ������
			Buffer<ssh_cs> install_buf;
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
				tga, bmp, fse, bfs, gif, dds, jpg, undef
			};
			// �����������
			ImgCnv(ssh_wcs path, int layer, int mip);
			// �������� ����� � ���� � ������������ �������
			static void save(ssh_wcs path, ImgMap* map, ImgCnv::Types type);
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
			void makeGIF(ssh_cs* buf, int kadr);
			// �������� ���� ������� BMP
			static void saveBMP(File* f, const Range<int>& wh, const Buffer<ssh_cs>& pix);
			// �������� ���� ������� TGA
			static void saveTGA(File* f, const Range<int>& wh, const Buffer<ssh_cs>& pix);
			// �������� ���� ������� FSE
			static void saveFSE(File* f, const Range<int>& wh, const Buffer<ssh_cs>& pix);
			// �������� ���� ������� BFS
			static void saveBFS(File* f, const Range<int>& wh, ssh_cs* pix);
			// ��������
			Range<int> wh;
			// ����� ��������
			Buffer<ssh_cs> pix;
		};
		enum class TypesMap : int
		{
			CubeMap, VolumeMap, TextureMap,
			ArrayMap, AtlasMap
		};
		enum class LayersCube : int
		{
			PositiveX, PositiveY, PositiveZ,
			NegativeX, NegativeY, NegativeZ
		};
		enum class OptsFont : int
		{
			ss
		};
		// ��������/���������� �����
		void set_map(ssh_wcs path, int layer, int mip, int src_layer, int src_mip);
		// ��������/���������� �����
		//void set_font(ssh_wcs face, OptsFont opts, int height, int layer);
		// ������� �����
		void del_map(int layer) { maps[layer] = nullptr; }
		// ������� �����
		const ImgMap* get_map(int layer) { return maps[layer]; }
		// ������������ ���� �����
		QUAD quad(int layer, const Pts<int>& pt, const Bar<int>& clip, const Bar<int>& screen, const color& col) const;
		// ������� �������� �����
		ImgMap* duplicate(int layer, int mip);
		// ������� �����������
		Buffer<ssh_cs> histogramm(int layer, int mip, const Range<int>& wh, ImgMod::Histogramms type, const color& bkg, const color& frg);
		// ������������
		Buffer<ssh_cs> make() const;
		// ������� ������ ����
		void* get_root() const { return maps.root(); }
		// ������� ���������� ����
		ssh_u count() const { return maps.count(); }
		// ������� �������� ���������� ��� �������
		ssh_u get_mips() const;
		// ������� ������������ ���������� �����
		ssh_u get_layers() const { return layers; }
		// ������� ������
		FormatsMap format() const { return fmt; }
		// ������� ���
		TypesMap type() const { return tp; }
		// ������� ��������
		const Range<int>& range() const { return wh; }
		// ���������
		virtual void save(ssh_wcs path, bool is_xml) override;
		// ������� ����� ��� �������������
		virtual SCHEME* get_scheme() const override { return nullptr; }
	protected:
		// ������������
		Image() {}
		Image(ssh_wcs path) { open(path); }
		Image(TypesMap _tp, FormatsMap _fmt, int width, int height, ssh_u _mips, ssh_u _layers) : tp(_tp), fmt(_fmt), wh(width, height), mips(_mips), layers(_layers) {}
		// ����������
		virtual ~Image() {}
		// ������������
		virtual void make(const Buffer<ssh_cs>& buf) override;
		// �������� ��� �������
		ssh_u mips = -1;
		// �������� �����
		ssh_u layers = -1;
		// �����
		Map<ImgMap*, int> maps;
		// ��������
		Range<int> wh;
		// ������
		FormatsMap fmt = FormatsMap::bgra8;
		// ���
		TypesMap tp = TypesMap::TextureMap;
	};

	extern "C"
	{
		void asm_ssh_to_bgra8(const Range<int>& wh, int crop_w, void* dst, void* src, FormatsMap fmt);
		void asm_ssh_from_bgra8(const Range<int>& wh, void* dst, void* src, FormatsMap fmt);
		void asm_ssh_bmp(void* buf, const Range<int>& wh, int crop_w, void* src, void* pal, int flags, int bpp, FormatsMap fmt);
		void asm_ssh_tga(void* buf, const Range<int>& wh, int crop_w, void* src, void* pal, int flags, int bpp, FormatsMap fmt);
		void asm_ssh_gif(int crop_w, void* dst, void* pal, void* stk, void* plzw, int iTrans);
		ssh_d asm_ssh_bfs(void* src);
		void asm_ssh_h_flip(void* buf, const Bar<int>& bar, const Range<int>& wh);
		void asm_ssh_v_flip(void* buf, const Bar<int>& bar, const Range<int>& wh);
	}
}


/*
extern "C"
{
uint_t	__stdcall	asmTargetSize(uint_t width, uint_t height, ImgIO::ImageFormats fmt, uint_t* flag = nullptr);
uint_t	__stdcall	asmPitch(uint_t width, ImgIO::ImageFormats fmt);
uint_t	__stdcall	asmDecode(uint_t width, uint_t height, void* dst, void* src, uint_t fmt, uint_t is_compress);
QUAD*	__stdcall	asmMakeQuad(const Bar<float>& tex, const Bar<uint_t>& pos, const Bar<uint_t>& clip, QUAD* quad, const Range<uint_t>& screen, uint_t bgra);
void	__stdcall	asmMakeQuadsText(const Bar<uint_t>& pos, const Bar<uint_t>& clip, QUAD* quads, BYTE* buffer, const Range<uint_t>& screen, ImgTexture* fnt);
uint_t	__stdcall	asmInstallText(PCC font, const Range<uint_t>& clip, BYTE* buffer, PCC msg, ImgText::Aligned flags, void* cells);
void	__stdcall	asmDrawText(const Bar<uint_t>& bar, const Range<uint_t>& clip, BYTE* pixels, BYTE* text, const Pts<uint_t>& pt);
void	__stdcall	asmCopy(const Bar<uint_t>& dstBar, const Range<uint_t>& dstClip, BYTE* dst, BYTE* src, const Bar<uint_t>& srcBar, const Range<uint_t>& srcClip, ImgModify::PixelOperations ops, ImgModify::Addresses address, ImgModify::Filters filers, uint_t msk, uint_t nWrap, const vec4& fltVec, float alpha = 1.0f, uint_t whMtx = 3);
BYTE*	__stdcall	asmHistogramm(const Range<uint_t>& wh, const Range<uint_t>& clip, BYTE* rgba, BYTE* pixels, uint_t background, uint_t foreground, ImgModify::Histogramms type);
void	__stdcall	asmMakeMipmapLevel(uint_t width, uint_t height, BYTE* dst);
uint_t	__stdcall	asmGetWidthPictureSymvol(BYTE* src);
uint_t	__stdcall	asmGetHeightPictureSymvol(BYTE* src, uint_t w, uint_t h);
void	__stdcall	asmFlip90(const Range<uint_t>& bar, BYTE* dst, BYTE* src);
void	__stdcall	asmBorder3d(const Bar<uint_t>& bar, const Range<uint_t>& clip, BYTE* src, uint_t widthBorder, uint_t val, uint_t msk, ImgModify::Borders side, ImgModify::TypeOperations typeOps);
void	__stdcall	asmGroupBox(const Bar<uint_t>& bar, const Range<uint_t>& clip, BYTE* src, uint_t widthBorder, uint_t val, uint_t msk, ImgModify::Borders side, ImgModify::TypeOperations typeOps);
void	__stdcall	asmTable(const Bar<uint_t>& bar, const Range<uint_t>& clip, BYTE* dst, BYTE* data, uint_t array_count, uint_t width, uint_t val, uint_t msk, ImgModify::PixelOperations pixOps, ImgModify::PixelOperations pixOpsEx, ImgModify::TypeOperations typeOps);
void	__stdcall	asmBorder(const Bar<uint_t>& bar, const Range<uint_t>& clip, BYTE* src, uint_t widthBorder, uint_t val, uint_t msk, ImgModify::Borders side, ImgModify::PixelOperations pixOps);
void	__stdcall	asmNoisePerlin(const Range<uint_t>& clip, uint_t val, BYTE* src, float scale);
void	__stdcall	asmNoiseTerrain(const Bar<uint_t>& bar, const Range<uint_t>& clip, BYTE* src, const Range<uint_t>& vals, const Range<uint_t>& wh, ImgModify::PixelOperations pixOps, uint_t nRepeat);
void	__stdcall	asmCorrect(const Range<uint_t>& clip, const Range<uint_t>& range, BYTE* src, ImgModify::Histogramms type);
void	__stdcall	asmMosaik(const Range<uint_t>& wh, void* cells, BYTE* bgra, String* src, const Range<uint_t>& count, const Range<uint_t>& wh_cells, uint_t rel);
void	__stdcall	asmFigure(const Bar<uint_t>& bar, const Range<uint_t>& clip, BYTE* src, BYTE* bgra, const Range<uint_t>& srcBar, const Range<uint_t>& vals, const Range<uint_t>& msks, ImgModify::PixelOperations pixOps, ImgModify::PixelOperations pixOpsEx, ImgModify::Figures figure, uint_t radius, uint_t shadow);
void	__stdcall	asmGradient(const Bar<uint_t>& bar, const Range<uint_t>& clip, BYTE* src, const Range<uint_t>& vals, uint_t msk, ImgModify::PixelOperations pixOps, ImgModify::Addresses address, uint_t nRepeat);
void	__stdcall	asmReplace(const Range<uint_t>& vals, const Range<uint_t>& msks, BYTE* src, const Range<uint_t>& clip);
Range<uint_t>*	__stdcall	asmGetRangeText();
Bar<uint_t>* __stdcall asmGetClipBar(const Bar<uint_t>& bar, Bar<uint_t>* out, const Bar<uint_t>& clip);
};
*/
