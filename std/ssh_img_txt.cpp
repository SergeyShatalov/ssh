
#include "stdafx.h"
#include "ssh_img.h"

namespace ssh
{
	ImgTxt::ImgTxt(ssh_wcs _name, int _height, const Range<int>& _wh, const Buffer<Bar<int>>& _pos, const Buffer<ssh_w>& _remap, const Buffer<ssh_cs>& _pix) : ImgMap(_wh, _pix), height(_height), remap(_remap), name(_name)
	{
		// скорректировать координаты
		/*
		fBar.x = (float)bar.x / (float)r.w;
		fBar.y = (float)bar.y / (float)r.h;
		fBar.w = (float)bar.w / (float)r.w;
		fBar.h = (float)bar.h / (float)r.h;
		*/
	}
	
	void ImgTxt::draw(ssh_cs * dst, const Pts<int>& pt, const Bar<int>& clip, const Bar<int>& screen) const
	{
	}
	
	int ImgTxt::install(Image * img, ssh_wcs text, const Range<int>& rn_text, Aligned align, ImgMod::Coord coords)
	{
		return 0;
	}
	
	Buffer<QUAD> ImgTxt::make_quads(const Pts<int>& pt, const Bar<int>& clip, const Bar<int>& screen)
	{
		return Buffer<QUAD>();
	}

	/*
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
	*/
}