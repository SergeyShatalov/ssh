
#include "stdafx.h"
#include "ssh_img.h"

namespace ssh
{
#pragma warning(push)
#pragma warning(disable:4005)
	// типы координат
	static ENUM_DATA m_mod_coords[] = { { L"@", SSH_CAST(ImgMod::Coord::absolute) },{ L"%", SSH_CAST(ImgMod::Coord::percent) },{ nullptr, 0 } };
	// модификаторы
#define ssh_enum_tp ImgMod::Types
	SSH_ENUMS(m_img_mods, _E(flip), _E(copy), _E(border), _E(resize), _E(noise), _E(correct), _E(figure), _E(gradient), _E(replace), _E(histogramm));
	// фильтры
#define ssh_enum_tp ImgMod::Flt
	SSH_ENUMS(m_mod_flts, _E(none), _E(sobel), _E(laplac), _E(prewit), _E(emboss), _E(normal), _E(hi), _E(low), _E(median), _E(roberts), _E(max), _E(min), _E(contrast), _E(binary), _E(gamma), _E(scale_bias));
	// фигуры
#define ssh_enum_tp ImgMod::Figures
	SSH_ENUMS(m_mod_figures, _E(ellipse), _E(rectangle), _E(tri_u), _E(tri_d), _E(tri_r), _E(tri_l), _E(sixangle), _E(eightangle), _E(romb), _E(star1), _E(star2), _E(arrow_r), _E(arrow_l), _E(arrow_d), _E(arrow_u), _E(cross_diag), _E(checked), _E(vplz), _E(hplz), _E(plus));
	// типы адресаций
#define ssh_enum_tp ImgMod::Addr
	SSH_ENUMS(m_mod_addrs, _E(lclamp), _E(lmirror), _E(lrepeat), _E(nclamp), _E(nmirror), _E(nrepeat));
	// границы
#define ssh_enum_tp ImgMod::Borders
	SSH_ENUMS(m_img_borders, _E(left), _E(right), _E(top), _E(bottom), _E(all));
	// гистограммы
#define ssh_enum_tp ImgMod::Histogramms
	SSH_ENUMS(m_img_histogramms, _E(rgb), _E(red), _E(green), _E(blue), _E(rgb_v), _E(red_v), _E(green_v), _E(blue_v));
	// пиксельные операции
#define ssh_enum_tp ImgMod::Pix
	SSH_ENUMS(m_mod_pix_ops, _E(add), _E(sub), _E(set), _E(xor), _E(and), _E(or), _E(lum), _E(not), _E(var_alpha), _E(fix_alpha), _E(mul), _E(lum_add), _E(lum_sub), _E(norm));
	// типы операций
#define ssh_enum_tp ImgMod::Ops
	SSH_ENUMS(m_mod_ops, _E(perlin), _E(brd_o3d), _E(brd_i3d), _E(grp), _E(h_flip), _E(v_flip), _E(flip_90), _E(tbl_2d), _E(tbl_3d), _E(tbl_grp), _E(terrain), _E(pow2));
	// выравнивание текста
#define ssh_enum_tp ImgTxt::Aligned
	SSH_ENUMS(m_txt_aligned, _E(top), _E(left), _E(right), _E(bottom), _E(h_center), _E(v_center), _E(brk));
	// типы ландшафта
#define ssh_enum_tp ImgMod::Terrain
	SSH_ENUMS(m_img_terrain, _E(hill), _E(island), _E(mountain), _E(pass), _E(pit), _E(plain), _E(raid), _E(valley));
	// SSH_ENUMS(, _E());
#pragma warning(pop)

