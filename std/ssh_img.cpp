
#include "stdafx.h"
#include "ssh_img.h"
#include "ssh_array.h"
#include "ssh_xml.h"

namespace ssh
{
#pragma warning(push)
#pragma warning(disable:4005)
	// форматы файлов
#define ssh_enum_tp ImgCnv::Types
	SSH_ENUMS(m_img_cnv_fmts, _E(tga), _E(dds), _E(bmp), _E(gif), _E(jpg), _E(fse), _E(bfs), _E(psd), _E(png));
	// комманды
#define ssh_enum_tp Image::Cmds
	SSH_ENUMS(m_img_cmds, _E(none), _E(make), _E(open), _E(save), _E(modify), _E(remove), _E(font), _E(empty), _E(duplicate), _E(draw));
	// тип карт
#define ssh_enum_tp Image::TypesMap
	SSH_ENUMS(m_img_maps, _E(AtlasMap), _E(CubeMap), _E(VolumeMap), _E(TextureMap), _E(ArrayMap));
	// форматы карт
#define ssh_enum_tp FormatsMap
	SSH_ENUMS(m_img_fmts, _E(bc1), _E(bc2), _E(bc3), _E(a8), _E(l8), _E(rgba8), _E(bgra8), _E(rgb8), _E(bgr8), _E(r5g6b5), _E(rgba4), _E(rgb5a1));
#pragma warning(pop)

	static int get_approximate_area(int volume)
	{
		return (volume >= 33554432 ? 8192 : volume >= 8388608 ? 4096 : volume >= 2097152 ? 2048 : volume >= 524288 ? 1024 : 512);
	}

	Image::Image(TypesMap _tp, FormatsMap _fmt, int width, int height)
	{
		head.tp = _tp;
		head.fmt = _fmt;
		head.wh.set(width, height);
	}

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

	ImgMap* Image::set_empty(const Range<int>& wh, int layer, Buffer<ssh_cs>* buf, int mip)
	{
		SSH_TRACE;
		ImgMap* new_map(new ImgMap(wh, Buffer<ssh_cs>(asm_ssh_compute_fmt_size(wh.w, wh.h, FormatsMap::rgba8))));
		if(buf) memcpy(new_map->pixels(), buf->to<ssh_cs>(), new_map->pixels().size());  else memset(new_map->pixels(), 0, new_map->pixels().size());
		set_map(new_map, layer, mip);
		return new_map;
	}

	void Image::del_map(int layer, int mip)
	{
		if(mip > 0)
		{
			ImgMap* map(maps[layer]);
			if(map) map->del_mip(mip);
		}
		else maps[layer] = nullptr;
	}

	ImgMap* Image::get_map(int layer, int mip)
	{
		ImgMap* map(maps[layer]);
		if(mip > 0) { if(map) map = map->get_mip(mip); }
		return map;
	}

