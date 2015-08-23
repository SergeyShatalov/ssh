
#include "stdafx.h"
#include "ssh_img.h"

namespace ssh
{
	Image::ImgTxt::ImgTxt(const Range<int>& _wh, const Buffer<ssh_cs>& _pix, int _height, const Buffer<Bar<int>>& _pos) : ImgMap(_wh, _pix), height(_height)
	{
		// скорректировать координаты
		/*
		fBar.x = (float)bar.x / (float)r.w;
		fBar.y = (float)bar.y / (float)r.h;
		fBar.w = (float)bar.w / (float)r.w;
		fBar.h = (float)bar.h / (float)r.h;
		*/
	}
	
	void Image::ImgTxt::draw(ssh_cs * dst, const Pts<int>& pt, const Bar<int>& clip, const Bar<int>& screen) const
	{
	}
	
	int Image::ImgTxt::install(Image * img, ssh_wcs text, const Range<int>& rn_text, Aligned align, ImgMod::Coord coords)
	{
		return 0;
	}
	
	Buffer<QUAD> Image::ImgTxt::make_quads(const Pts<int>& pt, const Bar<int>& clip, const Bar<int>& screen)
	{
		return Buffer<QUAD>();
	}
}