	void ImgMod::make(Xml * xml, HXML hroot, Image * img)
	{
		SSH_TRACE;
		try
		{
			if(!xml || !hroot) SSH_THROW(L"Недопустимые параметры при парсинге модификатора!");
			String def;
			type = (Types)ssh_cnv_value(xml->attr(hroot, L"type", def), m_img_mods, SSH_CAST(Types::undef));
			type_address = (Addr)ssh_cnv_value(xml->attr(hroot, L"addr", def), m_mod_addrs, SSH_CAST(Addr::lclamp));
			type_ops = (Ops)ssh_cnv_value(xml->attr(hroot, L"ops", def), m_mod_ops, SSH_CAST(Ops::none));
			type_coord = (Coord)ssh_cnv_value(xml->attr(hroot, L"coord", def), m_mod_coords, SSH_CAST(Coord::absolute));
			type_filter = (Flt)ssh_cnv_value(xml->attr(hroot, L"filter", def), m_mod_flts, SSH_CAST(Flt::none));
			type_figure = (Figures)ssh_cnv_value(xml->attr(hroot, L"figure", def), m_mod_figures, SSH_CAST(Figures::ellipse));
			type_terrain = (Terrain)ssh_cnv_value(xml->attr(hroot, L"terrain", def), m_img_terrain, SSH_CAST(Terrain::island));
			type_histogramm = (Histogramms)ssh_cnv_value(xml->attr(hroot, L"histogramm", def), m_img_histogramms, SSH_CAST(Histogramms::rgb));
			sides = (Borders)ssh_cnv_value(xml->attr(hroot, L"sides", def), m_img_borders, SSH_CAST(Borders::all));
			aspect = xml->attr(hroot, L"aspect", 1);
			w_border = xml->attr(hroot, L"wbrd", 1);
			w_mtx = xml->attr(hroot, L"mtx", 3);
			radius = xml->attr(hroot, L"radius", 1);
			scale = xml->attr(hroot, L"scale", 1.0f);
			alpha = xml->attr(hroot, L"alpha", 1.0f);
			shadow = xml->attr<String>(hroot, L"shadow", L"0").toNum<int>(0, String::_hex);
			ssh_explode<int>(L",", xml->attr(hroot, L"pix", def), (int*)&ops, 2, SSH_CAST(Pix::set), m_mod_pix_ops);
			ssh_explode<int>(L",", xml->attr(hroot, L"bar", def), bar, 4, 0);
			ssh_explode<int>(L",", xml->attr(hroot, L"msk", def), msks, 2, 0x00FFFFFF, nullptr, true);
			ssh_explode<int>(L",", xml->attr(hroot, L"val", def), vals, 2, 0, nullptr, true);
			ssh_explode<int>(L",", xml->attr(hroot, L"hcol", def), cols_histogramm, 2, 0, nullptr, true);
			ssh_explode<int>(L",", xml->attr(hroot, L"cell", def), wh_cell, 2, 1);
			ssh_explode<int>(L",", xml->attr(hroot, L"wh", def), wh, 2, 1);
			ssh_explode<int>(L",", xml->attr(hroot, L"wh_rgba", def), wh_rgba, 2, 1);
			ssh_explode<float>(L",", xml->attr(hroot, L"rep", def), wh_rep, 2, 1.0f);
			ssh_explode<float>(L",", xml->attr(hroot, L"vec", def), flt_vec, 4, 1.0f);
			if(xml->is_attr(hroot, L"map"))
			{
				ImgMap* t(img->get_map(xml->attr(hroot, L"map", -1)));
				if(t)
				{
					rgba = Buffer<ssh_cs>(t->pixels(), t->pixels().count(), false);
					wh_rgba = t->bar().range;
				}
			}
			else if(xml->is_attr(hroot, L"array"))
			{
				rgba = Buffer<ssh_cs>(wh_rgba.w * wh_rgba.h);
				ssh_explode<ssh_b>(L",", xml->attr(hroot, L"array", def), rgba.to<ssh_b>(), rgba.count(), 0);
			}
		}
		catch(const Exception& e)
		{
			e.add(L"");
		}
	}

	void ImgMod::apply(ImgMap* map)
	{
		SSH_TRACE;
		try
		{
			if(!map) SSH_THROW(L"Неопределена карта для модификатора!");
			// области
			Range<int> clip(map->bar().range);
			if(bar.is_empty()) bar = clip; else bar = absolute_bar(bar, clip, type_coord);
			switch(type)
			{
				case Types::flip:
					switch(type_ops)
					{
						case Ops::h_flip: asm_ssh_h_flip(bar, clip, map->pixels()); break;
						case Ops::v_flip: asm_ssh_v_flip(bar, clip, map->pixels()); break;
						case Ops::flip_90:
							buf_cs dst(map->pix.size());
							asm_ssh_flip_90(clip, dst, map->pixels());
							map->pix = dst;
							map->ixywh = Bar<int>(0, 0, clip.h, clip.w);
							break;
					}
					break;
				case Types::copy: asm_ssh_copy(bar, clip, map->pixels(), rgba, wh_rgba, wh_rgba, this); break;
				case Types::border:
					switch(type_ops)
					{
						case Ops::brd_o3d:
						case Ops::brd_i3d: asm_ssh_border3d(bar, clip, map->pixels(), this); break;
						case Ops::grp: asm_ssh_group(bar, clip, map->pixels(), this); break;
						case Ops::tbl_2d:
						case Ops::tbl_3d:
						case Ops::tbl_grp: asm_ssh_table(bar, clip, map->pixels(), this); break;
						default: asm_ssh_border2d(bar, clip, map->pixels(), this); break;
					}
					break;
				case Types::noise:
					switch(type_ops)
					{
						case Ops::perlin: asm_ssh_noise_perlin(clip, vals.w, map->pixels(), scale); break;
						case Ops::terrain: asm_ssh_noise_terrain(bar, clip, map->pixels(), this); break;
					}
					break;
				case Types::correct: asm_ssh_correct(clip, wh, type_histogramm, map->pixels()); break;
				case Types::resize:
				{
					Range<int> _wh(wh);
					if(type_coord == Coord::percent) { _wh.w *= (int)(clip.w / 100.0f); _wh.h *= (int)(clip.h / 100.0f); }
					if(type_ops == Ops::pow2) { _wh.w = ssh_pow2<int>(_wh.w, false); _wh.h = ssh_pow2<int>(_wh.h, false); }
					buf_cs ptr(_wh, 4);
					if(type == Types::resize) asm_ssh_copy(bar, clip, map->pixels(), ptr, _wh, _wh, this);
					map->pix = ptr;
					map->ixywh = _wh;
					break;
				}
				case Types::figure: asm_ssh_figure(bar, clip, map->pixels(), this); break;
				case Types::gradient: asm_ssh_gradient(bar, clip, map->pixels(), this); break;
				case Types::replace: asm_ssh_replace(vals, msks, map->pixels(), clip); break;
				case Types::histogramm:
					Range<int> tmp(SSH_CAST(type_histogramm) >= SSH_CAST(ImgMod::Histogramms::rgb_v) ? Range<int>(258, 1) : wh);
					buf_cs buf(tmp, 4);
					asm_ssh_histogramm(tmp, this, buf);
					map->ixywh = tmp;
					map->pix = buf;
					break;
			}
		}
		catch(const Exception& e) { e.add(L""); }
	}
}