	void Image::set_map(ImgMap* nmap, int layer, int mip)
	{
		if(mip > 0)
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
		if(!old_map) SSH_THROW(L"Карта дубликат <%i, %i> не обнаружена!", olayer, omip);
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

	Buffer<ssh_cs> Image::histogramm(const Range<int>& wh, ImgMod::Histogramms type, const color & bkg, const color & frg, int layer, int mip)
	{
		SSH_TRACE;
		ImgMap* map(get_map(layer, mip));
		if(!map) SSH_THROW(L"Карта <%i, %i> не обнаружена!", layer, mip);
		buf_cs buf((SSH_CAST(type) >= SSH_CAST(ImgMod::Histogramms::rgb_v)) ? 1024 + 8 : wh.w * wh.h * 4);
		ImgMod modify;
		modify.cols_histogramm.w = bkg.BGRA();
		modify.cols_histogramm.h = frg.BGRA();
		modify.rgba = buf_cs(map->pix, map->pix.count(), false);
		modify.wh_rgba = map->ixywh;
		asm_ssh_histogramm(wh, &modify, buf);
		return buf;
	}

	Buffer<ssh_cs> Image::make(ImgMap* map)
	{
		SSH_TRACE;
		int i, c, j;
		Bar<int> null;
		TypesMap type;
		ImgMap* tmp_map(nullptr);
		// модификатор по умолчанию
		ImgMod mod_def;
		if(map) { c = 1; type = TypesMap::TextureMap; } else { c = count_layers_from_type(); type = head.tp; }
		// определить количество мипуровней и габариты текстуры
		head.mip = get_mips(map, &head.wh);
		// расчитать размер буфера исходного формата
		Buffer<ssh_cs> rgba(head.wh.w * head.wh.h * 4);
		// расчитать размер буфера целевого формата
		ssh_u sz(0);
		for(i = 0; i < head.mip; i++) sz += asm_ssh_compute_fmt_size(head.wh.w >> i, head.wh.h >> i, head.fmt);
		sz *= c;
		head.tex = buf_cs(sz);
		ssh_cs* pdst(head.tex);
		if(type == TypesMap::VolumeMap)
		{
			for(i = 0; i < head.mip; i++)
			{
				ImgMap* map_mip(nullptr);
				Range<int> mip_wh(head.wh.w >> i, head.wh.h >> i);
				sz = asm_ssh_compute_fmt_size(mip_wh.w, mip_wh.h, head.fmt);
				for(j = 0; j < c; j++)
				{
					if(!(tmp_map = get_map(j))) SSH_THROW(L"Необнаружена карта на слое <%i>!", j);
					if(!(map_mip = tmp_map->get_mip(i))) map_mip = tmp_map;
					asm_ssh_copy(null, map_mip->bar().range, map_mip->pixels(), rgba, mip_wh, head.wh, &mod_def);
					asm_ssh_cnv(head.fmt, mip_wh, pdst, rgba, 1);
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
				asm_ssh_copy(tmp_map->bar().range, tmp_map->bar().range, tmp_map->pixels(), rgba, tmp_map->bar(), head.wh, &mod_def);
				n = n->next;
			}
			asm_ssh_cnv(head.fmt, head.wh, pdst, rgba, 1);
		}
		else
		{
			for(j = 0; j < c; j++)
			{
				tmp_map = (map ? map : get_map(j));	// если одна конкрентая карта(или карта мипа)
				if(!tmp_map) SSH_THROW(L"Необнаружена карта на слое <%i>!", j);
				for(i = 0; i < head.mip; i++)
				{
					ImgMap* map_mip(nullptr);
					if(!(map_mip = tmp_map->get_mip(i))) map_mip = tmp_map;
					Range<int> mip_wh(head.wh.w >> i, head.wh.h >> i);
					asm_ssh_copy(null, map_mip->bar().range, map_mip->pixels(), rgba, mip_wh, head.wh, &mod_def);
					asm_ssh_cnv(head.fmt, mip_wh, pdst, rgba, 1);
					pdst += asm_ssh_compute_fmt_size(mip_wh.w, mip_wh.h, head.fmt);
				}
			}
		}
		return head.tex;
	}

	int Image::count_layers_from_type() const
	{
		ssh_u c(layers());
		switch(head.tp)
		{
			case TypesMap::TextureMap:
			case TypesMap::AtlasMap: c = 1; break;
			case TypesMap::CubeMap: c = 6; break;
			case TypesMap::ArrayMap:
			case TypesMap::VolumeMap: break;
		}
		return (int)c;
	}

	int Image::get_mips(const ImgMap* map, Range<int>* wh)
	{
		int nMips(1);
		int limit;
		// определить реальные габариты изображения
		Range<int> _wh(range(map));
		if(wh) *wh = _wh;
		if(head.tp != TypesMap::AtlasMap) while(asm_ssh_compute_fmt_size(_wh.w >> nMips, _wh.h >> nMips, head.fmt, &limit) && !limit) nMips++;
		return nMips;
	}

	Range<int> Image::range(const ImgMap* map)
	{
		Range<int> _wh, tmp;
		int volume(0);
		if(!map)
		{
			_wh.set(INT_MAX, INT_MAX);
			// определить минимальную ширину и высоту, а также общую площадь всех слоев
			auto n(maps.root());
			while(n)
			{
				tmp = n->value->bar();
				_wh.w = ssh_min<int>(_wh.w, tmp.w);
				_wh.h = ssh_min<int>(_wh.h, tmp.h);
				volume += (tmp.w * tmp.w + tmp.h * tmp.h);
				n = n->next;
			}
		}
		else _wh = map->bar();
		if(head.tp != TypesMap::AtlasMap)
		{
			if(head.wh.is_null()) _wh = head.wh;
			_wh.set(ssh_pow2<int>(_wh.w, false), ssh_pow2<int>(_wh.h, false));
		}
		else
		{
			// определяем приблизительные габариты атласа
			volume = get_approximate_area(volume);
			if(head.wh.is_null()) tmp.set(volume, volume); else tmp = head.wh;
			// упаковть в атлас и определить его габариты
			while(_wh = tmp, !packed_atlas(_wh)) tmp.w <<= 1, tmp.h <<= 1;
		}
		return _wh;
	}

	void Image::save(ssh_wcs path, bool is_xml)
	{
		SSH_TRACE;
		String base_path(ssh_file_path(path) + name());
		if(is_xml)
		{
			Xml xml;
			HXML hroot(xml.add_node(xml.root(), L"image", L""));
			xml.set_attr(hroot, L"type", ssh_cnv_string(SSH_CAST(type()), m_img_maps, L""));
			xml.set_attr(hroot, L"format", ssh_cnv_string(SSH_CAST(format()), m_img_fmts, L""));
			xml.set_attr(hroot, L"mips", head.mip);
			xml.set_attr(hroot, L"wh", ssh_implode<int>(L",", head.wh, 2, L"0", nullptr, false, false));
			// слои и мипы
			// создаем вложенную папку для файлов
			String tmp;
			ssh_make_path(base_path, false);
			auto n(maps.root());
			while(n)
			{
				// базовое изображение
				save(tmp.fmt(L"%s\\map_%i.0.tga", base_path, n->key), ImgCnv::Types::tga, FormatsMap::rgba8, n->key);
				HXML hnode(xml.add_node(hroot, L"map", L""));
				xml.set_attr(hnode, L"path", tmp);
				xml.set_attr(hnode, L"layer", n->key);
				if(n->value->is_font())
				{
					// если карта - это шрифт
					ImgTxt* map((ImgTxt*)n->value);
					xml.set_attr(hnode, L"name", map->name);
					xml.set_attr(hnode, L"face", map->face);
					xml.set_attr(hnode, L"height", map->height);
					tmp.empty();
					// определить диапазоны символов
					int ii(0);
					bool is(false);
					for(int i = 0; i < map->remap.count(); i++)
					{
						if(map->remap[i] == 0xffff)
						{
							if(is)
							{
								if(!tmp.is_empty()) tmp += L",";
								tmp += String(ii + map->min_ws, String::Radix::_hex) + L"," + String(i + map->min_ws, String::Radix::_hex);
								is = false;
							}
						}
						else if(!is) { ii = i; is = true; }
					}
					tmp += L"FFFF,FFFF";
					xml.set_attr(hnode, L"groups", tmp);
				}
				else
				{
					// мип уровни
					auto m(n->value->mips.root());
					while(m)
					{
						HXML hmip(xml.add_node(hnode, L"mip", L""));
						save(tmp.fmt(L"%s\\map_%i.%i.tga", base_path, n->key, m->key), ImgCnv::Types::tga, FormatsMap::rgba8, n->key, m->key);
						xml.set_attr(hmip, L"path", tmp);
						xml.set_attr(hmip, L"mip", m->key);
						m = m->next;
					}
				}
				n = n->next;
			}
			xml.save(base_path + L".img", L"utf-8");
		}
		else
		{
			// создать текстуру
			make();
			// определить размер базовых параметров
			ssh_u sz(0);
			head.count = layers();
			if(type() == TypesMap::AtlasMap)
			{
				// определить размер буфера под все карты(если это атлас)
				auto n(maps.root());
				while(n)
				{
					auto v(n->value);
					// слой, габариты, плавающие габариты
					sz += (sizeof(v->ixywh) + sizeof(v->fxywh) + sizeof(int));
					if(v->is_font())
					{
						ImgTxt* t((ImgTxt*)v);
						// если шрифт - высота, мин. символ, количество габаритов символов, габариты символов, количество кодов ремапа, карта ремапа, имя шрифта, название шрифта
						sz += sizeof(t->height) + sizeof(t->min_ws);
						sz += t->pos.size() + sizeof(int);
						sz += t->remap.size() + sizeof(int);
						sz += (t->name.length() + 1) * sizeof(ssh_ws);
						sz += (t->face.length() + 1) * sizeof(ssh_ws);
					}
					n = n->next;
				}
			}
			buf_cs buf(sz); ssh_cs* pbuf(buf);
			// заголовок
			if(type() == TypesMap::AtlasMap)
			{
				auto n(maps.root());
				while(n)
				{
					auto v(n->value);
					// слой, габариты, плавающие габариты
					*(int*)pbuf = n->key; pbuf += sizeof(int);
					*(Bar<int>*)pbuf = v->ixywh; pbuf += sizeof(Bar<int>);
					*(Bar<float>*)pbuf = v->fxywh; pbuf += sizeof(Bar<float>);
					*(int*)pbuf = 0;
					if(v->is_font())
					{
						ImgTxt* t((ImgTxt*)v);
						// если шрифт - высота, мин. символ, количество габаритов символов, количество кодов ремапа, габариты символов, карта ремапа, имя шрифта, название шрифта
						*(int*)pbuf = t->height; pbuf += sizeof(int);
						*(int*)pbuf = t->min_ws; pbuf += sizeof(int);
						*(int*)pbuf = (int)t->pos.count(); pbuf += sizeof(int);
						*(int*)pbuf = (int)t->remap.count(); pbuf += sizeof(int);
						memcpy(pbuf, t->pos, t->pos.size()); pbuf += t->pos.size();
						memcpy(pbuf, t->remap, t->remap.size()); pbuf += t->remap.size();
						memcpy(pbuf, t->name, (t->name.length() + 1) * 2); pbuf += ((t->name.length() + 1) * 2);
						memcpy(pbuf, t->face, (t->face.length() + 1) * 2); pbuf += ((t->face.length() + 1) * 2);
					} else pbuf += sizeof(int);
					n = n->next;
				}
			}
			File f(base_path + L".img", File::create_write);
			f.write(&head, sizeof(HEADER) - sizeof(ssh_u));
			f.write(buf, buf.size());
			f.write(head.tex, head.tex.size());
		}
	}

	void Image::make(const Buffer<ssh_cs>& buf)
	{
		SSH_TRACE;
		String tmp;
		ssh_cs* pbuf(buf);
		if(*(int*)(pbuf) == 0x01020304)
		{
			memcpy(&head, pbuf, sizeof(HEADER) - sizeof(ssh_u)); pbuf += (sizeof(HEADER) - sizeof(ssh_u));
			if(head.tp == TypesMap::AtlasMap)
			{
				for(int i = 0; i < head.count; i++)
				{
					ImgMap* map;
					ssh_cs* p(pbuf);
					// определяем - это карта или шрифт?
					int layer(*(int*)pbuf); pbuf += (sizeof(int) + sizeof(Bar<int>) + sizeof(Bar<float>));
					if(*(int*)pbuf == 0)
					{
						// карта
						set_map((map = new ImgMap()), layer, 0);
						pbuf += sizeof(int);
					}
					else
					{
						// шрифт
						int height(*(int*)pbuf); pbuf += sizeof(int);
						int min_ws(*(int*)pbuf); pbuf += sizeof(int);
						Buffer<Bar<float>> pos(*(int*)pbuf); pbuf += sizeof(int);
						Buffer<ssh_w> remap(*(int*)pbuf); pbuf += sizeof(int);
						memcpy(pos, pbuf, pos.size()); pbuf += pos.size();
						memcpy(remap, pbuf, remap.size()); pbuf += remap.size();
						String name((ssh_ws*)pbuf); pbuf += ((name.length() + 1) * 2);
						String face((ssh_ws*)pbuf); pbuf += ((face.length() + 1) * 2);
						ImgTxt* txt(new ImgTxt(name, face, height, min_ws, pos, remap));
						map = txt;
					}
					map->ixywh = *(Bar<int>*)p; p += sizeof(Bar<int>);
					map->fxywh = *(Bar<float>*)p; p += sizeof(Bar<float>);
				}
			}
			head.tex = buf_cs(pbuf, head.tex.size(), false);
		}
		else
		{
			Xml xml(buf);
			ssh_l idx(0);
			Range<int> rn;
			HXML hroot(xml.node(xml.root(), L"image")), hnode, hmip;
			head.tp = (TypesMap)ssh_cnv_value(xml.attr(hroot, L"type", tmp), m_img_maps, SSH_CAST(TypesMap::TextureMap));
			head.fmt = (FormatsMap)ssh_cnv_value(xml.attr(hroot, L"format", tmp), m_img_fmts, SSH_CAST(FormatsMap::rgba8));
			head.mip = xml.attr(hroot, L"mips", 0);
			ssh_explode<int>(L",", xml.attr(hroot, L"wh", tmp), head.wh, 2, 0);
			while((hnode = xml.node(hroot, nullptr, idx++)))
			{
				int layer(xml.attr(hnode, L"layer", -1)), mip(xml.attr(hnode, L"mip", 0));
				if(xml.get_name(hnode).lower() != L"map")
				{
					switch((Cmds)ssh_cnv_value(xml.get_name(hnode), m_img_cmds, SSH_CAST(Cmds::none)))
					{
						case Cmds::draw:
							break;
						case Cmds::duplicate:
							duplicate(layer, mip, xml.attr(hnode, L"old_layer", -1), xml.attr(hnode, L"old_mip", 0));
							break;
						case Cmds::empty:
							set_empty(Range<int>(ssh_explode<int>(L",", xml.attr(hnode, L"wh", tmp), rn, 2, 0)), layer, nullptr, mip);
							break;
						case Cmds::font:
							{
								ssh_w groups[64];
								ssh_explode<ssh_w>(L",", xml.attr(hnode, L"groups", tmp), groups, 64, -1, nullptr, true);
								set_font(xml.attr(hnode, L"name", tmp), xml.attr(hnode, L"face", tmp), groups, xml.attr(hnode, L"height", -10), layer);
							}
							break;
						case Cmds::make:
							if(xml.is_attr(hnode, L"is_save"))
							{
								save(xml.attr(hnode, L"path", tmp), ImgCnv::Types::dds, (FormatsMap)ssh_cnv_value(xml.attr(hnode, L"format", tmp), m_img_fmts, SSH_CAST(FormatsMap::rgba8)), layer, mip);
							}
							else
							{
								make();
							}
							break;
						case Cmds::modify:
							ImgMod(&xml, hnode, this).apply(get_map(layer, mip));
							break;
						case Cmds::open:
							set_map(xml.attr(hnode, L"path", tmp), layer, mip);
							break;
						case Cmds::remove:
							del_map(layer, mip);
							break;
						case Cmds::save:
							save(xml.attr(hnode, L"path", tmp), (ImgCnv::Types)ssh_cnv_value(xml.attr(hnode, L"type", tmp), m_img_cnv_fmts, SSH_CAST(ImgCnv::Types::dds)),
								 (FormatsMap)ssh_cnv_value(xml.attr(hnode, L"format", tmp), m_img_fmts, SSH_CAST(FormatsMap::rgba8)), layer, mip);
							break;
					}
				}
				else if(xml.is_attr(hnode, L"name"))
				{
					// шрифт
					ssh_w groups[64];
					ssh_explode<ssh_w>(L",", xml.attr(hnode, L"groups", tmp), groups, 64, -1, nullptr, true);
					set_font(xml.attr(hnode, L"name", tmp), xml.attr(hnode, L"face", tmp), groups, xml.attr(hnode, L"height", -10), xml.attr(hnode, L"layer", 0));
				}
				else
				{
					// карта
					int layer(xml.attr(hnode, L"layer", 0));
					set_map(xml.attr(hnode, L"path", tmp), layer, 0);
					// мипы
					ssh_l mip(0);
					while((hmip = xml.node(hnode, nullptr, mip++))) set_map(xml.attr(hmip, L"path", tmp), layer, xml.attr(hmip, L"mip", 0));
				}
			}
		}
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
		rn.set(ssh_pow2<int>(wmax, false), ssh_pow2<int>(hmax, false));
		// сформировать у всех текстур плавающую область
		n = maps.root();
		while(n) { auto t(n->value); t->set_bar(t->bar(), rn); n = n->next; }
		return true;
	}

	ImgTxt* Image::set_font(ssh_wcs name, ssh_wcs face, ssh_w* groups, int height, int layer)
	{
		static ssh_w groups_def[] = {0x20, 0x7f, 0x410, 0x4ff, 0xffff, 0xffff};
		HFONT hFont[3] = { nullptr, nullptr, nullptr};
		HBITMAP hBmp(nullptr);
		HDC hdc(nullptr);
		SIZE sz;
		ssh_b* *ptmp(nullptr);
		ImgTxt* txt(nullptr);
		int count_chars(0), i(0), j, k(0), nTexHW, volume(0), x(0), y(0), w, wmax(0), ws_min(INT_MAX), ws_max(0);
		try
		{
			// 1. создаем контекст устройства
			if(!(hdc = ::CreateCompatibleDC(nullptr)))
				SSH_THROW(L"Не удалось создать контекст устройства.");
			// 2. создаем шрифты
			if(!(hFont[0] = ::CreateFont(height, 0, 0, 0, FW_MEDIUM, false, false, false, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FF_DONTCARE | DEFAULT_PITCH, face)))
				SSH_THROW(L"Не удалось создать обычный шрифт.");
			if(!(hFont[1] = ::CreateFont(height, 0, 0, 0, FW_BOLD, false, false, false, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FF_DONTCARE | DEFAULT_PITCH, face)))
				SSH_THROW(L"Не удалось создать полужирный шрифт.");
			if(!(hFont[2] = ::CreateFont(height, 0, 0, 0, FW_NORMAL, true, false, false, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FF_DONTCARE | DEFAULT_PITCH, face)))
				SSH_THROW(L"Не удалось создать наклонный шрифт.");
			// 3. опереляем реальную высоту символов
			::SetMapMode(hdc, MM_TEXT);
			height = -MulDiv(height, ::GetDeviceCaps(hdc, LOGPIXELSY), 72);
			// 4. определяем количество символов и их объем
			if(!groups) groups = groups_def;
			while(groups[i * 2] != 0xffff || count_chars >= 0xffff)
			{
				int f(groups[i * 2 + 0]), l(groups[i * 2 + 1]);
				if(f >= l) break;
				if(ws_min > f) ws_min = f;
				if(ws_max < l) ws_max = l;
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
			// 5. приблизительно определяем габариты битмапа(по суммарной площади всех символов) и создаем массив позиций символов
			nTexHW = get_approximate_area(volume);
			ws_max -= ws_min;
			Buffer<ssh_w> remap(ws_max);
			memset(remap, -1, ws_max * 2);
			Buffer<Bar<int>> pos(count_chars * 3);
			buf_cs tpix(nTexHW * nTexHW * 4);
			memset(tpix, 0, nTexHW * nTexHW * 4);
			// 6. создаем битмар для рендеринга
			BITMAPINFO bmi{ { sizeof(BITMAPINFOHEADER), 128, -128, 1, 32, BI_RGB, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0 } };
			if(!(hBmp = ::CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (void**)&ptmp, nullptr, 0)))
				SSH_THROW(L"Не удалось создать битмап рендеринга.");
			// 7. параметры отрисовки - выравнивание, основной цвет (белый)
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
					remap[f - ws_min] = k;
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
			set_map((txt = new ImgTxt(name, face, height, ws_min, rn, pos, remap, pix)), layer, 0);
		}
		catch(const Exception& e)
		{
			e.add(L"Не удалось создать текстуру шрифта <%s>!", name);
		}
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
