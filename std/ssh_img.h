
/*
*	Автор:		Шаталов С. В.
*	Создано:	Владикавказ, 18 августа 2015, 16:30
*	Модификация:--
*	Описание:	Класс операций по созданию и модификации мзображения
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
		// конструктор по умолчанию(отключён)
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
			// неопределенный
			Undefine,
			// целые стандартные
			BGRA8, RGBA8, RGB8, BGR8,
			// целые 16 битные
			B5G6R5, BGR5A1, BGRA4,
			// специальные
			Alpha, Luminance, Font,
			// упакованные
			BC1, BC2, BC3
		};
		enum class DxFormats : int
		{
			// неопределенный
			Undefine = DXGI_FORMAT_UNKNOWN,
			// стандартные
			BGRA8 = DXGI_FORMAT_B8G8R8A8_UNORM,
			RGBA8 = DXGI_FORMAT_R8G8B8A8_UNORM,
			RGB8 = DXGI_FORMAT_R8G8B8A8_UNORM,
			BGR8 = DXGI_FORMAT_B8G8R8X8_UNORM,
			// 16 битные
			B5G6R5 = DXGI_FORMAT_B5G6R5_UNORM,
			BGR5A1 = DXGI_FORMAT_B5G5R5A1_UNORM,
			BGRA4 = DXGI_FORMAT_B4G4R4A4_UNORM,
			// специальные
			Alpha = DXGI_FORMAT_A8_UNORM,
			Luminance = DXGI_FORMAT_R8_UNORM,
			Font = DXGI_FORMAT_B8G8R8A8_UNORM,
			// упакованные
			BC1 = DXGI_FORMAT_BC1_UNORM,
			BC2 = DXGI_FORMAT_BC2_UNORM,
			BC3 = DXGI_FORMAT_BC3_UNORM,
			// для DDS
			dx9DXT1 = OSTD_MAKEFOURCC('D', 'X', 'T', '1'),
			dx9DXT3 = OSTD_MAKEFOURCC('D', 'X', 'T', '3'),
			dx9DXT5 = OSTD_MAKEFOURCC('D', 'X', 'T', '5'),
		};
		enum class TypesDds : uint_t { Texture, Atlas, Array, Volume, Cube };
		struct IMAGE
		{
			// конструктор
			IMAGE() {}
			// инициализирующий конструктор
			IMAGE(uint_t width, uint_t height, ImageFormats fmt) : width(width), height(height), fmt(fmt) {}
			// декодировать во внутренний формат
			void decompress();
			// ширина
			uint_t width;
			// высота
			uint_t height;
			// формат
			ImageFormats fmt;
			// буфер пикселей
			Ptr<BYTE> pixels;
		};
		// конструктор для загрузки
		ImgIO(const String& path, bool is_decompress = true);
		// сохранение
		static void save(const String& path, const Ptr<BYTE>& pixels, uint_t width, uint_t height, TypesIO type, ImageFormats fmt = ImageFormats::BGRA8);
		static void save(const String& path, IMAGE* img, TypesIO type, ImageFormats fmt = ImageFormats::BGRA8);
		// деструктор
		~ImgIO() {}
		// перечислить все изображения
		IMAGE* enumerate(bool is_begin) { if(is_begin) imgs.root(); auto n(imgs.next()); return (n ? n->value : nullptr); }
		// записать файл формата DDS
		static void saveDDS(const String& path, uint_t width, uint_t height, ImageFormats fmt, const Ptr<BYTE>& pix, TypesDds type_dds, uint_t mips, uint_t layers, bool is_cnv);
	protected:
		// преобразовать bpp в формат изображения
		ImageFormats formatFromBpp(uint_t bpp, uint_t greenMask);
		// сформировать файл формата BMP
		void makeBMP(const String& path, BYTE* buf);
		// сформировать файл формата TGA
		void makeTGA(const String& path, BYTE* buf);
		// сформировать файл формата JPG
		void makeJPG(const String& path, BYTE* buf);
		// сформировать файл формата DDS
		void makeDDS(const String& path, BYTE* buf);
		// сформировать файл формата FSE
		void makeFSE(const String& path, BYTE* buf);
		// сформировать файл формата BFS (R-TYPE)
		void makeBFS(const String& path, BYTE* buf);
		// сформировать файл формата GIF
		void makeGIF(const String& path, BYTE* buf);
		// записать файл формата BMP
		static void saveBMP(const String& path, uint_t width, uint_t height, ImageFormats fmt, const Ptr<BYTE>& pix);
		// записать файл формата TGA
		static void saveTGA(const String& path, uint_t width, uint_t height, ImageFormats fmt, const Ptr<BYTE>& pix);
		// записать файл формата FSE
		static void saveFSE(const String& path, uint_t width, uint_t height, ImageFormats fmt, const Ptr<BYTE>& pix);
		// записать файл формата BFS
		static void saveBFS(const String& path, uint_t width, uint_t height, ImageFormats fmt, const Ptr<BYTE>& pix);
		// список изображений
		List<IMAGE*, OSTD_PTR> imgs;
	};

	// текстура
	class OSTD ImgTexture
	{
		friend class Image;
		friend class ImgModify;
	public:
		enum SideCube { positiveX, negativeX, positiveY, negativeY, positiveZ, negativeZ };
		// конструктор по умолчанию
		ImgTexture() : heightSyms(0), realHeightSyms(0), numMip(0), numLayer(-1) {}
		// инициализирующие конструкторы
		ImgTexture(const Bar<uint_t>& bar, const Bar<float>& fbar, const Ptr<Bar<float>>& syms, uint_t hsym, uint_t hsym2, uint_t mip = 0, uint_t layer = -1) :
			iBar(bar), fBar(fbar), heightSyms(hsym), realHeightSyms(hsym2), numLayer(layer), numMip(mip) { fontSyms.copy(syms, Ptr<Bar<float>>::opsCopy); }
		ImgTexture(const Bar<uint_t>& bar, uint_t hsym, uint_t hsym2, uint_t mip = 0, uint_t layer = -1) :
			heightSyms(hsym), realHeightSyms(hsym2), numLayer(layer), numMip(mip) { setBar(bar, bar.range); }
		// деструктор
		~ImgTexture() {}
		// установить мип уровень
		void mip(uint_t num) { numMip = num; }
		// установить порядок
		void layer(uint_t num) { numLayer = num; }
		// вернуть высоту шрифта
		uint_t getHeightSyms() const { return heightSyms; }
		// установить область
		void setBar(const Bar<uint_t>& bar, const Range<uint_t>& r)
		{
			iBar = bar;
			fBar.x = (float)bar.x / (float)r.w;
			fBar.y = (float)bar.y / (float)r.h;
			fBar.w = (float)bar.w / (float)r.w;
			fBar.h = (float)bar.h / (float)r.h;
		}
		// вернуть области
		const Bar<uint_t>& getBar() const { return iBar; }
		const Bar<float>& getFBar() const { return fBar; }
		// вернуть пиксели
		const Ptr<BYTE>& getPixels() const { return pixels; }
	protected:
		// номер слоя в объёме или сторона в кубической текстуре
		uint_t numLayer;
		// номер мип уровня
		uint_t numMip;
		// высота символов
		uint_t heightSyms;
		// еще высота символов
		uint_t realHeightSyms;
		// буфер пикселей
		Ptr<BYTE> pixels;
		// позиции символов шрифта в текстуре
		Ptr<Bar<float>> fontSyms;
		// область
		Bar<float> fBar;
		Bar<uint_t> iBar;
	};

	// модификатор
	class OSTD ImgModify
	{
	public:
		// типы модификаторов
		enum class Types : uint_t
		{
			Undefine,
			Flip,			// развернуть
			Copy,			// скопировать
			Border,			// создать рамку
			Resize,			// изменить размер
			Noise,			// генерировать шум
			Correct,		// коррекция по данным гистограммы
			Mosaik,			// формирование мазаики
			Figure,			// отрисовка фигуры
			Gradient,		// градиентная заливка
			Replace,		// замена цвета
			Histogramm		// гистограмма
		};
		// системы координат
		enum class Coordinates : uint_t
		{
			Absolute,		// абсолютные значения
			Percent			// значения в процентах
		};
		// типы адресаций
		enum class Addresses : uint_t
		{
			LinearClamp,	// 
			LinearMirror,	// зеркало
			LinearRepeat,	// повтор
			NearClamp,
			NearMirror,
			NearRepeat
		};
		// типы ландшафта
		enum class TerrainTypes : uint_t
		{
			Mountain,		// гористый
			Hill,			// холмистый
			Plain,			// равнинный
			Valley,			// долины
			Island,			// остров
			Pit,			// карьер(типа как в яме все)
			Pass,			// перевал
			Raid			// речной
		};
		// типы операций
		enum class TypeOperations : uint_t
		{
			None,					// нет операции
			Perlin,					// шум перлина
			Terrain,				// шум формирования ландшафта
			BrdOut3d,				// внешняя 3д рамка
			BrdIn3d,				// внутрення 3д рамка
			GroupBox,				// рамка для группы элементов
			FlipH,					// горизонтальное зеркало
			FlipV,					// вертикальное зеркало
			Flip90,					// поворот на 90 градусов
			Table,					// табличная рамка
			Table3d,				// рамка 3д таблицы
			TableGrp				// рамка таблицы для группы элементов
		};
		// основные операции над пикселями
		enum class PixelOperations : uint_t
		{
			Add,					// добавление
			Sub,					// вычитание
			Set,					// замещение
			Xor,					// исключающее ИЛИ
			And,					// логическое И
			Or,						// логическое ИЛИ
			Lum,					// оттенки серого
			Not,					// отрицание
			Alpha,					// только альфа
			Fixed,					// фиксированный альфа(задается)
			Mull,					// умножение
			LumAdd,					// добавление оттенков серого
			LumSub,					// вычитание оттенков серого
			Norm,					// нормализация
			Pow2					// стретч размеров на величину кратную степени 2
		};
		// типы фильтров
		enum class Filters : uint_t
		{
			None,			// без фильтра
			Sobel,			// собель
			Laplacian,		// лаплас
			Prewit,			// превит
			Emboss,			// эмбосс
			Normal,			// нормализация
			Hi,				// засветка
			Low,			// затемнение
			Median,			// медианный
			Roberts,		// робертс
			Max,			// максимизация
			Min,			// минимизация
			Contrast,		// контраст
			Binary,			// бинаризация
			Gamma,			// гамма коррекция
			ScaleBias		// 
		};
		// типы фигур
		enum class Figures : uint_t
		{
			Ellipse,		// эллипс
			Region,			// массив линий
			Rectangle,		// прямоугольник
			TriangleUp,		// треугольник вверх
			TriangleDown,	// треугольник вниз
			TriangleRight,	// треугольник вправо
			TriangleLeft,	// треугольник влево
			Pyramid,		// пирамида
			Sixangle,		// шестиугольник
			Eightangle,		// восьмиугольник
			Romb,			// ромб
			Star1,			// звезда
			Star2,			// звезда Давида
			ArrowRight,		// стрелка вправо
			ArrowLeft,		// стрелка влево
			ArrowDown,		// стрелка вниз
			ArrowUp,		// стрелка вверх
			Cross45,		// крест по диагонали
			Checked,		// флажок
			VPlzSlider,		// вертикальный слайдер
			HPlzSlider,		// горизонтальный слайдер
			Plus			// плюс
		};
		// типы границ
		enum class Borders : uint_t
		{
			Left = 1,	// слева
			Right = 2,	// справа
			Top = 4,	// сверху
			Bottom = 8,	// снизу
			All = 15	// все
		};
		// гистрограммы
		enum class Histogramms : uint_t { Rgb, Red, Green, Blue, ValRgb, ValRed, ValGreen, ValBlue };
		// конструктор по умолчанию
		ImgModify() {}
		// инициализирующий конструктор
		ImgModify(Types type, Addresses address, TypeOperations typeOps, PixelOperations pixOps, Coordinates coord, const Bar<uint_t>& bar, const Range<uint_t>& msks, const Range<uint_t>& vals) :
			type(type), address(address), coord(coord), typeOps(typeOps), pixOps(pixOps), bar(bar), msks(msks), vals(vals) {}
		ImgModify(Image* img, const String& img_name, Xml* xml, HXML hroot);
		// деструктор
		~ImgModify() {}
		// применить модификатор для атласа
		void apply(Image* img, const String& name);
		// преобразование из процентного задания координат в абсолютные
		static Bar<uint_t> absoluteBar(const Bar<uint_t>& bar, const Range<uint_t> clip, Coordinates coord)
		{
			if(coord != Coordinates::Percent) return bar;
			float x(bar.x / 100.0f), y(bar.y / 100.0f), w(bar.w / 100.0f), h(bar.h / 100.0f);
			return Bar<uint_t>(x * clip.w, y * clip.h, w * clip.w, h * clip.h);
		}
		// тип
		Types type;
		// тип координат
		Coordinates coord;
		// операции
		// с пикселями
		PixelOperations pixOps;
		PixelOperations pixOpsEx;
		// типы операций
		TypeOperations typeOps;
		// адресация
		Addresses address;
		// фигура
		Figures figure;
		// фильтр
		Filters filter;
		// вектор для фильтра
		vec4 fltVec;
		// маска границы
		Borders sideBorder;
		// область действия модификатора
		Bar<uint_t> bar;
		// диапазон
		Range<uint_t> range;
		// маски
		Range<uint_t> msks;
		// значения маски
		Range<uint_t> vals;
		// габариты
		Range<uint_t> wh;
		// габариты ячейки
		Range<uint_t> whCells;
		// радиус
		uint_t radius;
		// величина тени
		uint_t shadow;
		// массив значений
		Ptr<BYTE> arrayBGRA;
		// количество повторений
		uint_t nRepeat;
		// ширина границы
		uint_t widthBorder;
		// габариты матрицы
		uint_t whMtx;
		// масштаб
		float scale;
		// значение альфы
		float alpha;
		// тип ландшафта
		TerrainTypes terrain;
		// тип гистограммы
		Histogramms histogramm;
		// буфер имен
		Ptr<String> arrayNames;
		// диапазон элементов в массивах
		Range<uint_t> array_count;
		// пропорции изображений в мозаике
		uint_t imgRel;
		// цвета для формирования гистограммы
		Range<uint_t> colors;
	};

	// текст
	class OSTD ImgText
	{
		friend class Image;
	public:
		enum class StylesFont : uint_t { normal, bold, italic };
		enum class Aligned : uint_t
		{
			Left = 0x00000000,// выравнивать по левому краю (по умолчанию)
			Right = 0x00000001,// выравнивать по правому краю
			HCenter = 0x00000002,// выравнивать по горизонтальному центру
			Top = 0x00000000,// выравнивать по верхнему краю (по умолчанию)
			Bottom = 0x00000004,// выравнивать по нижнему краю
			VCenter = 0x00000008,// выравнивать по вертикальному центру
			WordBreak = 0x00000010// осуществлять разбивку на слова в заданном ректе
		};
		// конструктор
		ImgText() : flags(Aligned::Left) {}
		// инициализирующий конструктор
		ImgText(Image* img, const String& name, const String& fontDef, const String& msg, const Bar<uint_t>& clipText, Aligned flags, ImgModify::Coordinates coords) { install(img, name, fontDef, msg, clipText, flags, coords); }
		// деструктор
		~ImgText() {}
		// освободить занутую память
		void free();
		// нарисовать текст в изображение
		void draw(Image* img, const String& name, const Pts<uint_t>& pt) const;
		// нарисовать текст напрямую в память
		void draw(Image* img, BYTE* buf, const Range<uint_t>& clip, const Pts<uint_t>& pt) const;
		// сформировать массив квадов (для отрисовки на GPU)
		void makeQuads(Image* img, const Bar<uint_t>& clip, const Range<uint_t>& screen);
		// инсталляция
		uint_t install(Image* img, const String& name, const String& font, const String& msg, const Bar<uint_t>& barText, Aligned flags, ImgModify::Coordinates coords);
		// определить длину строки текста в пикселях
		static Range<uint_t> getRangeString(Image* img, const String& msg, const String& font, const Bar<uint_t>& bar, Aligned align);
		// вернуть флаги
		Aligned getFlags() const { return flags; }
		// вернуть клип
		const Bar<uint_t>& getClip() const { return bar; }
		// вернуть количество квадов
		const Ptr<QUAD>& getQuads() const { return quads; }
		// вернуть исходную строку
		const String& getSource() const { return source; }
		// вернуть имя шрифта
		const String& getFont() const { return font; }
	protected:
		// запомнить строку и выделить память под буфер
		void make(Aligned flgs, const Bar<uint_t>& b, const String& msg, const String& fontDef)
		{
			flags = flgs;
			bar = b;
			source = msg;
			font = fontDef;
			buffer = Ptr<BYTE>((msg.length() + 128) * 2);
			quads = Ptr<QUAD>();
		}
		// флаги выравнивания
		Aligned flags;
		// область клиппинга
		Bar<uint_t> bar;
		// форматированный буфер
		Ptr<BYTE> buffer;
		// буфер квадов
		Ptr<QUAD> quads;
		// исходная строка
		String source;
		// основной шрифт
		String font;
	};

	// изображение
	class OSTD Image : public Resource
	{
		OSTD_DYNCREATE(Image);
	public:
		// типы изображений (одиночная текстура, атлас, массив текстур, объём, кубическая текстура)
		enum class Types : uint_t { Texture, Atlas, Array, Volume, Cube };
		// комманды при создании
		enum class Commands : uint_t { None, Modify, Open, Save, Duplicate, Font, Empty, Remove, Draw, Make, Packed };
		// заголовок изображения
		struct HEAD_IMAGE
		{
			// сигнатура
			long sign;
			// тип
			Types type;
			// количество мип уровней
			uint_t mips;
			// количество текстур
			uint_t count;
			// формат изображения
			ImgIO::ImageFormats fmt;
			// буфер
			Ptr<BYTE> buffer;
			// смешенме атласов
			uint_t offsXY;
		};
		// конструктор для загрузки
		Image(const String& path) { open(path); }
		// конструктор создания шрифта
		Image(const String& name, const String& face, long height, DWORD charset, DWORD family, BYTE* syms = nullptr, uint_t nMip = 0, uint_t nLayer = -1) { init(); addFont(name, face, height, charset, family, syms, nMip, nLayer); }
		// конструктор создания пустой текстуры
		Image(const String& name, uint_t width, uint_t height, uint_t nMip = 0, uint_t nLayer = -1) { init(); addEmpty(name, width, height, nMip, nLayer); }
		// конструктор создания из файла
		Image(const String& name, const String& path, uint_t nMip = 0, uint_t nLayer = -1, Array<ImgTexture*>* texs = nullptr) { init(); addImg(name, path, nMip, nLayer, texs); }
		// сохранить
		virtual void save(const String& path, bool is_xml) override;
		// вернуть схему
		virtual base_scheme* getScheme() override { return nullptr; }
		// установить тип изображения
		void type(Types type) { head.type = type; }
		// установить количество генерируемых мип уровней
		void mips(uint_t count) { head.mips = count; }
		// установить внутренний формат
		void format(ImgIO::ImageFormats fmt) { head.fmt = fmt; }
		// освободить буферы
		void flush();
		// удалить
		void remove(const String& name) { map.remove(name); }
		// сформировать квад
		QUAD quad(const String& name, const Bar<uint_t>& pos, const Bar<uint_t>& clip, const Range<uint_t>& screen, const color& col);
		// записать текстуру в файл
		void save(const String& name, const String& path, ImgIO::TypesIO, ImgIO::ImageFormats fmt);
		void saveDds(const String& path, const Ptr<BYTE>& pixels, const Range<uint_t> range, ImgIO::ImageFormats fmt, uint_t mips);
		// вернуть количество текстур
		uint_t count(bool is_texture);
		// вернуть количество мип уровней
		uint_t mips() const { return head.mips; }
		// вычислить количество мип уровней
		uint_t computeCountMips(Range<uint_t>& rangeTex);
		// упаковать текстуры в атлас
		Range<size_t> packedAtlas(const Range<uint_t>& max);
		// вернуть габариты(в зависимости от типа текстур)
		Range<uint_t> range(const Range<uint_t>& max);
		// вернуть тип изображения
		Types type() const { return head.type; }
		// вернуть внутренний формат
		ImgIO::ImageFormats format() const { return head.fmt; }
		// вернуть целевой формат
		ImgIO::DxFormats dxFormat() const;
		// вернуть текстуру
		ImgTexture* get(const String& name) { return map[name]; }
		// создать дубликат текстуры
		ImgTexture* duplicate(const String& _old, const String& _new, uint_t nMip = 0, uint_t nLayer = -1);
		// cоздать из файла
		ImgTexture* addImg(const String& name, const String& path, uint_t nMip = 0, uint_t nLayer = -1, Array<ImgTexture*>* texs = nullptr);
		// создать пустую текстуру
		ImgTexture* addEmpty(const String& name, uint_t width, uint_t height, uint_t nMip = 0, uint_t nLayer = -1);
		// создать шрифт
		ImgTexture* addFont(const String& name, const String& face, long height, DWORD charset, DWORD family, BYTE* syms = nullptr, uint_t nMip = 0, uint_t nLayer = -1);
		// создать гистограмму
		Ptr<BYTE> histogramm(const String& name, const Range<uint_t>& wh, ImgModify::Histogramms type, const color& background, const color& foreground);
		// сформировать
		Ptr<BYTE> make(Range<uint_t>& rangeTex, uint_t* nMips = nullptr);
		// вернуть клип области
		Bar<uint_t>* clipBar(const Bar<uint_t>& bar, const Bar<uint_t>& clip, Bar<uint_t>* out = nullptr);
		// вернуть корень карты текстур
		void* getCells() { return map.getCells(); }
		// функция "прямого" копирования
		void copy(const String& name, BYTE* buf, const Bar<uint_t>& bar, const Range<uint_t>& clip, ImgModify::Addresses address, ImgModify::PixelOperations pixOps, ImgModify::Coordinates coord, ImgModify::Filters filter, uint_t mask, uint_t rep, float alpha, const vec4& fltVec, uint_t mtx);
		// удаление копирайта
		static void freeCopyright();
	protected:
		// деструктор
		virtual ~Image() {}
		// начальная инициализация
		virtual void init() override;
		// сброс
		virtual void reset() override { map.free(); init(); }
		// загрузить из памяти
		virtual void make(const Ptr<BYTE>& buf, const String& path) override;
		// добавить копирайт
		void checkCopyright(BYTE* buf, const Range<uint_t>& wh, const Range<uint_t>& clip);
		// проверить на дублирование изображений одного мип уровня и слоя
		void checkImg(ImgTexture* img);
		// вернуть текстуру по задынным критериям
		ImgTexture* get(uint_t nMip, uint_t nLayer);
		// заголовок
		HEAD_IMAGE head;
		// массив текстур
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