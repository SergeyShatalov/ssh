
/*
*	�����:		������� �. �.
*	�������:	�����������, 18 ������� 2015, 16:30
*	�����������:--
*	��������:	����� �������� �� �������� � ����������� �����������
*/

#pragma once

namespace ssh
{
	class SSH Image
	{
	public:
	protected:
	};
}

/*


#pragma once

#include <dxgi.h>
#include <ostd\oarch.h>
#include <ostd\omath.h>
#include <ostd\omap.h>

#define OSTD_MAKEFOURCC(ch0, ch1, ch2, ch3)	((DWORD)(BYTE)(ch0)|((DWORD)(BYTE)(ch1) << 8) | ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24))
#define SIGN_IMAGE		0x10203040

namespace ostd
{
	class Image;

	struct QUAD
	{
		vec4 pos;
		vec4 tex;
		color diffuse;
	};

	class OSTD ImgIO final
	{
		// ����������� �� ���������(��������)
		ImgIO() {}
	public:
		enum class TypesIO : uint_t
		{
			Tga, Bmp, Fse, Bfs, Jpg, Gif, Dds
		};
		enum class FlagsIO : uint_t
		{
			Upper = 32,
			Right = 16,
			Grey = 8,
			RGB = 4,
			Indexed = 2,
			RLE = 1,
			Null = 0
		};
		enum class ImageFormats : int
		{
			// ��������������
			Undefine,
			// ����� �����������
			BGRA8, RGBA8, RGB8, BGR8,
			// ����� 16 ������
			B5G6R5, BGR5A1, BGRA4,
			// �����������
			Alpha, Luminance, Font,
			// �����������
			BC1, BC2, BC3
		};
		enum class DxFormats : int
		{
			// ��������������
			Undefine = DXGI_FORMAT_UNKNOWN,
			// �����������
			BGRA8 = DXGI_FORMAT_B8G8R8A8_UNORM,
			RGBA8 = DXGI_FORMAT_R8G8B8A8_UNORM,
			RGB8 = DXGI_FORMAT_R8G8B8A8_UNORM,
			BGR8 = DXGI_FORMAT_B8G8R8X8_UNORM,
			// 16 ������
			B5G6R5 = DXGI_FORMAT_B5G6R5_UNORM,
			BGR5A1 = DXGI_FORMAT_B5G5R5A1_UNORM,
			BGRA4 = DXGI_FORMAT_B4G4R4A4_UNORM,
			// �����������
			Alpha = DXGI_FORMAT_A8_UNORM,
			Luminance = DXGI_FORMAT_R8_UNORM,
			Font = DXGI_FORMAT_B8G8R8A8_UNORM,
			// �����������
			BC1 = DXGI_FORMAT_BC1_UNORM,
			BC2 = DXGI_FORMAT_BC2_UNORM,
			BC3 = DXGI_FORMAT_BC3_UNORM,
			// ��� DDS
			dx9DXT1 = OSTD_MAKEFOURCC('D', 'X', 'T', '1'),
			dx9DXT3 = OSTD_MAKEFOURCC('D', 'X', 'T', '3'),
			dx9DXT5 = OSTD_MAKEFOURCC('D', 'X', 'T', '5'),
		};
		enum class TypesDds : uint_t { Texture, Atlas, Array, Volume, Cube };
		struct IMAGE
		{
			// �����������
			IMAGE() {}
			// ���������������� �����������
			IMAGE(uint_t width, uint_t height, ImageFormats fmt) : width(width), height(height), fmt(fmt) {}
			// ������������ �� ���������� ������
			void decompress();
			// ������
			uint_t width;
			// ������
			uint_t height;
			// ������
			ImageFormats fmt;
			// ����� ��������
			Ptr<BYTE> pixels;
		};
		// ����������� ��� ��������
		ImgIO(const String& path, bool is_decompress = true);
		// ����������
		static void save(const String& path, const Ptr<BYTE>& pixels, uint_t width, uint_t height, TypesIO type, ImageFormats fmt = ImageFormats::BGRA8);
		static void save(const String& path, IMAGE* img, TypesIO type, ImageFormats fmt = ImageFormats::BGRA8);
		// ����������
		~ImgIO() {}
		// ����������� ��� �����������
		IMAGE* enumerate(bool is_begin) { if(is_begin) imgs.root(); auto n(imgs.next()); return (n ? n->value : nullptr); }
		// �������� ���� ������� DDS
		static void saveDDS(const String& path, uint_t width, uint_t height, ImageFormats fmt, const Ptr<BYTE>& pix, TypesDds type_dds, uint_t mips, uint_t layers, bool is_cnv);
	protected:
		// ������������� bpp � ������ �����������
		ImageFormats formatFromBpp(uint_t bpp, uint_t greenMask);
		// ������������ ���� ������� BMP
		void makeBMP(const String& path, BYTE* buf);
		// ������������ ���� ������� TGA
		void makeTGA(const String& path, BYTE* buf);
		// ������������ ���� ������� JPG
		void makeJPG(const String& path, BYTE* buf);
		// ������������ ���� ������� DDS
		void makeDDS(const String& path, BYTE* buf);
		// ������������ ���� ������� FSE
		void makeFSE(const String& path, BYTE* buf);
		// ������������ ���� ������� BFS (R-TYPE)
		void makeBFS(const String& path, BYTE* buf);
		// ������������ ���� ������� GIF
		void makeGIF(const String& path, BYTE* buf);
		// �������� ���� ������� BMP
		static void saveBMP(const String& path, uint_t width, uint_t height, ImageFormats fmt, const Ptr<BYTE>& pix);
		// �������� ���� ������� TGA
		static void saveTGA(const String& path, uint_t width, uint_t height, ImageFormats fmt, const Ptr<BYTE>& pix);
		// �������� ���� ������� FSE
		static void saveFSE(const String& path, uint_t width, uint_t height, ImageFormats fmt, const Ptr<BYTE>& pix);
		// �������� ���� ������� BFS
		static void saveBFS(const String& path, uint_t width, uint_t height, ImageFormats fmt, const Ptr<BYTE>& pix);
		// ������ �����������
		List<IMAGE*, OSTD_PTR> imgs;
	};

	// ��������
	class OSTD ImgTexture
	{
		friend class Image;
		friend class ImgModify;
	public:
		enum SideCube { positiveX, negativeX, positiveY, negativeY, positiveZ, negativeZ };
		// ����������� �� ���������
		ImgTexture() : heightSyms(0), realHeightSyms(0), numMip(0), numLayer(-1) {}
		// ���������������� ������������
		ImgTexture(const Bar<uint_t>& bar, const Bar<float>& fbar, const Ptr<Bar<float>>& syms, uint_t hsym, uint_t hsym2, uint_t mip = 0, uint_t layer = -1) :
			iBar(bar), fBar(fbar), heightSyms(hsym), realHeightSyms(hsym2), numLayer(layer), numMip(mip) { fontSyms.copy(syms, Ptr<Bar<float>>::opsCopy); }
		ImgTexture(const Bar<uint_t>& bar, uint_t hsym, uint_t hsym2, uint_t mip = 0, uint_t layer = -1) :
			heightSyms(hsym), realHeightSyms(hsym2), numLayer(layer), numMip(mip) { setBar(bar, bar.range); }
		// ����������
		~ImgTexture() {}
		// ���������� ��� �������
		void mip(uint_t num) { numMip = num; }
		// ���������� �������
		void layer(uint_t num) { numLayer = num; }
		// ������� ������ ������
		uint_t getHeightSyms() const { return heightSyms; }
		// ���������� �������
		void setBar(const Bar<uint_t>& bar, const Range<uint_t>& r)
		{
			iBar = bar;
			fBar.x = (float)bar.x / (float)r.w;
			fBar.y = (float)bar.y / (float)r.h;
			fBar.w = (float)bar.w / (float)r.w;
			fBar.h = (float)bar.h / (float)r.h;
		}
		// ������� �������
		const Bar<uint_t>& getBar() const { return iBar; }
		const Bar<float>& getFBar() const { return fBar; }
		// ������� �������
		const Ptr<BYTE>& getPixels() const { return pixels; }
	protected:
		// ����� ���� � ������ ��� ������� � ���������� ��������
		uint_t numLayer;
		// ����� ��� ������
		uint_t numMip;
		// ������ ��������
		uint_t heightSyms;
		// ��� ������ ��������
		uint_t realHeightSyms;
		// ����� ��������
		Ptr<BYTE> pixels;
		// ������� �������� ������ � ��������
		Ptr<Bar<float>> fontSyms;
		// �������
		Bar<float> fBar;
		Bar<uint_t> iBar;
	};

	// �����������
	class OSTD ImgModify
	{
	public:
		// ���� �������������
		enum class Types : uint_t
		{
			Undefine,
			Flip,			// ����������
			Copy,			// �����������
			Border,			// ������� �����
			Resize,			// �������� ������
			Noise,			// ������������ ���
			Correct,		// ��������� �� ������ �����������
			Mosaik,			// ������������ �������
			Figure,			// ��������� ������
			Gradient,		// ����������� �������
			Replace,		// ������ �����
			Histogramm		// �����������
		};
		// ������� ���������
		enum class Coordinates : uint_t
		{
			Absolute,		// ���������� ��������
			Percent			// �������� � ���������
		};
		// ���� ���������
		enum class Addresses : uint_t
		{
			LinearClamp,	// 
			LinearMirror,	// �������
			LinearRepeat,	// ������
			NearClamp,
			NearMirror,
			NearRepeat
		};
		// ���� ���������
		enum class TerrainTypes : uint_t
		{
			Mountain,		// ��������
			Hill,			// ���������
			Plain,			// ���������
			Valley,			// ������
			Island,			// ������
			Pit,			// ������(���� ��� � ��� ���)
			Pass,			// �������
			Raid			// ������
		};
		// ���� ��������
		enum class TypeOperations : uint_t
		{
			None,					// ��� ��������
			Perlin,					// ��� �������
			Terrain,				// ��� ������������ ���������
			BrdOut3d,				// ������� 3� �����
			BrdIn3d,				// ��������� 3� �����
			GroupBox,				// ����� ��� ������ ���������
			FlipH,					// �������������� �������
			FlipV,					// ������������ �������
			Flip90,					// ������� �� 90 ��������
			Table,					// ��������� �����
			Table3d,				// ����� 3� �������
			TableGrp				// ����� ������� ��� ������ ���������
		};
		// �������� �������� ��� ���������
		enum class PixelOperations : uint_t
		{
			Add,					// ����������
			Sub,					// ���������
			Set,					// ���������
			Xor,					// ����������� ���
			And,					// ���������� �
			Or,						// ���������� ���
			Lum,					// ������� ������
			Not,					// ���������
			Alpha,					// ������ �����
			Fixed,					// ������������� �����(��������)
			Mull,					// ���������
			LumAdd,					// ���������� �������� ������
			LumSub,					// ��������� �������� ������
			Norm,					// ������������
			Pow2					// ������ �������� �� �������� ������� ������� 2
		};
		// ���� ��������
		enum class Filters : uint_t
		{
			None,			// ��� �������
			Sobel,			// ������
			Laplacian,		// ������
			Prewit,			// ������
			Emboss,			// ������
			Normal,			// ������������
			Hi,				// ��������
			Low,			// ����������
			Median,			// ���������
			Roberts,		// �������
			Max,			// ������������
			Min,			// �����������
			Contrast,		// ��������
			Binary,			// �����������
			Gamma,			// ����� ���������
			ScaleBias		// 
		};
		// ���� �����
		enum class Figures : uint_t
		{
			Ellipse,		// ������
			Region,			// ������ �����
			Rectangle,		// �������������
			TriangleUp,		// ����������� �����
			TriangleDown,	// ����������� ����
			TriangleRight,	// ����������� ������
			TriangleLeft,	// ����������� �����
			Pyramid,		// ��������
			Sixangle,		// �������������
			Eightangle,		// ��������������
			Romb,			// ����
			Star1,			// ������
			Star2,			// ������ ������
			ArrowRight,		// ������� ������
			ArrowLeft,		// ������� �����
			ArrowDown,		// ������� ����
			ArrowUp,		// ������� �����
			Cross45,		// ����� �� ���������
			Checked,		// ������
			VPlzSlider,		// ������������ �������
			HPlzSlider,		// �������������� �������
			Plus			// ����
		};
		// ���� ������
		enum class Borders : uint_t
		{
			Left = 1,	// �����
			Right = 2,	// ������
			Top = 4,	// ������
			Bottom = 8,	// �����
			All = 15	// ���
		};
		// ������������
		enum class Histogramms : uint_t { Rgb, Red, Green, Blue, ValRgb, ValRed, ValGreen, ValBlue };
		// ����������� �� ���������
		ImgModify() {}
		// ���������������� �����������
		ImgModify(Types type, Addresses address, TypeOperations typeOps, PixelOperations pixOps, Coordinates coord, const Bar<uint_t>& bar, const Range<uint_t>& msks, const Range<uint_t>& vals) :
			type(type), address(address), coord(coord), typeOps(typeOps), pixOps(pixOps), bar(bar), msks(msks), vals(vals) {}
		ImgModify(Image* img, const String& img_name, Xml* xml, HXML hroot);
		// ����������
		~ImgModify() {}
		// ��������� ����������� ��� ������
		void apply(Image* img, const String& name);
		// �������������� �� ����������� ������� ��������� � ����������
		static Bar<uint_t> absoluteBar(const Bar<uint_t>& bar, const Range<uint_t> clip, Coordinates coord)
		{
			if(coord != Coordinates::Percent) return bar;
			float x(bar.x / 100.0f), y(bar.y / 100.0f), w(bar.w / 100.0f), h(bar.h / 100.0f);
			return Bar<uint_t>(x * clip.w, y * clip.h, w * clip.w, h * clip.h);
		}
		// ���
		Types type;
		// ��� ���������
		Coordinates coord;
		// ��������
		// � ���������
		PixelOperations pixOps;
		PixelOperations pixOpsEx;
		// ���� ��������
		TypeOperations typeOps;
		// ���������
		Addresses address;
		// ������
		Figures figure;
		// ������
		Filters filter;
		// ������ ��� �������
		vec4 fltVec;
		// ����� �������
		Borders sideBorder;
		// ������� �������� ������������
		Bar<uint_t> bar;
		// ��������
		Range<uint_t> range;
		// �����
		Range<uint_t> msks;
		// �������� �����
		Range<uint_t> vals;
		// ��������
		Range<uint_t> wh;
		// �������� ������
		Range<uint_t> whCells;
		// ������
		uint_t radius;
		// �������� ����
		uint_t shadow;
		// ������ ��������
		Ptr<BYTE> arrayBGRA;
		// ���������� ����������
		uint_t nRepeat;
		// ������ �������
		uint_t widthBorder;
		// �������� �������
		uint_t whMtx;
		// �������
		float scale;
		// �������� �����
		float alpha;
		// ��� ���������
		TerrainTypes terrain;
		// ��� �����������
		Histogramms histogramm;
		// ����� ����
		Ptr<String> arrayNames;
		// �������� ��������� � ��������
		Range<uint_t> array_count;
		// ��������� ����������� � �������
		uint_t imgRel;
		// ����� ��� ������������ �����������
		Range<uint_t> colors;
	};

	// �����
	class OSTD ImgText
	{
		friend class Image;
	public:
		enum class StylesFont : uint_t { normal, bold, italic };
		enum class Aligned : uint_t
		{
			Left = 0x00000000,// ����������� �� ������ ���� (�� ���������)
			Right = 0x00000001,// ����������� �� ������� ����
			HCenter = 0x00000002,// ����������� �� ��������������� ������
			Top = 0x00000000,// ����������� �� �������� ���� (�� ���������)
			Bottom = 0x00000004,// ����������� �� ������� ����
			VCenter = 0x00000008,// ����������� �� ������������� ������
			WordBreak = 0x00000010// ������������ �������� �� ����� � �������� �����
		};
		// �����������
		ImgText() : flags(Aligned::Left) {}
		// ���������������� �����������
		ImgText(Image* img, const String& name, const String& fontDef, const String& msg, const Bar<uint_t>& clipText, Aligned flags, ImgModify::Coordinates coords) { install(img, name, fontDef, msg, clipText, flags, coords); }
		// ����������
		~ImgText() {}
		// ���������� ������� ������
		void free();
		// ���������� ����� � �����������
		void draw(Image* img, const String& name, const Pts<uint_t>& pt) const;
		// ���������� ����� �������� � ������
		void draw(Image* img, BYTE* buf, const Range<uint_t>& clip, const Pts<uint_t>& pt) const;
		// ������������ ������ ������ (��� ��������� �� GPU)
		void makeQuads(Image* img, const Bar<uint_t>& clip, const Range<uint_t>& screen);
		// �����������
		uint_t install(Image* img, const String& name, const String& font, const String& msg, const Bar<uint_t>& barText, Aligned flags, ImgModify::Coordinates coords);
		// ���������� ����� ������ ������ � ��������
		static Range<uint_t> getRangeString(Image* img, const String& msg, const String& font, const Bar<uint_t>& bar, Aligned align);
		// ������� �����
		Aligned getFlags() const { return flags; }
		// ������� ����
		const Bar<uint_t>& getClip() const { return bar; }
		// ������� ���������� ������
		const Ptr<QUAD>& getQuads() const { return quads; }
		// ������� �������� ������
		const String& getSource() const { return source; }
		// ������� ��� ������
		const String& getFont() const { return font; }
	protected:
		// ��������� ������ � �������� ������ ��� �����
		void make(Aligned flgs, const Bar<uint_t>& b, const String& msg, const String& fontDef)
		{
			flags = flgs;
			bar = b;
			source = msg;
			font = fontDef;
			buffer = Ptr<BYTE>((msg.length() + 128) * 2);
			quads = Ptr<QUAD>();
		}
		// ����� ������������
		Aligned flags;
		// ������� ���������
		Bar<uint_t> bar;
		// ��������������� �����
		Ptr<BYTE> buffer;
		// ����� ������
		Ptr<QUAD> quads;
		// �������� ������
		String source;
		// �������� �����
		String font;
	};

	// �����������
	class OSTD Image : public Resource
	{
		OSTD_DYNCREATE(Image);
	public:
		// ���� ����������� (��������� ��������, �����, ������ �������, �����, ���������� ��������)
		enum class Types : uint_t { Texture, Atlas, Array, Volume, Cube };
		// �������� ��� ��������
		enum class Commands : uint_t { None, Modify, Open, Save, Duplicate, Font, Empty, Remove, Draw, Make, Packed };
		// ��������� �����������
		struct HEAD_IMAGE
		{
			// ���������
			long sign;
			// ���
			Types type;
			// ���������� ��� �������
			uint_t mips;
			// ���������� �������
			uint_t count;
			// ������ �����������
			ImgIO::ImageFormats fmt;
			// �����
			Ptr<BYTE> buffer;
			// �������� �������
			uint_t offsXY;
		};
		// ����������� ��� ��������
		Image(const String& path) { open(path); }
		// ����������� �������� ������
		Image(const String& name, const String& face, long height, DWORD charset, DWORD family, BYTE* syms = nullptr, uint_t nMip = 0, uint_t nLayer = -1) { init(); addFont(name, face, height, charset, family, syms, nMip, nLayer); }
		// ����������� �������� ������ ��������
		Image(const String& name, uint_t width, uint_t height, uint_t nMip = 0, uint_t nLayer = -1) { init(); addEmpty(name, width, height, nMip, nLayer); }
		// ����������� �������� �� �����
		Image(const String& name, const String& path, uint_t nMip = 0, uint_t nLayer = -1, Array<ImgTexture*>* texs = nullptr) { init(); addImg(name, path, nMip, nLayer, texs); }
		// ���������
		virtual void save(const String& path, bool is_xml) override;
		// ������� �����
		virtual base_scheme* getScheme() override { return nullptr; }
		// ���������� ��� �����������
		void type(Types type) { head.type = type; }
		// ���������� ���������� ������������ ��� �������
		void mips(uint_t count) { head.mips = count; }
		// ���������� ���������� ������
		void format(ImgIO::ImageFormats fmt) { head.fmt = fmt; }
		// ���������� ������
		void flush();
		// �������
		void remove(const String& name) { map.remove(name); }
		// ������������ ����
		QUAD quad(const String& name, const Bar<uint_t>& pos, const Bar<uint_t>& clip, const Range<uint_t>& screen, const color& col);
		// �������� �������� � ����
		void save(const String& name, const String& path, ImgIO::TypesIO, ImgIO::ImageFormats fmt);
		void saveDds(const String& path, const Ptr<BYTE>& pixels, const Range<uint_t> range, ImgIO::ImageFormats fmt, uint_t mips);
		// ������� ���������� �������
		uint_t count(bool is_texture);
		// ������� ���������� ��� �������
		uint_t mips() const { return head.mips; }
		// ��������� ���������� ��� �������
		uint_t computeCountMips(Range<uint_t>& rangeTex);
		// ��������� �������� � �����
		Range<size_t> packedAtlas(const Range<uint_t>& max);
		// ������� ��������(� ����������� �� ���� �������)
		Range<uint_t> range(const Range<uint_t>& max);
		// ������� ��� �����������
		Types type() const { return head.type; }
		// ������� ���������� ������
		ImgIO::ImageFormats format() const { return head.fmt; }
		// ������� ������� ������
		ImgIO::DxFormats dxFormat() const;
		// ������� ��������
		ImgTexture* get(const String& name) { return map[name]; }
		// ������� �������� ��������
		ImgTexture* duplicate(const String& _old, const String& _new, uint_t nMip = 0, uint_t nLayer = -1);
		// c������ �� �����
		ImgTexture* addImg(const String& name, const String& path, uint_t nMip = 0, uint_t nLayer = -1, Array<ImgTexture*>* texs = nullptr);
		// ������� ������ ��������
		ImgTexture* addEmpty(const String& name, uint_t width, uint_t height, uint_t nMip = 0, uint_t nLayer = -1);
		// ������� �����
		ImgTexture* addFont(const String& name, const String& face, long height, DWORD charset, DWORD family, BYTE* syms = nullptr, uint_t nMip = 0, uint_t nLayer = -1);
		// ������� �����������
		Ptr<BYTE> histogramm(const String& name, const Range<uint_t>& wh, ImgModify::Histogramms type, const color& background, const color& foreground);
		// ������������
		Ptr<BYTE> make(Range<uint_t>& rangeTex, uint_t* nMips = nullptr);
		// ������� ���� �������
		Bar<uint_t>* clipBar(const Bar<uint_t>& bar, const Bar<uint_t>& clip, Bar<uint_t>* out = nullptr);
		// ������� ������ ����� �������
		void* getCells() { return map.getCells(); }
		// ������� "�������" �����������
		void copy(const String& name, BYTE* buf, const Bar<uint_t>& bar, const Range<uint_t>& clip, ImgModify::Addresses address, ImgModify::PixelOperations pixOps, ImgModify::Coordinates coord, ImgModify::Filters filter, uint_t mask, uint_t rep, float alpha, const vec4& fltVec, uint_t mtx);
		// �������� ���������
		static void freeCopyright();
	protected:
		// ����������
		virtual ~Image() {}
		// ��������� �������������
		virtual void init() override;
		// �����
		virtual void reset() override { map.free(); init(); }
		// ��������� �� ������
		virtual void make(const Ptr<BYTE>& buf, const String& path) override;
		// �������� ��������
		void checkCopyright(BYTE* buf, const Range<uint_t>& wh, const Range<uint_t>& clip);
		// ��������� �� ������������ ����������� ������ ��� ������ � ����
		void checkImg(ImgTexture* img);
		// ������� �������� �� �������� ���������
		ImgTexture* get(uint_t nMip, uint_t nLayer);
		// ���������
		HEAD_IMAGE head;
		// ������ �������
		Map<ImgTexture*, String> map;
	};

	extern "C"
	{
		uint_t	__stdcall	asmTargetSize(uint_t width, uint_t height, ImgIO::ImageFormats fmt, uint_t* flag = nullptr);
		uint_t	__stdcall	asmPitch(uint_t width, ImgIO::ImageFormats fmt);
		uint_t	__stdcall	asmDecode(uint_t width, uint_t height, void* dst, void* src, uint_t fmt, uint_t is_compress);
		void	__stdcall	asmCnv(uint_t width, uint_t height, void* dst, void* src, void* palette, ImgIO::FlagsIO flags, uint_t bpp, uint_t bmp, uint_t fmt);
		void	__stdcall	asmDecodeGIF(void* dst, void* palette, void* stk, void* plzw, uint_t iTrans);
		QUAD*	__stdcall	asmMakeQuad(const Bar<float>& tex, const Bar<uint_t>& pos, const Bar<uint_t>& clip, QUAD* quad, const Range<uint_t>& screen, uint_t bgra);
		void	__stdcall	asmMakeQuadsText(const Bar<uint_t>& pos, const Bar<uint_t>& clip, QUAD* quads, BYTE* buffer, const Range<uint_t>& screen, ImgTexture* fnt);
		uint_t	__stdcall	asmInstallText(PCC font, const Range<uint_t>& clip, BYTE* buffer, PCC msg, ImgText::Aligned flags, void* cells);
		void	__stdcall	asmDrawText(const Bar<uint_t>& bar, const Range<uint_t>& clip, BYTE* pixels, BYTE* text, const Pts<uint_t>& pt);
		void	__stdcall	asmCopy(const Bar<uint_t>& dstBar, const Range<uint_t>& dstClip, BYTE* dst, BYTE* src, const Bar<uint_t>& srcBar, const Range<uint_t>& srcClip, ImgModify::PixelOperations ops, ImgModify::Addresses address, ImgModify::Filters filers, uint_t msk, uint_t nWrap, const vec4& fltVec, float alpha = 1.0f, uint_t whMtx = 3);
		BYTE*	__stdcall	asmHistogramm(const Range<uint_t>& wh, const Range<uint_t>& clip, BYTE* rgba, BYTE* pixels, uint_t background, uint_t foreground, ImgModify::Histogramms type);
		void	__stdcall	asmMakeMipmapLevel(uint_t width, uint_t height, BYTE* dst);
		uint_t	__stdcall	asmGetWidthPictureSymvol(BYTE* src);
		uint_t	__stdcall	asmGetHeightPictureSymvol(BYTE* src, uint_t w, uint_t h);
		void	__stdcall	asmFlipH(const Bar<uint_t>& bar, const Range<uint_t>& clip, BYTE* src);
		void	__stdcall	asmFlipV(const Bar<uint_t>& bar, const Range<uint_t>& clip, BYTE* src);
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
		DWORD	__stdcall	asmDecodeBFS(BYTE* src);
		Range<uint_t>*	__stdcall	asmGetRangeText();
		Bar<uint_t>* __stdcall asmGetClipBar(const Bar<uint_t>& bar, Bar<uint_t>* out, const Bar<uint_t>& clip);
	};
}
*/