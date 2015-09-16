
#include "stdafx.h"
#include "ssh_img.h"

namespace ssh
{
	// форматы файлов
	SSH_ENUMS(m_img_cnv_fmts, _E(tga, ImgCnv::Types), _E(dds, ImgCnv::Types), _E(bmp, ImgCnv::Types), _E(gif, ImgCnv::Types), _E(jpg, ImgCnv::Types), _E(fse, ImgCnv::Types), _E(bfs, ImgCnv::Types));
	// комманды
	SSH_ENUMS(m_img_cmds,	_E(none, Image::Cmds), _E(make, Image::Cmds), _E(open, Image::Cmds), _E(save, Image::Cmds), _E(modify, Image::Cmds), _E(remove, Image::Cmds), _E(font, Image::Cmds),\
							_E(empty, Image::Cmds), _E(duplicate, Image::Cmds), _E(draw, Image::Cmds), _E(packed, Image::Cmds));
	// тип карт
	SSH_ENUMS(m_img_maps,	_E(AtlasMap, Image::TypesMap), _E(CubeMap, Image::TypesMap), _E(VolumeMap, Image::TypesMap), _E(TextureMap, Image::TypesMap), _E(ArrayMap, Image::TypesMap));
	// форматы карт
	SSH_ENUMS(m_img_fmts,	_E(bc1, FormatsMap), _E(bc2, FormatsMap), _E(bc3, FormatsMap), _E(a8, FormatsMap), _E(l8, FormatsMap), _E(rgba8, FormatsMap), _E(bgra8, FormatsMap), _E(rgb8, FormatsMap),\
							_E(bgr8, FormatsMap), _E(r5g6b5, FormatsMap), _E(rgba4, FormatsMap), _E(rgb5a1, FormatsMap));
	// модификаторы
	SSH_ENUMS(m_img_mods,	_E(flip, ImgMod::Types), _E(copy, ImgMod::Types), _E(border, ImgMod::Types), _E(resize, ImgMod::Types), _E(noise, ImgMod::Types), _E(correct, ImgMod::Types),\
							_E(mosaik, ImgMod::Types), _E(figure, ImgMod::Types), _E(gradient, ImgMod::Types), _E(replace, ImgMod::Types), _E(histogramm, ImgMod::Types));
	// фильтры
	SSH_ENUMS(m_mod_flts,	_E(none, ImgMod::Flt), _E(sobel, ImgMod::Flt), _E(laplacian, ImgMod::Flt), _E(prewit, ImgMod::Flt), _E(emboss, ImgMod::Flt), _E(normal, ImgMod::Flt), _E(hi, ImgMod::Flt), _E(low, ImgMod::Flt),\
							_E(median, ImgMod::Flt), _E(roberts, ImgMod::Flt), _E(max, ImgMod::Flt), _E(min, ImgMod::Flt), _E(contrast, ImgMod::Flt), _E(binary, ImgMod::Flt), _E(gamma, ImgMod::Flt), _E(scale_bias, ImgMod::Flt));
	// фигуры
	SSH_ENUMS(m_mod_figures,	_E(ellipse, ImgMod::Figures), _E(rectangle, ImgMod::Figures), _E(tri_u, ImgMod::Figures), _E(tri_d, ImgMod::Figures), _E(tri_r, ImgMod::Figures), _E(tri_l, ImgMod::Figures),\
								_E(sixangle, ImgMod::Figures), _E(eightangle, ImgMod::Figures), _E(romb, ImgMod::Figures), _E(star1, ImgMod::Figures), _E(star2, ImgMod::Figures), _E(arrow_r, ImgMod::Figures),\
								_E(arrow_l, ImgMod::Figures), _E(arrow_d, ImgMod::Figures), _E(arrow_u, ImgMod::Figures), _E(cross_diag, ImgMod::Figures), _E(checked, ImgMod::Figures), _E(vplz, ImgMod::Figures),\
								_E(hplz, ImgMod::Figures), _E(plus, ImgMod::Figures));
	// типы координат
	static ENUM_DATA m_mod_coords[] = { { L"@", SSH_CAST(ImgMod::Coord::absolute) }, { L"%", SSH_CAST(ImgMod::Coord::percent) }, { nullptr, 0 } };
	// типы адресаций
	SSH_ENUMS(m_mod_addrs, _E(lclamp, ImgMod::Addr), _E(lmirror, ImgMod::Addr), _E(lrepeat, ImgMod::Addr), _E(nclamp, ImgMod::Addr), _E(nmirror, ImgMod::Addr), _E(nrepeat, ImgMod::Addr));
	// границы
	SSH_ENUMS(m_img_borders, _E(left, ImgMod::Borders), _E(right, ImgMod::Borders), _E(top, ImgMod::Borders), _E(bottom, ImgMod::Borders), _E(all, ImgMod::Borders));
	// гистограммы
	SSH_ENUMS(m_img_histogramms, _E(rgb, ImgMod::Histogramms), _E(red, ImgMod::Histogramms), _E(green, ImgMod::Histogramms), _E(blue, ImgMod::Histogramms), _E(rgb_v, ImgMod::Histogramms), _E(red_v, ImgMod::Histogramms),\
								 _E(green_v, ImgMod::Histogramms), _E(blue_v, ImgMod::Histogramms));
	// пиксельные операции
	SSH_ENUMS(m_mod_pix_ops, _E(add, ImgMod::Pix), _E(sub, ImgMod::Pix), _E(set, ImgMod::Pix), _E(xor, ImgMod::Pix), _E(and, ImgMod::Pix), _E(or, ImgMod::Pix), _E(lum, ImgMod::Pix), _E(not, ImgMod::Pix), _E(var_alpha, ImgMod::Pix),\
							 _E(fix_alpha, ImgMod::Pix), _E(mul, ImgMod::Pix), _E(lum_add, ImgMod::Pix), _E(lum_sub, ImgMod::Pix), _E(norm, ImgMod::Pix), _E(pow2, ImgMod::Pix));
	// типы операций
	SSH_ENUMS(m_mod_ops, _E(perlin, ImgMod::Ops), _E(brd_o3d, ImgMod::Ops), _E(brd_i3d, ImgMod::Ops), _E(grp, ImgMod::Ops), _E(h_flip, ImgMod::Ops), _E(v_flip, ImgMod::Ops), _E(flip_90, ImgMod::Ops), _E(tbl_2d, ImgMod::Ops),\
						 _E(tbl_3d, ImgMod::Ops), _E(tbl_grp, ImgMod::Ops), _E(terrain, ImgMod::Ops));
	// выравнивание текста
	SSH_ENUMS(m_txt_aligned, _E(top, ImgTxt::Aligned), _E(left, ImgTxt::Aligned), _E(right, ImgTxt::Aligned), _E(bottom, ImgTxt::Aligned), _E(h_center, ImgTxt::Aligned), _E(v_center, ImgTxt::Aligned), _E(brk, ImgTxt::Aligned));
	// типы ландшафта
	SSH_ENUMS(m_img_terrain, _E(hill , ImgMod::Terrain), _E(island, ImgMod::Terrain), _E(mountain, ImgMod::Terrain), _E(pass, ImgMod::Terrain), _E(pit, ImgMod::Terrain), _E(plain, ImgMod::Terrain), _E(raid, ImgMod::Terrain), _E(valley, ImgMod::Terrain));
	// SSH_ENUMS(m_, _E(, ));
	// SSH_ENUMS(m_, _E(, ));
	// SSH_ENUMS(m_, _E(, ));
	// SSH_ENUMS(m_, _E(, ));

