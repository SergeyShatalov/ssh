
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
		public:
			// конструктор
			ImgMap(const Range<int>& _wh, const Buffer<ssh_cs>& _pix) : wh(_wh), pix(_pix) { }
			// добавить/заменить мип
			void set_mip(int level, ImgMap* mip) { mips[level] = mip; }
			// удалить мип
			void del_mip(int level) { mips[level] = nullptr; }
			// вернуть мип
			const ImgMap* get_mip(int level) { return mips[level]; }
			// вернуть габариты
			const Range<int>& range() const { return wh; }
			// вернуть буфер пикселей
			const Buffer<ssh_cs>& pixels() const { return pix; }
		protected:
			// габариты
			Range<int> wh;
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
				tga, bmp, fse, bfs, gif, dds, jpg, undef
			};
			// конструктор
			ImgCnv(ssh_wcs path, int layer, int mip);
			// записать карту в файл в определенном формате
			static void save(ssh_wcs path, ImgMap* map, ImgCnv::Types type);
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
			void makeGIF(ssh_cs* buf, int kadr);
			// записать файл формата BMP
			static void saveBMP(File* f, const Range<int>& wh, const Buffer<ssh_cs>& pix);
			// записать файл формата TGA
			static void saveTGA(File* f, const Range<int>& wh, const Buffer<ssh_cs>& pix);
			// записать файл формата FSE
			static void saveFSE(File* f, const Range<int>& wh, const Buffer<ssh_cs>& pix);
			// записать файл формата BFS
			static void saveBFS(File* f, const Range<int>& wh, ssh_cs* pix);
			// габариты
			Range<int> wh;
			// буфер пикселей
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
		// добавить/установить карту
		void set_map(ssh_wcs path, int layer, int mip, int src_layer, int src_mip);
		// добавить/установить шрифт
		//void set_font(ssh_wcs face, OptsFont opts, int height, int layer);
		// удалить карту
		void del_map(int layer) { maps[layer] = nullptr; }
		// вернуть карту
		const ImgMap* get_map(int layer) { return maps[layer]; }
		// сформировать квад карты
		QUAD quad(int layer, const Pts<int>& pt, const Bar<int>& clip, const Bar<int>& screen, const color& col) const;
		// создать дубликат карты
		ImgMap* duplicate(int layer, int mip);
		// создать гистограмму
		Buffer<ssh_cs> histogramm(int layer, int mip, const Range<int>& wh, ImgMod::Histogramms type, const color& bkg, const color& frg);
		// сформировать
		Buffer<ssh_cs> make() const;
		// вернуть корень карт
		void* get_root() const { return maps.root(); }
		// вернуть количество карт
		ssh_u count() const { return maps.count(); }
		// вернуть реальное количество мип уровней
		ssh_u get_mips() const;
		// вернуть максимальное количество слоев
		ssh_u get_layers() const { return layers; }
		// вернуть формат
		FormatsMap format() const { return fmt; }
		// вернуть тип
		TypesMap type() const { return tp; }
		// вернуть габариты
		const Range<int>& range() const { return wh; }
		// сохранить
		virtual void save(ssh_wcs path, bool is_xml) override;
		// вернуть схему для сериализатора
		virtual SCHEME* get_scheme() const override { return nullptr; }
	protected:
		// конструкторы
		Image() {}
		Image(ssh_wcs path) { open(path); }
		Image(TypesMap _tp, FormatsMap _fmt, int width, int height, ssh_u _mips, ssh_u _layers) : tp(_tp), fmt(_fmt), wh(width, height), mips(_mips), layers(_layers) {}
		// деструктор
		virtual ~Image() {}
		// сформировать
		virtual void make(const Buffer<ssh_cs>& buf) override;
		// максимум мип уровней
		ssh_u mips = -1;
		// максимум слоев
		ssh_u layers = -1;
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
