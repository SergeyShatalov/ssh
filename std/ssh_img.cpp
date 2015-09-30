
#include "stdafx.h"
#include "ssh_img.h"
#include "ssh_array.h"

namespace ssh
{
	Image::Image(TypesMap _tp, FormatsMap _fmt, int width, int height, bool _mips) : tp(_tp), fmt(_fmt), wh(ssh_pow2<int>(width, false), ssh_pow2<int>(height, false)), mips(_mips) {}

	const ImgMap* Image::set_map(ssh_wcs path, int layer, int mip)
	{
		SSH_TRACE;
		ImgCnv cnv(path);
		ImgMap* _new_map(nullptr);
		const ImgCnv::IMAGE* img(nullptr);
		while(img = cnv.enumerate(img == nullptr))
		{
			_new_map = new ImgMap(img->wh, img->pix);
			set_map(_new_map, layer, mip);
			layer++;
		}
		return _new_map;
	}

	const ImgMap* Image::set_empty(const Range<int>& wh, int layer, int mip)
	{
		ImgMap* new_map(new ImgMap(wh, Buffer<ssh_cs>(asm_ssh_compute_fmt_size(wh.w, wh.h, FormatsMap::rgba8))));
		memset(new_map->pixels(), 0, new_map->pixels().size());
		const ImgMap* map(get_map(layer));
		if(map && mip != -1 && mips) const_cast<ImgMap*>(map)->set_mip(mip, new_map); else maps[layer] = new_map;
		return new_map;
	}

	void Image::del_map(int layer, int mip)
	{
		if(mip != -1)
		{
			ImgMap* map(maps[layer]);
			if(map) map->del_mip(mip);
		}
		else maps[layer] = nullptr;
	}

	ImgMap* Image::get_map(int layer, int mip)
	{
		ImgMap* map(maps[layer]);
		if(mip != -1) { if(map) map = map->get_mip(mip); }
		return map;
	}

	bool Image::set_map(ImgMap* nmap, int layer, int mip)
	{
		if(mip != -1)
		{
			ImgMap* map(maps[layer]);
			if(map) map->set_mip(mip, nmap); else delete nmap;
		}
		else maps[layer] = nmap;
		return false;
	}

	QUAD Image::quad(int layer, const Pts<int>& pt, const Bar<int>& clip, const Bar<int>& screen, const color & col) const
	{
		return QUAD();
	}

	const ImgMap* Image::duplicate(int nlayer, int nmip, int olayer, int omip)
	{
		return nullptr;
	}

	Buffer<ssh_cs> Image::histogramm(int layer, int mip, const Range<int>& wh, ImgMod::Histogramms type, const color & bkg, const color & frg)
	{
		const ImgMap* map(get_map(layer));
		return Buffer<ssh_cs>();
	}

	/*
	Ptr<BYTE> Image::histogramm(const String& name, const Range<uint_t>& wh, ImgModify::Histogramms type, const color& background, const color& foreground)
	{
	ImgTexture* tex(get(name));
	if(!tex) throw ExceptionImage(ExceptionImage::NOTFOUND_TEXTURE, name, OSTD_FLD);
	Ptr<BYTE> buf(CAST(type) >= CAST(ImgModify::Histogramms::ValRgb) ? 1028 : wh.w * wh.h * 4);
	asmHistogramm(wh, tex->getBar().range, buf, tex->getPixels(), background.BGRA(), foreground.BGRA(), type);
	return buf;
	}

	void Image::flush()
	{
	OSTD_TRACE(__FUNCTION__);
	auto n(map.getNull());
	while(n = map.value(n)) n->value->pixels = Ptr<BYTE>();
	head.buffer = Ptr<BYTE>();
	}

	QUAD Image::quad(const String& name, const Bar<uint_t>& pos, const Bar<uint_t>& clip, const Range<uint_t>& screen, const color& col)
	{
	OSTD_TRACE("%s(%s)", __FUNCTION__, name);
	static QUAD quad;
	ImgTexture* tex(get(name));
	if(!tex) throw ExceptionImage(ExceptionImage::NOTFOUND_TEXTURE, name, OSTD_FLD);
	asmMakeQuad(tex->getFBar(), pos, clip, &quad, screen, col.BGRA());
	return quad;
	}


	ImgTexture* Image::duplicate(const String& _old, const String& _new, uint_t nMip, uint_t nLayer)
	{
	OSTD_TRACE("%s(%s,%s)", __FUNCTION__, _old, _new);
	ImgTexture* oldTex(get(_old)), *newTex(get(_new));
	// проверить - такая текстура уже есть?
	if(!oldTex) throw ExceptionImage(ExceptionImage::NOTFOUND_TEXTURE, _old, OSTD_FLD);
	if(newTex) throw ExceptionImage(ExceptionImage::DUPLICATE_NAME, _new, OSTD_FLD);
	map[_new] = (newTex = new ImgTexture(oldTex->iBar, oldTex->heightSyms, oldTex->realHeightSyms, nMip, nLayer));
	newTex->pixels.copy(oldTex->pixels); newTex->fontSyms.copy(oldTex->fontSyms);
	return newTex;
	}

	*/