	ImgMod::ImgMod(Image* img, int layer_map, Xml* xml, HXML hroot)
	{
		String def;
		type = (Types)ssh_cnv_value(xml->attr(hroot, L"type", def), m_img_mods, SSH_CAST(Types::undef));
		type_address = (Addr)ssh_cnv_value(xml->attr(hroot, L"type_addr", def), m_mod_addrs, SSH_CAST(Addr::lclamp));
		type_ops = (Ops)ssh_cnv_value(xml->attr(hroot, L"type_ops", def), m_mod_ops, SSH_CAST(Ops::none));
		type_coord = (Coord)ssh_cnv_value(xml->attr(hroot, L"coord", def), m_mod_coords, SSH_CAST(Coord::absolute));
		type_filter = (Flt)ssh_cnv_value(xml->attr(hroot, L"filter", def), m_mod_flts, SSH_CAST(Flt::none));
		type_figure = (Figures)ssh_cnv_value(xml->attr(hroot, L"figure", def), m_mod_figures, SSH_CAST(Figures::ellipse));
		type_terrain = (Terrain)ssh_cnv_value(xml->attr(hroot, L"terrain", def), m_img_terrain, SSH_CAST(Terrain::island));
		type_histogramm = (Histogramms)ssh_cnv_value(xml->attr(hroot, L"histogramm", def), m_img_histogramms, SSH_CAST(Histogramms::rgb));
		sides = (Borders)ssh_cnv_value(xml->attr(hroot, L"side_border", def), m_img_borders, SSH_CAST(Borders::all));
		img_rel = xml->attr(hroot, L"rel", 1);
		w_border = xml->attr(hroot, L"w_border", 1);
		w_mtx = xml->attr(hroot, L"mtx", 3);
		radius = xml->attr<int>(hroot, L"radius", 1);
		scale = xml->attr<float>(hroot, L"scale", 1.0f);
		alpha = xml->attr(hroot, L"alpha", 1.0f);
		//shadow = Hex<uint_t>(xml->attr<String>(hroot, "shadow", "0"));
		ssh_explode<Pix>(L",", xml->attr(hroot, L"pix", def), ops, 2, Pix::set, m_mod_pix_ops);
		ssh_explode<int>(L",", xml->attr(hroot, L"bar", def), bar, 4, 0);
		ssh_explode<int>(L",", xml->attr(hroot, L"msk", def), msks, 2, 0x00FFFFFF, nullptr, true),
		ssh_explode<int>(L",", xml->attr(hroot, L"val", def), vals, 2, 0, nullptr, true);
		ssh_explode<int>(L",", xml->attr(hroot, L"col", def), cols_histogramm, 2, 0);
		ssh_explode<int>(L",", xml->attr(hroot, L"cell", def), wh_cell, 2, 1);
		ssh_explode<int>(L",", xml->attr(hroot, L"rn", def), rn, 2, 0);
		ssh_explode<int>(L",", xml->attr(hroot, L"wh", def), wh, 2, 1);
		ssh_explode<int>(L",", xml->attr(hroot, L"array_count", def), array_count, 2, 1);
		ssh_explode<float>(L",", xml->attr(hroot, L"rep", def), wh_rep, 2, 1.0f);
		ssh_explode<float>(L",", xml->attr(hroot, L"vec", def), flt_vec, 4, 1.0f);
		if(xml->is_attr(hroot, L"array_map"))
		{
			const ImgMap* t(img->get_map(xml->attr(hroot, L"array_map", layer_map)));
			if(t)
			{
				rgba = Buffer<ssh_cs>(t->pixels());
				array_count = t->bar().range;
			}
		}
		else
		{
			rgba = Buffer<ssh_cs>(array_count.w * array_count.h);
			ssh_explode<ssh_cs>(L",", xml->attr(hroot, L"array_values", def), rgba, rgba.count(), 0);
		}
		if(xml->is_attr(hroot, L"names"))
		{
			names = Buffer<String>(array_count.w * array_count.h);
			def = xml->attr(hroot, L"array_names", def); ssh_ws* pws(def.buffer());
			Buffer<int> vec(names.count() * 2);
			int count(ssh_split(L',', def, vec, (int)vec.count()));
			String* pnames(names);
			for(int i = 0; i < count; i++)
			{
				pws[vec[i * 2 + 1]] = 0;
				*pnames = pws[vec[i * 2 + 0]];
				pnames++;
			}
		}
	}

