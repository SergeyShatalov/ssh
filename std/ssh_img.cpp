
#include "stdafx.h"
#include "ssh_img.h"
#include "ssh_array.h"

#pragma pack(push, 1)

struct HEADER_TGA
{
	enum TypesTGA : ssh_b
	{
		INDEXED = 1,
		RGB = 2,
		GREY = 3,
		RLE = 8
	};
	enum FlagsTGA : ssh_b
	{
		MASK = 0x30,
		RIGHT = 0x10,
		UPPER = 0x20,
		ALPHA = 0x08
	};
	ssh_b	bIdLength;		//
	ssh_b	bColorMapType;	// тип цветовой карты ()
	ssh_b	bType;			// тип файла ()
	ssh_w	wCmIndex;		// тип индексов в палитре
	ssh_w	wCmLength;		// длина палитры
	ssh_b	bCmEntrySize;	// число бит на элемент палитры
	ssh_w	wXorg;			// 
	ssh_w	wYorg;			// 
	ssh_w	wWidth;			// ширина
	ssh_w	wHeight;		// высота
	ssh_b	bBitesPerPixel;	// бит на пиксель
	ssh_b	bFlags;			// 
};

#pragma pack(pop)

namespace ssh
{
	Image::Image(TypesMap _tp, FormatsMap _fmt, int width, int height, bool _mips) : tp(_tp), fmt(_fmt), wh(ssh_pow2<int>(width, false), ssh_pow2<int>(height, false)), mips(_mips) {}

	int Image::set_map(ssh_wcs path, int layer, int mip)
	{
		SSH_TRACE;
		ImgCnv cnv(path);
		ImgMap* _new_map(nullptr);
		int count(0);
		const ImgCnv::IMAGE* img(nullptr);
		while(img = cnv.enumerate(img == nullptr))
		{
			_new_map = new ImgMap(img->wh, img->pix);
			set_map(_new_map, layer, mip);
			layer++;
			count++;
		}
		return count;
	}