	Buffer<ssh_cs> Image::make(const ImgMap* map)
	{
		int i, c;
		Bar<int> null;
		TypesMap type;
		Range<int> _wh;
		const ImgMap* tmp_map(nullptr);
		// модификатор по умолчанию
		ImgMod mod_def;
		if(map) { c = 1; type = TypesMap::TextureMap; } else { c = layers_from_type(); type = tp; }
		// определить количество мипуровней и габариты текстуры
		ssh_u nMip(get_mips(map, &_wh));
		// расчитать размер буфера исходного формата
		Buffer<ssh_cs> rgba(_wh.w * _wh.h * 4);
		// расчитать размер буфера целевого формата
		ssh_u sz(0);
		for(i = 0; i < nMip; i++) sz += asm_ssh_compute_fmt_size(_wh.w >> i, _wh.h >> i, fmt);
		sz *= c;
		Buffer<ssh_cs> dst(sz);
		ssh_cs* pdst(dst);
		if(type == TypesMap::VolumeMap)
		{
			for(i = 0; i < nMip; i++)
			{
				const ImgMap* map_mip(nullptr);
				Range<int> mip_wh(_wh.w >> i, _wh.h >> i);
				sz = asm_ssh_compute_fmt_size(mip_wh.w, mip_wh.h, fmt);
				auto n(maps.root());
				while(n)
				{
					tmp_map = n->value;
					if(i) map_mip = const_cast<ImgMap*>(tmp_map)->get_mip(i - 1);
					if(!map_mip) map_mip = tmp_map;
					asm_ssh_copy(null, map_mip->bar().range, map_mip->pixels(), rgba, mip_wh, _wh, &mod_def);
					asm_ssh_cnv(fmt, mip_wh, pdst, rgba, 1);
					pdst += sz;
					n = n->next;
				}
			}
		}
		else if(type == TypesMap::AtlasMap)
		{
			// собрать атлас
			auto n(maps.root());
			while(n)
			{
				tmp_map = n->value;
				asm_ssh_copy(tmp_map->bar().range, tmp_map->bar().range, tmp_map->pixels(), rgba, tmp_map->bar(), _wh, &mod_def);
				n = n->next;
			}
			asm_ssh_cnv(fmt, _wh, dst, rgba, 1);
		}
		else
		{
			auto n(maps.root());
			while(n && c)
			{
				tmp_map = (map ? map : n->value);	// если одна конкрентая карта(или карта мипа)
				for(i = 0; i < nMip; i++)
				{
					const ImgMap* map_mip(nullptr);
					if(i) map_mip = const_cast<ImgMap*>(tmp_map)->get_mip(i - 1);
					if(!map_mip) map_mip = tmp_map;
					Range<int> mip_wh(_wh.w >> i, _wh.h >> i);
					asm_ssh_copy(null, map_mip->bar().range, map_mip->pixels(), rgba, mip_wh, _wh, &mod_def);
					asm_ssh_cnv(fmt, mip_wh, pdst, rgba, 1);
					pdst += asm_ssh_compute_fmt_size(mip_wh.w, mip_wh.h, fmt);
				}
				n = n->next;
				c--;
			}
		}
		return dst;
	}

	int Image::layers_from_type() const
	{
		ssh_u c(layers());
		switch(tp)
		{
			case TypesMap::TextureMap:
			case TypesMap::AtlasMap: c = (c < 1 ? 0 : 1); break;
			case TypesMap::CubeMap: c = (c < 6 ? c : 6); break;
			case TypesMap::ArrayMap:
			case TypesMap::VolumeMap: break;
		}
		return (int)c;
	}

	ssh_u Image::get_mips(const ImgMap* map, Range<int>* wh)
	{
		ssh_u nMips(1);
		int limit;
		// определить реальные габариты изображения
		Range<int> _wh(range(map));
		if(wh) *wh = _wh;
		if(mips && tp != TypesMap::AtlasMap) while(asm_ssh_compute_fmt_size(_wh.w >> nMips, _wh.h >> nMips, fmt, &limit) && !limit) nMips++;
		return nMips;
	}
	