	void ImgMod::apply(ImgMap* map)
	{
		try
		{
			if(!map) SSH_THROW(L"Неопределена карта для модификатора!");
			// проверить на корректность параметров
			// 1. области
			ssh_cs* src(map->pixels());
			Range<int> clip(map->bar().range);
			if(bar.is_empty()) bar = clip; else bar = absolute_bar(bar, clip, type_coord);
			switch(type)
			{
				case Types::flip:
					switch(type_ops)
					{
						case Ops::h_flip:
							asm_ssh_h_flip(bar, clip, src);
							break;
						case Ops::v_flip:
							asm_ssh_v_flip(bar, clip, src);
							break;
						case Ops::flip_90:
							Buffer<ssh_cs> dst(map->pix.size());
							//asm_ssh_flip_90(clip, dst, src);
							map->pix = dst;
							map->ixywh = Bar<int>(0, 0, clip.h, clip.w);
							break;
					}
					break;
				case Types::copy:
					asm_ssh_copy(bar, clip, src, rgba, array_count, array_count, this);
					break;
				case Types::border:
					switch(type_ops)
					{
						case Ops::brd_o3d:
						case Ops::brd_i3d:
							//asm_ssh_border_3d(bar, clip, src, w_border, vals.w, msks.w, sides, type_ops);
							break;
						case Ops::grp:
							//asm_ssh_group(bar, clip, src, w_border, vals.w, msks.w, sides, type_ops);
							break;
						case Ops::tbl_2d:
						case Ops::tbl_3d:
						case Ops::tbl_grp:
							//asm_ssh_table(bar, clip, src, rgba, array_count.w, w_border, vals.w, msks.w, ops, type_ops);
							break;
						default:
							//asm_ssh_border2d(bar, clip, src, w_border, vals.w, msks.w, sides, ops.w);
							break;
					}
					break;
				case Types::noise:
					switch(type_ops)
					{
						case Ops::perlin:
							// scale(масштаб в диапазоне 0.005 до 1.0), vals.w(минимальное значение)
							//asm_ssh_noise_perlin(clip, vals.w, src, scale);
							break;
						case Ops::terrain:
							// range(диапазон по высоте), wh(габариты холмов), nRepeat(количество итераций), terrain(тип ланшафта), coordSet(система координат)
							//asm_ssh_noise_terrain(bar, clip, src, vals, wh, ops.w, wh_rep.w);
							break;
					}
					break;
				case Types::correct:
					// range(диапазон значений для обрезки от 0 до 255), histogramm(тип коррекции - отдельно по каналам или сразу по всем)
					//asm_ssh_correct(clip, rn, src, type_histogramm);
					break;
				case Types::resize:
				case Types::mosaik:
				{
					Range<int> _wh(wh);
					if(type_coord == Coord::percent) { _wh.w *= (int)(clip.w / 100.0f); _wh.h *= (int)(clip.h / 100.0f); }
					if(ops.h == Pix::pow2) { _wh.w = ssh_pow2<int>(_wh.w, true); _wh.h = ssh_pow2<int>(_wh.h, true); }
					Buffer<ssh_cs> ptr(asm_ssh_compute_fmt_size(_wh.w, _wh.h, FormatsMap::rgba8));
					if(type == Types::mosaik)
					{
						// rgba(массив слоев изображений в мозаике), array_count(размерность массива), whCells(расстояние между смежными изображениями), imgRel(признак учёта пропорции изображений)
						//asm_ssh_mosaik(_wh, img->cells(), ptr, rgba, array_count, wh_cell, img_rel);
					}
					else
					{
						asm_ssh_copy(_wh, _wh, ptr, src, map->bar(), map->bar().range, this);
					}
					map->pix = ptr;
					map->ixywh = _wh;
				}
				break;
				case Types::figure:
					// vals.h(значение за пределами фигуры), vals.w(значение внутри фигуры), msks.h(маска запределами фигуры), msks.w(маска внутри фигуры), arrayARGB(массив линий(в процентах) для произвольной фигуры),
					// pixOpsEx(операция за пределами фигуры), pixOps(операция внутри фигуры), coords.w(система координат), figure(тип фигуры FigureTypes), radius(радиус углов), shadow(цвет тени - 0 = отсутствие тени)
					//asm_ssh_figure(bar, clip, src, rgba, array_count, vals, msks, ops, type_figure, radius, shadow);
					break;
				case Types::gradient:
					//asm_ssh_gradient(bar, clip, src, vals, msks.w, ops.w, type_address, wh_rep.w);
					break;
				case Types::replace:
					//asm_ssh_replace(vals, msks, src, clip);
					break;
				case Types::histogramm:
				{
					Range<int> tmp(SSH_CAST(type_histogramm) >= SSH_CAST(ImgMod::Histogramms::rgb_v) ? Range<int>(256, 1) : wh);
					Buffer<ssh_cs> buf(tmp.w * tmp.h * 4);
					//asm_ssh_histogramm(tmp, array_count, buf, rgba, cols_histogramm, type_histogramm);
					map->ixywh = tmp;
					map->pix = buf;
				}
				break;
			}
		}
		catch(const Exception& e) { e.add(L""); }
	}
}