	ImgMap* Image::set_empty(const Range<int>& wh, int layer, int mip)
	{
		SSH_TRACE;
		ImgMap* new_map(new ImgMap(wh, Buffer<ssh_cs>(asm_ssh_compute_fmt_size(wh.w, wh.h, FormatsMap::rgba8))));
		memset(new_map->pixels(), 0, new_map->pixels().size());
		set_map(new_map, layer, mip);
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

	void Image::set_map(ImgMap* nmap, int layer, int mip)
	{
		if(mip != -1)
		{
			ImgMap* map(maps[layer]);
			if(map) map->set_mip(mip, nmap); else delete nmap;
		}
		else maps[layer] = nmap;
	}

	ImgMap* Image::duplicate(int nlayer, int nmip, int olayer, int omip)
	{
		SSH_TRACE;
		ImgMap* old_map(get_map(olayer, omip));
		ImgMap* new_map(get_map(nlayer, nmip));
		if(!old_map) SSH_THROW(L"Карта дупликат <%i, %i> не обнаружена!", olayer, omip);
		if(new_map) SSH_THROW(L"Карта <%i, %i> для дублирования уже существует!", nlayer, nmip);
		Bar<int> new_bar(old_map->ixywh);
		buf_cs new_pix(new_bar.w * new_bar.h * 4);
		memcpy(new_pix, old_map->pix, new_pix.size());
		set_map(new ImgMap(new_bar, new_pix), nlayer, nmip);
		return new_map;
	}

	void Image::flush()
	{
		SSH_TRACE;
		auto n(maps.root());
		while(n)
		{
			auto nn(n->value->mips.root());
			while(nn) { nn->value->pix = Buffer<ssh_cs>();  nn = nn->next; }
			n->value->pix = Buffer<ssh_cs>();
			n = n->next;
		}
	}

	Buffer<ssh_cs> Image::histogramm(int layer, int mip, const Range<int>& wh, ImgMod::Histogramms type, const color & bkg, const color & frg)
	{
		SSH_TRACE;
		ImgMap* map(get_map(layer, mip));
		if(!map) SSH_THROW(L"Карта <%i, %i> не обнаружена!", layer, mip);
		buf_cs buf((SSH_CAST(type) >= SSH_CAST(ImgMod::Histogramms::rgb_v)) ? 1028 : wh.w * wh.h * 4);
		ImgMod modify;
		modify.cols_histogramm.w = bkg.BGRA();
		modify.cols_histogramm.h = frg.BGRA();
		modify.rgba = map->pixels();
		modify.wh = map->ixywh;
		asm_ssh_histogramm(wh, &modify, buf);
		return buf;
	}

	Buffer<ssh_cs> Image::make(ImgMap* map)
	{
		SSH_TRACE;
		int i, c, j;
		Bar<int> null;
		TypesMap type;
		Range<int> _wh;
		ImgMap* tmp_map(nullptr);
		// модификатор по умолчанию
		ImgMod mod_def;
		if(map) { c = 1; type = TypesMap::TextureMap; } else { c = count_layers_from_type(); type = tp; }
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
				ImgMap* map_mip(nullptr);
				Range<int> mip_wh(_wh.w >> i, _wh.h >> i);
				sz = asm_ssh_compute_fmt_size(mip_wh.w, mip_wh.h, fmt);
				for(j = 0; j < c; j++)
				{
					if(!(tmp_map = get_map(j))) SSH_THROW(L"Необнаружена карта на слое <%i>!", j);
					if(i) map_mip = tmp_map->get_mip(i - 1);
					if(!map_mip) map_mip = tmp_map;
					asm_ssh_copy(null, map_mip->bar().range, map_mip->pixels(), rgba, mip_wh, _wh, &mod_def);
					asm_ssh_cnv(fmt, mip_wh, pdst, rgba, 1);
					pdst += sz;
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
			for(j = 0; j < c; j++)
			{
				tmp_map = (map ? map : get_map(j));	// если одна конкрентая карта(или карта мипа)
				if(!tmp_map) SSH_THROW(L"Необнаружена карта на слое <%i>!", j);
				for(i = 0; i < nMip; i++)
				{
					ImgMap* map_mip(nullptr);
					if(i) map_mip = tmp_map->get_mip(i - 1);
					if(!map_mip) map_mip = tmp_map;
					Range<int> mip_wh(_wh.w >> i, _wh.h >> i);
					asm_ssh_copy(null, map_mip->bar().range, map_mip->pixels(), rgba, mip_wh, _wh, &mod_def);
					asm_ssh_cnv(fmt, mip_wh, pdst, rgba, 1);
					pdst += asm_ssh_compute_fmt_size(mip_wh.w, mip_wh.h, fmt);
				}
			}
		}
		return dst;
	}

	int Image::count_layers_from_type() const
	{
		ssh_u c(layers());
		switch(tp)
		{
			case TypesMap::TextureMap:
			case TypesMap::AtlasMap: c = 1; break;
			case TypesMap::CubeMap: c = 6; break;
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
		SSH_TRACE;
		/*
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
		*/
	}

	void Image::make(const Buffer<ssh_cs>& buf)
	{
		SSH_TRACE;
		/*
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
		*/
	}

	bool Image::packed_atlas(Range<int>& rn, int offsXY)
	{
		SSH_TRACE;
		struct MapNode
		{
			MapNode() : type(1) {}
			MapNode(const Bar<int>& bar, int type) : bar(bar), type(type) {}
			Bar<int> bar;
			int type;
		};
		int wmax(0), hmax(0), i, j, tp1, tp2;
		static Bar<int> bar1;
		static Bar<int> bar2;
		// массив отсортированных карт
		Array<ImgMap*, SSH_TYPE> sortw;
		// массив узлов, свободных в глобальной области 
		Array<MapNode*> nodes;
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
		for(i = 0; i < sortw.size(); i++)
		{
			int h1(sortw[i]->ixywh.h), _i(i);
			for(j = i + 1; j < sortw.size(); j++)
			{
				int h2(sortw[j]->ixywh.h);
				if(h2 > h1) { h1 = h2; _i = j; }
			}
			if(_i != i) ssh_swap<ImgMap*>(sortw[_i], sortw[i]);
		}
		for(j = 0; j < sortw.size(); j++)
		{
			ImgMap* tex(sortw[j]);
			// 2. упаковать
			Bar<int>* barA(&tex->ixywh);
			int aw(barA->w + offsXY), ah(barA->h + offsXY);
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
					wmax = ssh_max<int>(wmax, tex->bar().right());
					hmax = ssh_max<int>(hmax, tex->bar().bottom());
					tex = nullptr;
					break;
				}
			}
			if(tex) return false;
		}
		rn.set(wmax, hmax);
		return true;
	}

