
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
		// типы модификаторов
		enum class Types : int
		{
			undef,
			flip,			// развернуть
			copy,			// скопировать
			border,			// создать рамку
			resize,			// изменить размер
			noise,			// генерировать шум
			correct,		// коррекция по данным гистограммы
			//mosaik,			// формирование мазаики
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
		enum class Terrain : int
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
			brd_o3d,				// внешняя 3д рамка
			brd_i3d,				// внутрення 3д рамка
			grp,					// рамка для группы элементов
			h_flip,					// горизонтальное зеркало
			v_flip,					// вертикальное зеркало
			flip_90,				// поворот на 90 градусов
			tbl_2d,					// табличная рамка
			tbl_3d,					// рамка 3д таблицы
			tbl_grp					// рамка таблицы для группы элементов
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
			var_alpha,				// только альфа
			fix_alpha,				// фиксированный альфа(задается)
			mul,					// умножение
			lum_add,				// добавление оттенков серого
			lum_sub,				// вычитание оттенков серого
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
			hi,				// повышение четкости
			low,			// смазывание
			median,			// убрать шум
			roberts,		// робертс
			max,			// максимизация
			min,			// минимизация
			contrast,		// контраст
			binary,			// бинаризация
			gamma,			// гамма коррекция
			scale_bias		// 
		};
		// типы фигур
		enum class Figures : int
		{
			ellipse,		// эллипс
			region,			// массив линий
			rectangle,		// прямоугольник
			tri_u,			// треугольник вверх
			tri_d,			// треугольник вниз
			tri_r,			// треугольник вправо
			tri_l,			// треугольник влево
			pyramid,		// пирамида
			sixangle,		// шестиугольник
			eightangle,		// восьмиугольник
			romb,			// ромб
			star1,			// звезда
			star2,			// звезда Давида
			arrow_r,		// стрелка вправо
			arrow_l,		// стрелка влево
			arrow_d,		// стрелка вниз
			arrow_u,		// стрелка вверх
			cross_diag,		// крест по диагонали
			checked,		// флажок
			vplz,			// вертикальный слайдер
			hplz,			// горизонтальный слайдер
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
			rgb_v,
			red_v,
			green_v,
			blue_v
		};
		ImgMod() {}
		// инициализирующий конструктор из xml
		ImgMod(Xml* xml, HXML hroot, Image* img = nullptr) { make(xml, hroot, img); }
		// создать модификатор
		void make(Xml* xml, HXML hroot, Image* img = nullptr);
		// применить модификатор для карты
		void apply(ImgMap* map);
		// преобразование из процентного задания координат в абсолютные
		static Bar<int> absolute_bar(const Bar<int>& bar, const Range<int>& clip, Coord coord)
		{
			if(coord != Coord::percent) return bar;
			float x(bar.x / 100.0f), y(bar.y / 100.0f), w(bar.w / 100.0f), h(bar.h / 100.0f);
			return Bar<int>((int)(x * clip.w), (int)(y * clip.h), (int)(w * clip.w), (int)(h * clip.h));
		}
		// тип
		Types type = Types::undef;
		// типы операций
		Ops type_ops = Ops::none;
		// адресация
		Addr type_address = Addr::lclamp;
		// фигура
		Figures type_figure = Figures::star1;
		// фильтр
		Flt type_filter = Flt::none;
		// тип ландшафта
		Terrain type_terrain = Terrain::island;
		// тип гистограммы
		Histogramms type_histogramm = Histogramms::rgb;
		// тип координат
		Coord type_coord = Coord::absolute;
		// операции с пикселями
		Range<Pix> ops = Range<Pix>(Pix::set, Pix::set);
		// маски
		Range<int> msks = Range<int>(0xff000000, 0xff000000);
		// значения маски
		Range<int> vals;
		// маска границы
		Borders sides = Borders::all;
		// ширина границы
		int w_border = 1;
		// радиус
		int radius = 1;
		// маска тени
		int shadow = 0xff000000;
		// пропорции изображений в мозаике
		int aspect = 1;
		// габариты матрицы
		int w_mtx = 3;
		// масштаб
		float scale = 1.0f;
		// значение альфы
		float alpha = 1.0f;
		// массив значений
		Buffer<ssh_cs> rgba;
		// вектор для фильтра
		vec4 flt_vec = vec4(1.0f, 1.0f, 1.0f, 1.0f);
		// область действия модификатора
		Bar<int> bar;
		// диапазон
		Range<int> rn;
		// габариты
		Range<int> wh = Range<int>(10, 10);
		// габариты ячейки
		Range<int> wh_cell = Range<int>(1, 1);
		// количество повторений
		Range<float> wh_rep = Range<float>(1.0f, 1.0f);
		// диапазон элементов в массивах
		Range<int> array_count = Range<int>(1, 1);
		// цвета для формирования гистограммы
		Range<int> cols_histogramm;
	};

	class ImgMap
	{
		friend Image;
		friend ImgMod;
	public:
		// конструктор
		ImgMap() {}
		ImgMap(const Range<int>& _wh, const Buffer<ssh_cs>& _pix) : ixywh(_wh), pix(_pix), fxywh(0.0f, 0.0f, 1.0f, 1.0f) {}
		// признак того, что карта является шрифтом
		virtual bool is_font() const { return false; }
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
		ImgMap* get_mip(int level) { return mips[level]; }
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
		friend class Image;
	public:
		enum class Styles : int
		{
			normal, bold, italic
		};
		enum class Aligned : int
		{
			left	= 0x00000000,// выравнивать по левому краю (по умолчанию)
			right	= 0x00000001,// выравнивать по правому краю
			h_center= 0x00000002,// выравнивать по горизонтальному центру
			top		= 0x00000000,// выравнивать по верхнему краю (по умолчанию)
			bottom	= 0x00000004,// выравнивать по нижнему краю
			v_center= 0x00000008,// выравнивать по вертикальному центру
			brk		= 0x00000010 // осуществлять разбивку на слова в заданном клипе
		};
		// конструктор
		ImgTxt(ssh_wcs name, ssh_wcs face, int height, int min_ws, const Buffer<Bar<float>>& pos, const Buffer<ssh_w>& remap);
		ImgTxt(ssh_wcs name, ssh_wcs face, int height, int min_ws, const Range<int>& wh, const Buffer<Bar<int>>& pos, const Buffer<ssh_w>& remap, const Buffer<ssh_cs>& pix);
		// признак того, что карта является шрифтом
		virtual bool is_font() const override { return true; }
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
		int height = 0;
		// минимальный символ
		int min_ws = 0;
		// буфер квадов - для ГПУ
		Buffer<QUAD> quads;
		// массив позиций символов текста
		Buffer<Bar<float>> pos;
		// массив ремапа кодов символов
		Buffer<ssh_w> remap;
		// буфер инсталяции текста
		Buffer<ssh_cs> install_buf;
		// внутреннее имя щшрифта
		String name;
		// название шрифта
		String face;
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
			tga, bmp, fse, bfs, dds, gif, jpg, psd, png, undef
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
		// сформировать файл формата PSD
		void makePSD(ssh_cs* buf);
		// сформировать файл формата PNG
		void makePNG(ssh_cs* buf) {}
		// список изображений
		List<IMAGE*> imgs;
	};

	class SSH Image : public Resource
	{
		SSH_DYNCREATE(Image);
	public:
		// комманды при создании
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
			// габариты
			Range<int> wh;
			// формат
			FormatsMap fmt = FormatsMap::bgra8;
			// тип
			TypesMap tp = TypesMap::TextureMap;
			// количество мип уровней
			int mip;
			// количество текстур
			int count;
			// сформированная текстура
			buf_cs tex;
		};
		// конструкторы
		Image() {}
		Image(ssh_wcs path) { open(path); }
		Image(TypesMap _tp, FormatsMap _fmt, int width = 0, int height = 0);
		// добавить/установить карту
		int set_map(ssh_wcs path, int layer, int mip = 0);
		// добавить/установить пустую карту
		ImgMap* set_empty(const Range<int>& wh, int layer, int mip = 0);
		// добавить/установить шрифт
		ImgTxt* set_font(ssh_wcs name, ssh_wcs face, ssh_w* groups, int height, int layer);
		// удалить карту
		void del_map(int layer, int mip = 0);
		// сброс внутренних буферов
		void flush();
		// вернуть карту
		ImgMap* get_map(int layer, int mip = 0);
		// создать дубликат карты
		ImgMap* duplicate(int nlayer, int nmip, int olayer, int omip);
		// создать гистограмму
		Buffer<ssh_cs> histogramm(int layer, int mip, const Range<int>& wh, ImgMod::Histogramms type, const color& bkg, const color& frg);
		// сформировать
		Buffer<ssh_cs> make(ImgMap* map = nullptr);
		// вернуть корень карт
		auto root() const { return maps.root(); }
		// вернуть количество слоев
		int layers() const { return (int)maps.count(); }
		// вернуть реальное количество мип уровней
		int get_mips(const ImgMap* map = nullptr, Range<int>* wh = nullptr);
		// вернуть формат
		FormatsMap format() const { return head.fmt; }
		// вернуть тип
		TypesMap type() const { return head.tp; }
		// вернуть реальные габариты
		Range<int> range(const ImgMap* map = nullptr);
		// сохранить
		virtual void save(ssh_wcs path, bool is_xml) override;
		// записать карту в файл в определенном формате
		void save(ssh_wcs path, ImgCnv::Types type, FormatsMap fmt, int layer, int mip = 0);
		// вернуть схему для сериализатора
		virtual SCHEME* get_scheme() const override { return nullptr; }
	protected:
		// деструктор
		virtual ~Image() {}
		// установка карты
		void set_map(ImgMap* map, int layer, int mip);
		// упаковка атласов
		bool packed_atlas(Range<int>& rn, int offsXY = 1);
		// вернуть количество слоев, в зависимости от типа изображения
		int count_layers_from_type() const;
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
		void saveDDS(File* f, ImgMap* map, FormatsMap fmt);
		// карты
		Map<ImgMap*, int> maps;
		// заголовок
		HEADER head;
		// сформировать квад карты
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
