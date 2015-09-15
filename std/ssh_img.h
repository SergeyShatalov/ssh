
/*
*	Автор:		Шаталов С. В.
*	Создано:	Владикавказ, 20 августа 2015, 8:00
*	Модификация:--
*	Описание:	Класс операций по созданию и модификации мзображения
*/

#pragma once

#include "ssh_arch.h"
#include "ssh_map.h"
#include "ssh_math.h"

namespace ssh
{
	enum class FormatsMap : int
	{
		bc1, bc2, bc3, bgra8, a8, l8, rgba8, rgb8, bgr8, r5g6b5, rgb5a1, rgba4, font, undef
	};

	struct QUAD
	{
		vec4 xy;
		vec4 uv;
		color diff;
	};

	class Image;

	class ImgMod
	{
	public:
		// типы модификаторов
		enum class Types : int
		{
			flip,			// развернуть
			copy,			// скопировать
			border,			// создать рамку
			resize,			// изменить размер
			noise,			// генерировать шум
			correct,		// коррекция по данным гистограммы
			mosaik,			// формирование мазаики
			figure,			// отрисовка фигуры
			gradient,		// градиентная заливка
			replace,		// замена цвета
			histogramm		// гистограмма
		};
		// системы координат
		enum class Coord : int
		{
			absolute,		// абсолютные значения
			percent			// значения в процентах
		};
		// типы адресаций
		enum class Addr : int
		{
			lclamp,		// 
			lmirror,	// зеркало
			lrepeat,	// повтор
			nclamp,
			nmirror,
			nrepeat
		};
		// типы ландшафта
		enum class TerrainTypes : int
		{
			mountain,		// гористый
			hill,			// холмистый
			plain,			// равнинный
			valley,			// долины
			island,			// остров
			pit,			// карьер(типа как в яме все)
			pass,			// перевал
			raid			// речной
		};
		// типы операций
		enum class Ops : int
		{
			none,					// нет операции
			perlin,					// шум перлина
			terrain,				// шум формирования ландшафта
			brdOut3d,				// внешняя 3д рамка
			rrdIn3d,				// внутрення 3д рамка
			groupBox,				// рамка для группы элементов
			flipH,					// горизонтальное зеркало
			flipV,					// вертикальное зеркало
			flip90,					// поворот на 90 градусов
			table,					// табличная рамка
			table3d,				// рамка 3д таблицы
			tableGrp				// рамка таблицы для группы элементов
		};
		// основные операции над пикселями
		enum class Pix : int
		{
			add,					// добавление
			sub,					// вычитание
			set,					// замещение
			xor,					// исключающее ИЛИ
			and,					// логическое И
			or,						// логическое ИЛИ
			lum,					// оттенки серого
			not,					// отрицание
			alpha,					// только альфа
			fixed,					// фиксированный альфа(задается)
			mull,					// умножение
			lumAdd,					// добавление оттенков серого
			lumSub,					// вычитание оттенков серого
			norm,					// нормализация
			pow2					// стретч размеров на величину кратную степени 2
		};
		// типы фильтров
		enum class Flt : int
		{
			none,			// без фильтра
			sobel,			// собель
			laplacian,		// лаплас
			prewit,			// превит
			emboss,			// эмбосс
			normal,			// нормализация
			hi,				// засветка
			low,			// затемнение
			median,			// медианный
			roberts,		// робертс
			max,			// максимизация
			min,			// минимизация
			contrast,		// контраст
			binary,			// бинаризация
			gamma,			// гамма коррекция
			scaleBias		// 
		};
		// типы фигур
		enum class Figures : int
		{
			ellipse,		// эллипс
			region,			// массив линий
			rectangle,		// прямоугольник
			triangleUp,		// треугольник вверх
			triangleDown,	// треугольник вниз
			triangleRight,	// треугольник вправо
			triangleLeft,	// треугольник влево
			pyramid,		// пирамида
			sixangle,		// шестиугольник
			eightangle,		// восьмиугольник
			romb,			// ромб
			star1,			// звезда
			star2,			// звезда Давида
			arrowRight,		// стрелка вправо
			arrowLeft,		// стрелка влево
			arrowDown,		// стрелка вниз
			arrowUp,		// стрелка вверх
			cross45,		// крест по диагонали
			checked,		// флажок
			vPlzSlider,		// вертикальный слайдер
			hPlzSlider,		// горизонтальный слайдер
			plus			// плюс
		};
		// типы границ
		enum class Borders : int
		{
			left	= 1,	// слева
			right	= 2,	// справа
			top		= 4,	// сверху
			bottom	= 8,	// снизу
			all		= 15	// все
		};
		// гистрограммы
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
		friend Image;
	public:
		// конструктор
		ImgMap(const Range<int>& _wh, const Buffer<ssh_cs>& _pix) : ixywh(_wh), pix(_pix) {}
		// добавить/заменить мип
		void set_mip(int level, ImgMap* mip) { mips[level] = mip; }
		// удалить мип
		void del_mip(int level) { mips[level] = nullptr; }
		// установить область карты в изображении(для атласа)
		void set_bar(const Bar<int>& bar, const Range<int>& r)
		{
			ixywh = bar;
			fxywh.x = (float)bar.x / (float)r.w;
			fxywh.y = (float)bar.y / (float)r.h;
			fxywh.w = (float)bar.w / (float)r.w;
			fxywh.h = (float)bar.h / (float)r.h;
		}
		// вернуть мип
		const ImgMap* get_mip(int level) { return mips[level]; }
		// вернуть габариты
		const Bar<int>& bar() const { return ixywh; }
		// вернуть буфер пикселей
		const Buffer<ssh_cs>& pixels() const { return pix; }
	protected:
		// областм
		Bar<int> ixywh;
		Bar<float> fxywh;
		// буфер
		Buffer<ssh_cs> pix;
		// мип уровни
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
			left	= 0x00000000,// выравнивать по левому краю (по умолчанию)
			right	= 0x00000001,// выравнивать по правому краю
			hcenter = 0x00000002,// выравнивать по горизонтальному центру
			top		= 0x00000000,// выравнивать по верхнему краю (по умолчанию)
			bottom	= 0x00000004,// выравнивать по нижнему краю
			vcenter = 0x00000008,// выравнивать по вертикальному центру
			wbreak	= 0x00000010 // осуществлять разбивку на слова в заданном клипе
		};
		// конструктор
		ImgTxt(const Range<int>& _wh, const Buffer<ssh_cs>& _pix, int _height, const Buffer<Bar<int>>& _pos);
		// нарисовать текст из инсталлированного буфера
		void draw(ssh_cs* dst, const Pts<int>& pt, const Bar<int>& clip, const Bar<int>& screen) const;
		// инсталляция - формирование буфера(разбиение, выравнивание)
		int install(Image* img, ssh_wcs text, const Range<int>& clip, Aligned align, ImgMod::Coord coords);
		// вернуть клип инсталлированного текста
		const Range<int>& get_clip() const { return clip; }
		// вернуть инсталлированный текст
		String get_text() const { return text; }
		// вернуть флаги выравнивания инсталлированного текста
		Aligned get_aligned() const { return align; }
		// сформировать массив квадов из инсталлированного буфера
		Buffer<QUAD> make_quads(const Pts<int>& pt, const Bar<int>& clip, const Bar<int>& screen);
		// вернуть квады
		Buffer<QUAD> get_quads() const { return quads; }
	protected:
		// высота символов
		ssh_w height = 0;
		// буфер квадов - для ГПУ
		Buffer<QUAD> quads;
		// массив позиций символов текста
		Buffer<Bar<float>> pos;
		// буфер инсталяции текста
		Buffer<ssh_cs> install_buf;
		// инсталлированный текст
		String text;
		// клип разбивки текста
		Range<int> clip;
		// выравнивание текста
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
			tga, bmp, fse, bfs, dds, gif, jpg, undef
		};
		struct IMAGE
		{
			IMAGE(const Range<int>& _wh) : wh(_wh), pix(Buffer<ssh_cs>(_wh.w * _wh.h * 4)) {}
			// габариты
			Range<int> wh;
			// буфер пикселей
			Buffer<ssh_cs> pix;
		};
		// конструктор
		ImgCnv(ssh_wcs path);
		// перечислить все изображения
		const IMAGE* enumerate(bool is_begin) const
		{
			static List<IMAGE*>::Node* n(nullptr);
			IMAGE* img(nullptr);
			if(is_begin) n = imgs.root();
			if(n) { img = n->value; n = n->next; }
			return img;
		}
	protected:
		// сформировать файл формата BMP
		void makeBMP(ssh_cs* buf);
		// сформировать файл формата TGA
		void makeTGA(ssh_cs* buf);
		// сформировать файл формата JPG
		void makeJPG(ssh_cs* buf, ssh_u sz);
		// сформировать файл формата DDS
		void makeDDS(ssh_cs* buf, int layer, int mip);
		// сформировать файл формата FSE
		void makeFSE(ssh_cs* buf);
		// сформировать файл формата BFS (R-TYPE)
		void makeBFS(ssh_cs* buf);
		// сформировать файл формата GIF
		void makeGIF(ssh_cs* buf);
		// список изображений
		List<IMAGE*> imgs;
	};

	class SSH Image : public Resource
	{
		SSH_DYNCREATE(Image);
	public:

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
		// конструкторы
		Image() {}
		Image(ssh_wcs path) { open(path); }
		Image(TypesMap _tp, FormatsMap _fmt, int width = 0, int height = 0, bool _mips = false);
		// добавить/установить карту
		const ImgMap* set_map(ssh_wcs path, int layer, int mip = -1);
		// добавить/установить шрифт
		//void set_font(ssh_wcs face, OptsFont opts, int height, int layer);
		// удалить карту
		void del_map(int layer) { maps[layer] = nullptr; }
		// вернуть карту
		const ImgMap* get_map(int layer) { return maps[layer]; }
		// сформировать квад карты
		QUAD quad(int layer, const Pts<int>& pt, const Bar<int>& clip, const Bar<int>& screen, const color& col) const;
		// создать дубликат карты
		const ImgMap* duplicate(int nlayer, int nmip, int olayer, int omip);
		// создать гистограмму
		Buffer<ssh_cs> histogramm(int layer, int mip, const Range<int>& wh, ImgMod::Histogramms type, const color& bkg, const color& frg);
		// сформировать
		Buffer<ssh_cs> make(const ImgMap* map = nullptr);
		// вернуть корень карт
		void* get_root() const { return maps.root(); }
		// вернуть количество слоев
		ssh_u layers() const { return maps.count(); }
		// вернуть реальное количество мип уровней
		ssh_u get_mips(const ImgMap* map = nullptr, Range<int>* wh = nullptr);
		// вернуть формат
		FormatsMap format() const { return fmt; }
		// вернуть тип
		TypesMap type() const { return tp; }
		// вернуть реальные габариты
		Range<int> range(const ImgMap* map = nullptr);
		// сохранить
		virtual void save(ssh_wcs path, bool is_xml) override;
		// записать карту в файл в определенном формате
		void save(ssh_wcs path, ImgCnv::Types type, FormatsMap fmt, int layer, int mip = -1);
		// вернуть схему для сериализатора
		virtual SCHEME* get_scheme() const override { return nullptr; }
	protected:
		// деструктор
		virtual ~Image() {}
		// упаковка атласов
		bool packed_atlas(Range<int>& rn);
		// вернуть количество слоев, в зависимости от типа изображения
		int layers_from_type() const;
		// сформировать
		virtual void make(const Buffer<ssh_cs>& buf) override;
		// записать файл формата BMP
		void saveBMP(File* f, const Range<int>& wh, const Buffer<ssh_cs>& pix, FormatsMap fmt);
		// записать файл формата TGA
		void saveTGA(File* f, const Range<int>& wh, const Buffer<ssh_cs>& pix, FormatsMap fmt);
		// записать файл формата FSE
		void saveFSE(File* f, const Range<int>& wh, const Buffer<ssh_cs>& pix, FormatsMap fmt);
		// записать файл формата BFS
		void saveBFS(File* f, const Range<int>& wh, ssh_cs* pix, FormatsMap fmt);
		// записать файл формата DDS
		void saveDDS(File* f, const ImgMap* map, FormatsMap fmt);
		// максимум мип уровней
		bool mips = true;
		// карты
		Map<ImgMap*, int> maps;
		// габариты
		Range<int> wh;
		// формат
		FormatsMap fmt = FormatsMap::bgra8;
		// тип
		TypesMap tp = TypesMap::TextureMap;
	};

	extern "C"
	{
		void asm_ssh_cnv(FormatsMap fmt, const Range<int>& wh, void* dst, void* src, int is);
		void asm_ssh_h_flip(const Bar<int>& bar, const Range<int>& wh, void* buf);
		void asm_ssh_v_flip(const Bar<int>& bar, const Range<int>& wh, void* buf);
		void asm_ssh_unpack_bmp(ssh_u w, ssh_u h, void* dst, void* src, void* pal);
		void asm_ssh_unpack_tga(const Range<int>& wh, void* pal, void* dst, void* src, int bpp, int flags);
		void asm_ssh_unpack_gif(int iTrans, void* pal, void* dst, void* src, void* stk);
		int asm_ssh_compute_fmt_size(int width, int height, FormatsMap fmt, int* is_limit = nullptr);
		void asm_ssh_copy(const Bar<int>& src_bar, const Range<int>& src_wh, void* src, void* dst, const Bar<int>& dst_bar, const Range<int>& dst_wh, ImgMod* modify = nullptr);
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