	Range<int> Image::range(const ImgMap* map)
	{
		Range<int> _wh, tmp;
		if(map) _wh = map->bar();
		else
		{
			if(tp == TypesMap::AtlasMap)
			{
				tmp.set(wh.is_null() ? 512 : wh.w, wh.is_null() ? 512 : wh.h);
				// определить минимальный объем всех атласов (pow2)
				while(_wh = tmp, !packed_atlas(_wh)) tmp.w <<= 1, tmp.h <<= 1;
			}
			else
			{
				if(wh.is_null())
				{
					_wh.set(INT_MAX, INT_MAX);
					// определить минимальную ширину и высоту
					auto n(maps.root());
					while(n)
					{
						auto m(n->value);
						tmp = m->bar();
						_wh.w = ssh_min<int>(_wh.w, tmp.w);
						_wh.h = ssh_min<int>(_wh.h, tmp.h);
						n = n->next;
					}
				}
			}
		}
		_wh.set(ssh_pow2<int>(_wh.w, false), ssh_pow2<int>(_wh.h, false));
		return _wh;
	}

	void Image::save(ssh_wcs path, bool is_xml)
	{
	}

	void Image::make(const Buffer<ssh_cs>& buf)
	{
	}

	bool Image::packed_atlas(Range<int>& rn)
	{
		struct MapNode
		{
			MapNode() : type(1) {}
			MapNode(const Bar<int>& bar, int type) : bar(bar), type(type) {}
			Bar<int> bar;
			int type;
		};
		static Bar<int> bar1;
		static Bar<int> bar2;
		
		ImgMap* tex(nullptr);
		// массив отсортированных карт
		Array<ImgMap*, SSH_TYPE> sortw;
		// массив узлов, свободных в глобальной области 
		Array<MapNode*> nodes;
		
		int wmax(0), hmax(0), i, j, tp1, tp2;
		// 1. отсортировать все текстуры по убыванию площади
		nodes += new MapNode(rn, 1);
		// 1.1. скопировать во временный массив
		auto n(maps.root());
		while(n) { sortw += n->value; n = n->next; }
		// 1.2. отсортировать по ширине
		for(i = 0; i < sortw.size(); i++)
		{
			int w1(sortw[i]->ixywh.w), _i(i);
			for(j = i + 1; j < sortw.size(); j++)
			{
				int w2(sortw[j]->ixywh.w);
				if(w2 > w1) { w1 = w2; _i = j; }
			}
			if(_i != i) ssh_swap<ImgMap*>(sortw[_i], sortw[i]);
		}
		// 1.3. сортируем по высоте
		while(sortw.size() > 0)
		{
			tex = nullptr;
			int h1(sortw[0]->ixywh.h), _i(0);
			for(i = 1; i < sortw.size(); i++)
			{
				int h2(sortw[i]->ixywh.h);
				if(h2 > h1) { h1 = h2; _i = i; }
			}
			// 2. упаковать
			Bar<int>* barA(&sortw[_i]->ixywh);
			int aw(barA->w), ah(barA->h);
			for(i = 0; i < nodes.size(); i++)
			{
				MapNode* tn(nodes[i]);
				Bar<int>* barN(&tn->bar);
				if(aw <= barN->w && ah <= barN->h)
				{
					barA->x = barN->x; barA->y = barN->y;
					if(tn->type == 1)
					{
						tp1 = 1; tp2 = 2;
						bar1.set(barN->x, barN->y + ah, barN->w, barN->h - ah);
						bar2.set(barN->x + aw, barN->y, barN->w - aw, ah);
					}
					else
					{
						tp1 = 2; tp2 = 1;
						bar1.set(barN->x + aw, barN->y, barN->w - aw, barN->h);
						bar2.set(barN->x, barN->y + ah, aw, barN->h - ah);
					}
					if(bar2.w && bar2.h)
					{
						*barN = bar2;
						tn->type = tp2;
						if(bar1.w && bar1.h) nodes.insert(i + 1, new MapNode(bar1, tp1));
					}
					else if(bar1.w && bar1.h)
					{
						*barN = bar1;
						tn->type = tp1;
					}
					else
					{
						nodes.remove(i, 1);
					}
					tex = sortw[_i];
					break;
				}
			}
			if(tex)
			{
				wmax = ssh_max<int>(wmax, tex->bar().right());
				hmax = ssh_max<int>(hmax, tex->bar().bottom());
				sortw.remove(_i, 1);
			}
			else return false;
		}
		rn.set(wmax, hmax);
		return true;
	}
}