	ImgTxt* Image::set_font(ssh_wcs name, ssh_wcs face, ssh_w* groups, int height, int layer, int mip)
	{
		static ssh_w groups_def[] = {0x20, 0x7f, 0x410, 0x4ff, 0x10A0, 0x10FF, 0xffff, 0xffff};
		HFONT hFont[3];
		HBITMAP hBmp;
		HDC hdc;
		SIZE sz;
		ssh_b* *ptmp(nullptr);
		ImgTxt* txt(nullptr);
		int count_chars(0), i(0), j, k(0), nTexHW, volume(0), x(0), y(0), w, wmax(0);
		try
		{
			// 1. создаем контекст устройства
			if(!(hdc = ::CreateCompatibleDC(nullptr))) throw(-1);
			// 2. создаем шрифты
			if(!(hFont[0] = ::CreateFont(height, 0, 0, 0, FW_MEDIUM, false, false, false, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FF_DONTCARE | DEFAULT_PITCH, face))) throw(-1);
			if(!(hFont[1] = ::CreateFont(height, 0, 0, 0, FW_BOLD, false, false, false, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FF_DONTCARE | DEFAULT_PITCH, face))) throw(-1);
			if(!(hFont[2] = ::CreateFont(height, 0, 0, 0, FW_NORMAL, true, false, false, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FF_DONTCARE | DEFAULT_PITCH, face))) throw(-1);
			// 3. опереляем реальную высоту символов
			::SetMapMode(hdc, MM_TEXT);
			height = -MulDiv(height, ::GetDeviceCaps(hdc, LOGPIXELSY), 72);
			// 4. определяем количество символов и их объем
			Buffer<ssh_w> remap(65536);
			memset(remap, -1, 65536 * 2);
			if(!groups) groups = groups_def;
			while(groups[i * 2] != 0xffff || count_chars >= 0xffff)
			{
				int f(groups[i * 2 + 0]), l(groups[i * 2 + 1]);
				if(f >= l) break;
				count_chars += (l - f);
				for(; f < l; f++)
				{
					for(j = 0; j < 3; j++)
					{
						::SelectObject(hdc, hFont[j]);
						GetTextExtentPoint32(hdc, (ssh_ws*)&f, 1, &sz);
						// вычисляем суммарную площадь всех символов
						w = sz.cx;
						volume += (w * w + height * height);
					}
				}
				i++;
			}
			// 5. приблизительно определяем габариты битмапа и создаем массив позиций символов
			nTexHW = (volume >= 8388608 ? 4096 : volume >= 2097152 ? 2048 : volume >= 524288 ? 1024 : 512);
			Buffer<Bar<int>> pos(count_chars * 3);
			buf_cs tpix(nTexHW * nTexHW * 4);
			memset(tpix, 0, nTexHW * nTexHW * 4);
			// создаем битмар для рендеринга
			BITMAPINFO bmi{ { sizeof(BITMAPINFOHEADER), 128, -128, 1, (WORD)32, BI_RGB, 0, 0, 0, 0, 0 },{ 0, 0, 0, 0 } };
			if(!(hBmp = ::CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (void**)&ptmp, nullptr, 0))) throw(-1);
			// 7. параметры отрисовки, выравнивание, основной цвет (белый)
			::SelectObject(hdc, hBmp);
			::SetTextAlign(hdc, TA_LEFT);
			::SetTextColor(hdc, RGB(255, 255, 255));
			// 8. создаем символы
			i = 0;
			while(groups[i * 2] != 0xffff)
			{
				int f(groups[i * 2 + 0]), l(groups[i * 2 + 1]);
				if(f >= l) break;
				for(; f < l; f++)
				{
					remap[f] = k;
					for(j = 0; j < 3; j++)
					{
						// выбираем шрифт
						::SelectObject(hdc, hFont[j]);
						GetTextExtentPoint32(hdc, (ssh_ws*)&f, 1, &sz);
						if((x + (sz.cx + sz.cx / 2)) >= nTexHW) { x = 0; y += height; }
						// рисуем "засвеченный" символ
						::SetBkColor(hdc, RGB(255, 255, 255));
						TextOut(hdc, 0, 0, (ssh_ws*)&f, 1);
						// определяем ширину символа
						w = asm_ssh_compute_width_wchar(ptmp);
						// рисуем нормальный символ
						::SetBkColor(hdc, RGB(0, 0, 0));
						TextOut(hdc, 0, 0, (ssh_ws*)&f, 1);
						// копируем символ
						pos[k + j * count_chars].set(x, y, sz.cx, w);
						asm_ssh_copy_wchar(pos[k + j * count_chars], sz.cy, tpix, ptmp, nTexHW * 4);
						x += w + 1;
						wmax = ssh_max<int>(wmax, x);
						memset(ptmp, 0, 128 * 128 * 4);
					}
					k++;
				}
				i++;
			}
			// 9. создаем текстуру
			Range<int> rn(wmax, y + height);
			buf_cs pix(rn.w * rn.h * 4);
			// 10. скопировать в новый буфер, оптимального размера
			ssh_cs* dst(pix), *src(tpix);
			for(i = 0; i < rn.h; i++)
			{
				memcpy(dst, src, rn.w * 4);
				dst += rn.w * 4;
				src += nTexHW * 4;
			}
			// 11. создаем шрифт
			set_map((txt = new ImgTxt(name, height, rn, pos, remap, pix)), layer, mip);
		}
		catch(...) { }
		// освобождаем ресурсы
		::DeleteObject(hBmp);
		::DeleteObject(hdc);
		::DeleteObject(hFont[0]);
		::DeleteObject(hFont[1]);
		::DeleteObject(hFont[2]);
		return txt;
	}
}

/*

QUAD Image::quad(int layer, const Pts<int>& pt, const Bar<int>& clip, const Range<int>& screen, const color & col)
{
ImgMap* map(get_map(layer));
if(!map) SSH_THROW(L"Карта <%i> не обнаружена!", layer);
return *asm_ssh_make_quad(map->fxywh, Bar<int>(pt, map->ixywh.range), clip, screen, col.BGRA());
}
*/