/*
// водяной знак
static bool is_copyright(false);
static Image* imgOstrov(nullptr);
static ImgText textOstrov;

void Image::freeCopyright()
{
	//imgOstrov->release();
	textOstrov.free();
	imgOstrov = nullptr;
	is_copyright = false;
}

static void makeCopyrightFile()
{
	OSTD_TRACE(__FUNCTION__);
	DWORD serial = 0, ostrov1 = 'RTSO', ostrov2 = '90VO';
	if(::GetVolumeInformation("c:\\", nullptr, 0, &serial, nullptr, nullptr, nullptr, 0)) serial ^= 0xffffffff;

	File f("c:\\ostrov.lic", File::create_write);
	f.write(&serial, 4);
	f.write(&ostrov1, 4);
	f.write(&ostrov2, 4);
}

static void makeCopyright()
{
	OSTD_TRACE(__FUNCTION__);
	static BYTE syms[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x80, 4 + 8 + 16 + 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	is_copyright = true;
	String mainDir;
	// O S T R V
	// определяем рабочую директорию
	hlp->getSystemFolders(&mainDir);
	try
	{
		// открываем файл лицензии
		File file(mainDir + "ostrov.lic", File::open_read);
		// проверяем на достоверность
		if(file.length() == 12)
		{
			DWORD serial(0);
			Ptr<BYTE> pf(file.read<BYTE>());
			// получаем серийный номер тома
			if(::GetVolumeInformation("c:\\", nullptr, 0, &serial, nullptr, nullptr, nullptr, 0)) serial ^= 0xffffffff;
			// проверяем на совпадение
			if((*(DWORD*)(BYTE*)pf == serial) && (*(DWORD*)(BYTE*)(pf + 4) == 'RTSO') && (*(DWORD*)(BYTE*)(pf + 8) == '90VO')) return;
		}
	}
	catch(const ExceptionFile&) {}
	// создаем изображение (шрифт)
	new(&imgOstrov, "_{{{image}}}_") Image("_{{{font}}}_", "Courier New", 48, ANSI_CHARSET, FF_DONTCARE, syms);
	// получаем высоту символов шрифта
	uint_t hsym(imgOstrov->get("_{{{font}}}_")->getHeightSyms());
	// инсталлируем текст
	textOstrov.install(imgOstrov, "", "_{{{font}}}_", "$c(60ffffff)$BOSTROV", Bar<uint_t>(0, 0, 260, hsym), (ImgText::Aligned)(CAST(ImgText::Aligned::HCenter) | CAST(ImgText::Aligned::VCenter)), ImgModify::Coordinates::Absolute);
}

void Image::checkCopyright(BYTE* buf, const Range<uint_t>& wh, const Range<uint_t>& clip)
{
	if(!is_copyright) makeCopyright();
	if(imgOstrov)
	{
		// если лицензии нет - то добавляем водяной знак
		textOstrov.draw(imgOstrov, buf, clip, Pts<uint_t>(wh.w - 260, wh.h - 69));
	}
}

void ImgText::free()
{
	quads.free();
	buffer.free();
	source.empty();
	font.empty();
}

void ImgText::draw(Image* img, const String& name, const Pts<uint_t>& pt) const
{
	ImgTexture* tex(img->get(name));
	if(!tex) throw ExceptionImage(ExceptionImage::NOTFOUND_TEXTURE, name, OSTD_FLD);
	asmDrawText(bar, tex->getBar().range, tex->getPixels(), buffer, pt);
}

void ImgText::draw(Image* img, BYTE* buf, const Range<uint_t>& clip, const Pts<uint_t>& pt) const
{
	if(!buf) throw ExceptionCommon(ExceptionCommon::NULL_PTR, OSTD_FLD);
	asmDrawText(Bar<uint_t>(clip), clip, buf, buffer, pt);
}

uint_t ImgText::install(Image* img, const String& name, const String& fontDef, const String& msg, const Bar<uint_t>& barText, Aligned flags, ImgModify::Coordinates coords)
{
	if(source != msg || font != fontDef)
	{
		if(name.is_empty())
		{
			make(flags, barText, msg, fontDef);
		}
		else
		{
			ImgTexture* tex(img->get(name));
			if(!tex) throw ExceptionImage(ExceptionImage::NOTFOUND_TEXTURE, name, OSTD_FLD);
			make(flags, ImgModify::absoluteBar(barText, tex->getBar().range, coords), msg, fontDef);
		}
		quads = Ptr<QUAD>(asmInstallText(fontDef, bar.range, buffer, msg, flags, img->getCells()));
	}
	return quads.count();
}

void ImgText::makeQuads(Image* img, const Bar<uint_t>& clip, const Range<uint_t>& screen)
{
	OSTD_TRACE("%s(%s,%Ii,%Ii)", __FUNCTION__, screen.w, screen.h);
	ImgTexture* fnt(img->get(font));
	if(!fnt) throw ExceptionImage(ExceptionImage::NOTFOUND_TEXTURE, font, OSTD_FLD);
	asmMakeQuadsText(bar, clip, quads, buffer, screen, fnt);
}

Range<uint_t> ImgText::getRangeString(Image* img, const String& msg, const String& font, const Bar<uint_t>& bar, Aligned align)
{
	ImgText txt;
	txt.install(img, "", font, msg, bar, align, ImgModify::Coordinates::Absolute);
	return Range<uint_t>(*asmGetRangeText());
}

ImgTexture* Image::addEmpty(const String& name, uint_t width, uint_t height, uint_t nMip, uint_t nLayer)
{
	OSTD_TRACE("%s(%s)", __FUNCTION__, name);
	ImgTexture* tex(get(name));
	// проверить - такая текстура уже есть?
	if(tex) throw ExceptionImage(ExceptionImage::DUPLICATE_NAME, name, OSTD_FLD);
	map[name] = (tex = new ImgTexture(Bar<uint_t>(0, 0, width, height), 0, 0, nMip, nLayer));
	tex->pixels.move(nullptr, width * height * 4, Ptr<BYTE>::opsMove);
	OSTD_MEMZERO(tex->pixels, tex->pixels.size());
	// проверить на дублирование слоя
	checkImg(tex);
	return tex;
}

ImgTexture* Image::addFont(const String& name, const String& face, long hf, DWORD charset, DWORD family, BYTE* syms, uint_t nMip, uint_t nLayer)
{
	OSTD_TRACE("%s(%s)", __FUNCTION__, name);
	ImgTexture* font(get(name));
	try
	{
		HFONT hFont[3];
		HBITMAP hBmp, hBmpSym;
		HDC hdc, hdcSym;
		SIZE szSym;
		BYTE* imgSym(nullptr), *bufSym(nullptr);
		// набор символов по умолчанию
		static BYTE syms_def[32] = { 0, 0, 0, 0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0, 0, 0, 0, 0, 1, 0, 1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
		// проверить на дублирование имени
		if(font) throw ExceptionImage(ExceptionImage::DUPLICATE_NAME, name, OSTD_FLD);
		// добавим пробел
		if(!syms) syms = syms_def;
		syms[4] |= 1; // пробел
					  // рассчитать габариты текстуры в зависимости от высоты шрифта
		uint_t nTexHW(hf > 96 ? 4096 : hf > 62 ? 3072 : hf > 47 ? 2048 : hf > 35 ? 1536 : hf > 16 ? 768 : hf > 10 ? 512 : 384), i, x(0), y(0), wmax(0), hf2(0);
		// заполняем структуру для отрисовки всех символов
		BITMAPINFO bmi{ { sizeof(BITMAPINFOHEADER), nTexHW, nTexHW, 1, (WORD)32, BI_RGB, 0, 0, 0, 0, 0 },{ 0, 0, 0, 0 } };
		// заполняем структуру для оценки ширины символа
		BITMAPINFO bmiSym{ { sizeof(BITMAPINFOHEADER), 256, 128, 1, (WORD)32, BI_RGB, 0, 0, 0, 0, 0 },{ 0, 0, 0, 0 } };
		// создать контекст устройства для отрисовки оцениваемого символа
		if(!(hdcSym = ::CreateCompatibleDC(nullptr))) throw(-1);
		if(!(hBmpSym = ::CreateDIBSection(hdcSym, &bmiSym, DIB_RGB_COLORS, (void**)&imgSym, nullptr, 0))) throw(-1);
		// создать контекст устройства для отрисовки символов
		if(!(hdc = ::CreateCompatibleDC(nullptr))) throw(-1);
		if(!(hBmp = ::CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (void**)&bufSym, nullptr, 0))) throw(-1);
		// определяем реальную высоту шрифта
		::SetMapMode(hdc, MM_TEXT);
		::SetMapMode(hdcSym, MM_TEXT);
		hf = -MulDiv(hf, ::GetDeviceCaps(hdc, LOGPIXELSY), 72);
		// создаем шрифт в трех вариантах - обычный, полужирный и наклонный
		if(!(hFont[0] = ::CreateFont(hf, 0, 0, 0, FW_MEDIUM, false, false, false, charset, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, family | DEFAULT_PITCH, face))) throw(-1);
		if(!(hFont[1] = ::CreateFont(hf, 0, 0, 0, FW_BOLD, false, false, false, charset, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, family | DEFAULT_PITCH, face))) throw(-1);
		if(!(hFont[2] = ::CreateFont(hf, 0, 0, 0, FW_NORMAL, true, false, false, charset, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, family | DEFAULT_PITCH, face))) throw(-1);
		// ставим параметры для отрисовки символов шрифта
		::SelectObject(hdc, hBmp);
		::SelectObject(hdcSym, hBmpSym);
		// выравнивание
		::SetTextAlign(hdc, TA_TOP);
		::SetTextAlign(hdcSym, TA_TOP);
		// основной цвет (белый)
		::SetTextColor(hdc, RGB(255, 255, 255));
		::SetTextColor(hdcSym, RGB(255, 255, 255));
		// фоновый цвет (чёрный)
		::SetBkColor(hdc, RGB(0, 0, 0));
		::SetBkColor(hdcSym, RGB(0, 0, 0));
		// определяем максимальную высоту символов
		::SelectObject(hdc, hFont[0]); ::SelectObject(hdcSym, hFont[0]);
		// определяем реальную высоту символов
		::TextOut(hdcSym, 0, 0, "0", 1);
		::GetTextExtentPoint(hdc, "ФЩ09AZ|{}[]/@&ЩруУчЧфФцЦ", 24, &szSym); hf = szSym.cy;
		::GetTextExtentPoint(hdcSym, "0", 1, &szSym);
		hf2 = asmGetHeightPictureSymvol(imgSym, szSym.cx, hf);
		// создаем текстуру
		font = new ImgTexture(Bar<uint_t>(0, 0, nTexHW, nTexHW), hf, hf2, nMip, nLayer);
		font->fontSyms = Ptr<Bar<float>>(768);
		OSTD_MEMZERO(font->fontSyms, sizeof(Bar<float>) * 768);
		// сформировать символы
		::SetBkColor(hdcSym, RGB(255, 255, 255));
		for(uint_t j = 0; j < 3; j++)
		{
			// выбираем шрифт
			::SelectObject(hdc, hFont[j]);
			::SelectObject(hdcSym, hFont[j]);
			// какие символы создавать - единичный бит указывает на признак наличия
			for(i = 0; i < 256; i++)
			{
				uint_t wImg;
				if(syms[i >> 3] & (1 << (i & 7)))
				{
					::GetTextExtentPoint32(hdc, (char*)&i, 1, &szSym);
					if(j == 2)
					{
						::TextOut(hdcSym, 0, 0, (char*)&i, 1);
						wImg = asmGetWidthPictureSymvol(imgSym);
					}
					else wImg = szSym.cx;
					if((x + wImg) >= nTexHW) { x = 0; y += hf; }
					font->fontSyms[i + j * 256].set(x, y, szSym.cx, wImg);
					::TextOut(hdc, x, y, (char*)&i, 1);
					x += wImg + 2;
					wmax = hlp->Max<uint_t>(wmax, x);
				}
			}
			::DeleteObject(hFont[j]);
		}
		// габариты текстуры
		Range<uint_t> clip(nTexHW, nTexHW), range(hlp->clamp<uint_t>(wmax, 0, nTexHW), hlp->clamp<uint_t>(y + hf, 0, nTexHW));
		// сформировать ректы символов в диапазоне [0...1]
		Bar<float> fbar(range.w, range.h, range.w, range.w);
		for(i = 0; i < 768; i++) font->fontSyms[i] /= fbar;
		// преобразовать из исходного во внутренний(BGRA8)
		asmDecode(nTexHW, nTexHW, bufSym, bufSym, CAST(ImgIO::ImageFormats::Font), false);
		// перевернуть
		asmFlipV(clip, clip, bufSym);
		// добавляем
		font->pixels.move(nullptr, asmTargetSize(range.w, range.h, ImgIO::ImageFormats::BGRA8), Ptr<BYTE>::opsMove);
		font->setBar(Bar<uint_t>(range), range);
		map[name] = font;
		// копируем из максимального в реальный
		asmCopy(font->getBar(), range, font->getPixels(), bufSym, Bar<uint_t>(clip), clip, ImgModify::PixelOperations::Set, ImgModify::Addresses::NearClamp, ImgModify::Filters::None, 0, 1, vec4());
		// удаляем временные объекты
		::DeleteObject(hBmp);
		::DeleteObject(hBmpSym);
		::DeleteObject(hdc);
		::DeleteObject(hdcSym);
		// проверить на дублирование слоя
		checkImg(font);
	}
	catch(...) { throw ExceptionImage(ExceptionImage::ERROR_ADD_FONT, name, OSTD_FLD); }
	// возвращаем
	return font;
}

void Image::save(const String& path, bool is_xml)
{
	OSTD_TRACE("%s(%s)", __FUNCTION__, path);
	String tmp;
	if(is_xml)
	{
		Xml xml;
		String path_img(hlp->extractPathFromFullPath(path));
		HXML hroot(xml.node(xml.root(), "image", ""));
		xml.attr<String>(hroot, "type", hlp->cnvString<Image::Types>(type(), m_img_types, ""), true);
		xml.attr<String>(hroot, "format", hlp->cnvString<ImgIO::ImageFormats>(format(), m_img_fmts, ""), true);
		xml.attr<uint_t>(hroot, "count_mips", mips(), true);
		// текстуры
		auto n(map.getNull());
		while(n = map.value(n))
		{
			ImgTexture* tex(n->value);
			tmp.format("%s%s.%s.tga", path_img, name(), n->key);
			save(n->key, tmp, ImgIO::TypesIO::Tga, ImgIO::ImageFormats::BGRA8);
			HXML hnode(xml.node(hroot, "tex", ""));
			xml.attr(hnode, "path", tmp, true);
			xml.attr(hnode, "name", n->key, true);
			xml.attr(hnode, "mip", tex->numMip, true);
			xml.attr(hnode, "layer", tex->numLayer, true);
			if(tex->getHeightSyms())
			{
				HXML hsym;
				xml.attr<uint_t>(hnode, "hsyms", tex->getHeightSyms(), true);
				for(uint_t j = 0; j < 768; j++)
				{
					if(tex->fontSyms[j].is_empty()) continue;
					if((hsym = xml.node(hnode, "sym")))
					{
						xml.attr<uint_t>(hsym, "num", j, true);
						xml.attr<String>(hsym, "coords", hlp->implode<float, false, false, false>(",", tex->fontSyms[j], 4, nullptr, nullptr, false), true);
					}
				}
			}
		}
		xml.save(path);
	}
	else
	{
		Range<uint_t> rangeTex(8192, 8192);
		Ptr<BYTE> texture(make(rangeTex));
		uint_t sz_tmp(0);
		// установить количество текстур
		head.count = count(false);
		// 1. подсчитать размер
		auto n(map.getNull());
		while(n = map.value(n))
		{
			ImgTexture* tex(n->value);
			if(tex->numMip) continue;
			sz_tmp += sizeof(uint_t) + ((n->key.length() + 1) + sizeof(Bar<uint_t>) + sizeof(Bar<float>));
			if(tex->getHeightSyms()) sz_tmp += 768 * sizeof(Bar<float>);
		}
		Ptr<BYTE> buf(sz_tmp);
		BYTE* pbuf((BYTE*)buf);
		// 2. текстуры
		auto nt(map.getNull());
		while(nt = map.value(nt))
		{
			ImgTexture* tex(nt->value);
			if(tex->numMip) continue;
			OSTD_MEMCPY(pbuf, nt->key, nt->key.length() + 1); pbuf += (nt->key.length() + 1);
			*(Bar<uint_t>*)pbuf = tex->getBar(); pbuf += sizeof(Bar<uint_t>);
			*(Bar<float>*)pbuf = tex->getFBar(); pbuf += sizeof(Bar<float>);
			*(uint_t*)pbuf = tex->getHeightSyms(); pbuf += sizeof(uint_t);
			if(tex->getHeightSyms())
			{
				OSTD_MEMCPY(pbuf, tex->fontSyms, 768 * sizeof(Bar<float>));
				pbuf += (768 * sizeof(Bar<float>));
			}
		}
		// 3. записать
		File file(path, File::create_write);
		file.write(&head, sizeof(HEAD_IMAGE));
		file.write(buf, buf.size());
		file.write(texture, texture.size());
	}
}

// <image type="" format="" count_mips="">
//	<img name="" path="" hsyms="" mip="" layer="">
//		<sym num="" coords="" />
//		........................
//	</img>
//	..........................................
// </image>

void Image::make(const Ptr<BYTE>& buf, const String& path)
{
	OSTD_TRACE(__FUNCTION__);
	if(path.is_empty())
	{
		// двоичный файл
		// 1. заголовок и структура
		memcpy(&head, buf, sizeof(HEAD_IMAGE));
		BYTE* tmp(buf + sizeof(HEAD_IMAGE));
		// 2. формируем текстуры
		for(uint_t i = 0; i < head.count; i++)
		{
			// для каждой текстуры (name, bar, fbar, hsym, [syms])
			String name((PCC)tmp); tmp += (name.length() + 1);
			map[name] = new ImgTexture(Bar<uint_t>((uint_t*)tmp), Bar<float>((float*)(tmp + 32)),
									   Ptr<Bar<float>>((Bar<float>*)(tmp + 64), (*(uint_t*)(tmp + 48) ? 768 : 0), Ptr<Bar<float>>::opsMove), *(uint_t*)(tmp + 48), *(uint_t*)(tmp + 56));
			tmp += (64 + (*(uint_t*)(tmp + 56) ? 768 * 16 : 0));
		}
		// 3. установить буфер уже сформированной текстуры
		head.buffer.move(tmp, (buf.size() - (tmp - (buf + sizeof(HEAD_IMAGE)))), Ptr<BYTE>::opsMove);
		// как то убрать удаление этого буфера!!!!
		return;
	}
	Xml xml(path);
	String def;
	HXML hroot(xml.node(xml.root(), "image")), hnode, hsym, hops;
	head.fmt = hlp->cnvValue<ImgIO::ImageFormats>(xml.attr(hroot, "format", def), m_img_fmts, ImgIO::ImageFormats::BGRA8);
	head.type = hlp->cnvValue<Image::Types>(xml.attr(hroot, "type", def), m_img_types, Image::Types::Texture);
	head.mips = xml.attr<uint_t>(hroot, "count_mips", 1);
	// пройтись по всем узлам
	uint_t idx(0);
	while(hnode = xml.node(hroot, nullptr, idx++))
	{
		ImgTexture* tex(nullptr);
		String name, path;
		// имя
		if((name = xml.attr(hnode, "name", def)).is_empty()) continue;
		// путь
		if((path = xml.attr(hnode, "path", def)).is_empty())
		{
			uint_t idx(0);
			while(hops = xml.node(hnode, nullptr, idx++))
			{
				uint_t tmp[4];
				bool is_syms;
				switch(hlp->cnvValue<Commands>(xml.name(hops), m_img_cmds, Commands::None))
				{
					case Commands::None:
						break;
					case Commands::Open:
						tex = addImg(name, xml.attr(hops, "path", def), xml.attr(hops, "mip", 0), xml.attr(hops, "layer", -1));
						break;
					case Commands::Save:
						save(name, xml.attr(hops, "path", def), hlp->cnvValue<ImgIO::TypesIO>(xml.attr(hops, "type", def), m_img_typesIO, ImgIO::TypesIO::Dds),
							 hlp->cnvValue<ImgIO::ImageFormats>(xml.attr(hops, "format", def), m_img_fmts, ImgIO::ImageFormats::BC1));
						break;
					case Commands::Duplicate:
						duplicate(name, xml.attr(hops, "name", def), xml.attr(hops, "mip", 0), xml.attr(hops, "layer", -1));
						break;
					case Commands::Font:
						static BYTE syms[32];
						is_syms = xml.is_attr(hops, "syms");
						if(is_syms) hlp->explode<BYTE>(",", xml.attr(hops, "syms", def), syms, 32, 0xff);
						tex = addFont(name, xml.attr(hops, "face", def), xml.attr(hops, "height", 10), hlp->cnvValue<DWORD>(xml.attr<String>(hops, "charset", "ANSI"), m_img_charset, ANSI_CHARSET),
									  hlp->cnvValue<DWORD>(xml.attr<String>(hops, "family", "DONTCARE"), m_img_family, FF_DONTCARE), is_syms ? syms : nullptr, xml.attr(hops, "mip", 0), xml.attr(hops, "layer", -1));
						break;
					case Commands::Empty:
						tex = addEmpty(name, xml.attr(hops, "width", 1), xml.attr(hops, "height", 1), xml.attr(hops, "mip", 0), xml.attr(hops, "layer", -1));
						break;
					case Commands::Remove:
						remove(name);
						break;
					case Commands::Draw:
					{
						ImgText text(this, name, xml.attr(hops, "font", def), xml.attr(hops, "msg", def), Bar<uint_t>(hlp->explode<uint_t>(",", xml.attr(hops, "clip", def), tmp, 4, 0)),
									 hlp->cnvValue<ImgText::Aligned>(xml.attr(hops, "flags", def), m_img_aligned, ImgText::Aligned::Left),
									 hlp->cnvValue<ImgModify::Coordinates>(xml.attr<String>(hops, "coord", "@"), m_img_coords, ImgModify::Coordinates::Absolute));
						text.draw(this, name, Pts<uint_t>(hlp->explode<uint_t>(",", xml.attr(hops, "pt", def), tmp, 2, 0)));
					}
					break;
					case Commands::Make:
					{
						uint_t nMips(0);
						// максимальный диапазон(для атласа)
						Range<uint_t> rangeTex(hlp->explode<uint_t>(",", xml.attr(hops, "rangeMax", def), tmp, 2, 8192));
						Ptr<BYTE> texture(make(rangeTex, &nMips));
						if(!xml.attr(hops, "is_save", 0)) break;
						saveDds(xml.attr<String>(hops, "path", "c:\\tmp"), texture, rangeTex, format(), nMips);
					}
					break;
					case Commands::Modify:
						ImgModify mod(this, name, &xml, hops);
						mod.apply(this, name);
						break;
				}
			}
		}
		else
		{
			// создаем текстуру
			ImgTexture* tex(addImg(name, path, xml.attr(hnode, "mip", 0), xml.attr(hnode, "layer", -1)));
			// устанавливаем параметры текстуры
			tex->heightSyms = xml.attr(hnode, "hsyms", 0);
			tex->realHeightSyms = xml.attr(hnode, "hsyms2", 0);
			if(tex->getHeightSyms())
			{
				uint_t idx(0);
				// создаем массив символов
				tex->fontSyms = Ptr<Bar<float>>(768);
				// грузим габариты символов
				while(hsym = xml.node(hnode, "sym", idx++))
				{
					int_t num(xml.attr(hsym, "num", -1));
					if(num == -1 || num >= 768) continue;
					hlp->explode<float>(",", xml.attr(hsym, "coords", def), tex->fontSyms[num], 4, 0.0f);
				}
			}
		}
	}
}

Bar<uint_t>* Image::clipBar(const Bar<uint_t>& bar, const Bar<uint_t>& clip, Bar<uint_t>* out)
{
	return asmGetClipBar(bar, out, clip);
}
*/
