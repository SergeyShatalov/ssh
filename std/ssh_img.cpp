
#include "stdafx.h"
#include "ssh_img.h"

namespace ssh
{
	void Image::set_map(ssh_wcs path, int layer, int mip, int src_layer, int src_mip)
	{
		ImgCnv cnv(path, src_layer, src_mip);
		if(cnv.pix.size())
		{
			ImgMap* _new_map(new ImgMap(cnv.wh, cnv.pix));
			ImgMap* map(maps[layer]);
			if(map) map->set_mip(mip, _new_map); else maps[layer] = _new_map;
		}
	}

	QUAD Image::quad(int layer, const Pts<int>& pt, const Bar<int>& clip, const Bar<int>& screen, const color & col) const
	{
		return QUAD();
	}

	Image::ImgMap* Image::duplicate(int layer, int mip)
	{
		return nullptr;
	}

	Buffer<ssh_cs> Image::histogramm(int layer, int mip, const Range<int>& wh, ImgMod::Histogramms type, const color & bkg, const color & frg)
	{
		return Buffer<ssh_cs>();
	}

	Buffer<ssh_cs> Image::make() const
	{
		return Buffer<ssh_cs>();
	}

	ssh_u Image::get_mips() const
	{
		return ssh_u();
	}
	
	void Image::save(ssh_wcs path, bool is_xml)
	{
	}

	void Image::make(const Buffer<ssh_cs>& buf)
	{
	}
}

/*

#include "stdafx.h"

#include <ostd\oimg.h>
#include <ostd\ostkImg.h>

namespace ostd
{
static STK_ENUM m_img_typesIO[] =
{
{"Tga", CAST(ImgIO::TypesIO::Tga)},
{"Dds", CAST(ImgIO::TypesIO::Dds)},
{"Bmp", CAST(ImgIO::TypesIO::Bmp)},
{"Gif", CAST(ImgIO::TypesIO::Gif)},
{"Jpg", CAST(ImgIO::TypesIO::Jpg)},
{"Fse", CAST(ImgIO::TypesIO::Fse)},
{"Bfs", CAST(ImgIO::TypesIO::Bfs)},
{nullptr, 0},
};

static STK_ENUM m_img_cmds[] =
{
{"None", CAST(Image::Commands::None)},
{"Make", CAST(Image::Commands::Make)},
{"Open", CAST(Image::Commands::Open)},
{"Save", CAST(Image::Commands::Save)},
{"Modify", CAST(Image::Commands::Modify)},
{"Remove", CAST(Image::Commands::Remove)},
{"Font", CAST(Image::Commands::Font)},
{"Empty", CAST(Image::Commands::Empty)},
{"Duplicate", CAST(Image::Commands::Duplicate)},
{"Draw", CAST(Image::Commands::Draw)},
{"Packed", CAST(Image::Commands::Packed)},
{nullptr, 0}
};

static STK_ENUM m_img_types[] =
{
{"Atlas", CAST(Image::Types::Atlas)},
{"Cube", CAST(Image::Types::Cube)},
{"Volume", CAST(Image::Types::Volume)},
{"Texture", CAST(Image::Types::Texture)},
{"Array", CAST(Image::Types::Array)},
{nullptr, 0}
};
// форматы
static STK_ENUM m_img_fmts[]=
{
{"BC1", CAST(ImgIO::ImageFormats::BC1)},
{"BC2", CAST(ImgIO::ImageFormats::BC2)},
{"BC3", CAST(ImgIO::ImageFormats::BC3)},
{"Alpha", CAST(ImgIO::ImageFormats::Alpha)},
{"Luminance", CAST(ImgIO::ImageFormats::Luminance)},
{"BGRA8", CAST(ImgIO::ImageFormats::BGRA8)},
{"RGBA8", CAST(ImgIO::ImageFormats::RGBA8)},
{"BGR8", CAST(ImgIO::ImageFormats::BGR8)},
{"RGB8", CAST(ImgIO::ImageFormats::RGB8)},
{"B5G6R5", CAST(ImgIO::ImageFormats::B5G6R5)},
{"BGR5A1", CAST(ImgIO::ImageFormats::BGR5A1)},
{"BGRA4", CAST(ImgIO::ImageFormats::BGRA4)},
{nullptr, 0}
};
// модификаторы
static STK_ENUM m_img_modifiers[] =
{
{"flip", CAST(ImgModify::Types::Flip)},
{"copy", CAST(ImgModify::Types::Copy)},
{"border", CAST(ImgModify::Types::Border)},
{"resize", CAST(ImgModify::Types::Resize)},
{"noise", CAST(ImgModify::Types::Noise)},
{"correct", CAST(ImgModify::Types::Correct)},
{"mosaik", CAST(ImgModify::Types::Mosaik)},
{"figure", CAST(ImgModify::Types::Figure)},
{"gradient", CAST(ImgModify::Types::Gradient)},
{"replace", CAST(ImgModify::Types::Replace)},
{"histogramm", CAST(ImgModify::Types::Histogramm)},
{nullptr, 0}
};
// фильтры
static STK_ENUM m_img_filters[] =
{
{"none", CAST(ImgModify::Filters::None)},
{"sobel", CAST(ImgModify::Filters::Sobel)},
{"laplacian", CAST(ImgModify::Filters::Laplacian)},
{"prewit", CAST(ImgModify::Filters::Prewit)},
{"emboss", CAST(ImgModify::Filters::Emboss)},
{"normal", CAST(ImgModify::Filters::Normal)},
{"high", CAST(ImgModify::Filters::Hi)},
{"lower", CAST(ImgModify::Filters::Low)},
{"madian", CAST(ImgModify::Filters::Median)},
{"roberts", CAST(ImgModify::Filters::Roberts)},
{"maximum", CAST(ImgModify::Filters::Max)},
{"minimum", CAST(ImgModify::Filters::Min)},
{"contrast", CAST(ImgModify::Filters::Contrast)},
{"binary", CAST(ImgModify::Filters::Binary)},
{"gamma", CAST(ImgModify::Filters::Gamma)},
{"scaleBias", CAST(ImgModify::Filters::ScaleBias)},
{nullptr, 0}
};
// фигуры
static STK_ENUM m_img_figures[] =
{
{"ellipse", CAST(ImgModify::Figures::Ellipse)},
{"rectangle", CAST(ImgModify::Figures::Rectangle)},
{"triUp", CAST(ImgModify::Figures::TriangleUp)},
{"triDown", CAST(ImgModify::Figures::TriangleDown)},
{"triRight", CAST(ImgModify::Figures::TriangleRight)},
{"triLeft", CAST(ImgModify::Figures::TriangleLeft)},
{"pyramid", CAST(ImgModify::Figures::Pyramid)},
{"sixangle", CAST(ImgModify::Figures::Sixangle)},
{"eightangle", CAST(ImgModify::Figures::Eightangle)},
{"romb", CAST(ImgModify::Figures::Romb)},
{"star1", CAST(ImgModify::Figures::Star1)},
{"star2", CAST(ImgModify::Figures::Star2)},
{"arrowRight", CAST(ImgModify::Figures::ArrowRight)},
{"arrowLeft", CAST(ImgModify::Figures::ArrowLeft)},
{"arrowDown", CAST(ImgModify::Figures::ArrowDown)},
{"arrowUp", CAST(ImgModify::Figures::ArrowUp)},
{"cross45",	CAST(ImgModify::Figures::Cross45)},
{"checked",	CAST(ImgModify::Figures::Checked)},
{"vplzSlider", CAST(ImgModify::Figures::VPlzSlider)},
{"hplzSlider", CAST(ImgModify::Figures::HPlzSlider)},
{"plus", CAST(ImgModify::Figures::Plus)},
{nullptr, 0}
};
// типы координат
static STK_ENUM m_img_coords[] =
{
{"@", CAST(ImgModify::Coordinates::Absolute)},
{"%", CAST(ImgModify::Coordinates::Percent)},
{nullptr, 0}
};
// типы адресаций
static STK_ENUM m_img_addresses[] =
{
{"l_clamp", CAST(ImgModify::Addresses::LinearClamp)},
{"l_mirror", CAST(ImgModify::Addresses::LinearMirror)},
{"l_repeat", CAST(ImgModify::Addresses::LinearRepeat)},
{"n_clamp", CAST(ImgModify::Addresses::NearClamp)},
{"n_mirror", CAST(ImgModify::Addresses::NearMirror)},
{"n_repeat", CAST(ImgModify::Addresses::NearRepeat)},
{nullptr, 0}
};
// границы
static STK_ENUM m_img_borders[] =
{
{"left", CAST(ImgModify::Borders::Left)},
{"right", CAST(ImgModify::Borders::Right)},
{"top", CAST(ImgModify::Borders::Top)},
{"bottom", CAST(ImgModify::Borders::Bottom)},
{"all", CAST(ImgModify::Borders::All)},
{nullptr, 0}
};
// гистограммы
static STK_ENUM m_img_histogramms[] =
{
{"rgb", CAST(ImgModify::Histogramms::Rgb)},
{"red", CAST(ImgModify::Histogramms::Red)},
{"green", CAST(ImgModify::Histogramms::Green)},
{"blue", CAST(ImgModify::Histogramms::Blue)},
{"rgbV", CAST(ImgModify::Histogramms::ValRgb)},
{"redV", CAST(ImgModify::Histogramms::ValRed)},
{"greenV", CAST(ImgModify::Histogramms::ValGreen)},
{"blueV", CAST(ImgModify::Histogramms::ValBlue)},
{nullptr, 0}
};
// пиксельные операции
static STK_ENUM m_img_pix_ops[] =
{
{"add", CAST(ImgModify::PixelOperations::Add)},
{"sub", CAST(ImgModify::PixelOperations::Sub)},
{"set", CAST(ImgModify::PixelOperations::Set)},
{"xor", CAST(ImgModify::PixelOperations::Xor)},
{"and", CAST(ImgModify::PixelOperations::And)},
{"or", CAST(ImgModify::PixelOperations::Or)},
{"lum", CAST(ImgModify::PixelOperations::Lum)},
{"not", CAST(ImgModify::PixelOperations::Not)},
{"alpha", CAST(ImgModify::PixelOperations::Alpha)},
{"fixed", CAST(ImgModify::PixelOperations::Fixed)},
{"mul", CAST(ImgModify::PixelOperations::Mull)},
{"lumAdd", CAST(ImgModify::PixelOperations::LumAdd)},
{"lumSub", CAST(ImgModify::PixelOperations::LumSub)},
{"norm", CAST(ImgModify::PixelOperations::Norm)},
{"pow2", CAST(ImgModify::PixelOperations::Pow2)},
{nullptr, 0}
};
// типы операций
static STK_ENUM m_img_type_ops[] =
{
{"perlin", CAST(ImgModify::TypeOperations::Perlin)},
{"borderO3d", CAST(ImgModify::TypeOperations::BrdOut3d)},
{"borderI3d", CAST(ImgModify::TypeOperations::BrdIn3d)},
{"group", CAST(ImgModify::TypeOperations::GroupBox)},
{"flipH", CAST(ImgModify::TypeOperations::FlipH)},
{"flipV", CAST(ImgModify::TypeOperations::FlipV)},
{"flip90", CAST(ImgModify::TypeOperations::Flip90)},
{"table2d", CAST(ImgModify::TypeOperations::Table)},
{"table3d", CAST(ImgModify::TypeOperations::Table3d)},
{"tableGrp", CAST(ImgModify::TypeOperations::TableGrp)},
{"terrain", CAST(ImgModify::TypeOperations::Terrain)},
{nullptr, 0}
};
// выравнивание текста
static STK_ENUM m_img_aligned[] =
{
{"top", CAST(ImgText::Aligned::Top)},
{"left", CAST(ImgText::Aligned::Left)},
{"right", CAST(ImgText::Aligned::Right)},
{"bottom", CAST(ImgText::Aligned::Bottom)},
{"hcenter", CAST(ImgText::Aligned::HCenter)},
{"vcenter", CAST(ImgText::Aligned::VCenter)},
{"break", CAST(ImgText::Aligned::WordBreak)},
{nullptr, 0}
};
// типы ландшафта
static STK_ENUM m_img_terrain[] =
{
{"hill", CAST(ImgModify::TerrainTypes::Hill)},
{"island", CAST(ImgModify::TerrainTypes::Island)},
{"mountain", CAST(ImgModify::TerrainTypes::Mountain)},
{"pass", CAST(ImgModify::TerrainTypes::Pass)},
{"pit", CAST(ImgModify::TerrainTypes::Pit)},
{"plain", CAST(ImgModify::TerrainTypes::Plain)},
{"raid", CAST(ImgModify::TerrainTypes::Raid)},
{"valley", CAST(ImgModify::TerrainTypes::Valley)},
{nullptr, 0}
};

static STK_ENUM m_img_charset[] =
{
{"ANSI", ANSI_CHARSET},
{"DEFAULT", DEFAULT_CHARSET},
{"SYMBOL", SYMBOL_CHARSET},
{"RUSSIAN", RUSSIAN_CHARSET},
{"OEM", OEM_CHARSET},
{nullptr, 0}
};

static STK_ENUM m_img_family[] =
{
{"DONTCARE", FF_DONTCARE},	// неопределено
{"ROMAN", FF_ROMAN},		// переменная ширина(Serifed)
{"SWISS", FF_SWISS},		// переменная ширина(Sans Serifed)
{"MODERN", FF_MODERN},		// одинаковая ширина
{"SCRIPT", FF_SCRIPT},		// курсив
{"DECORATIVE", FF_DECORATIVE},// декоративный
{nullptr, 0}
};

void Image::init()
{
head.type = Types::Texture;
head.fmt = ImgIO::ImageFormats::BGRA8;
head.mips = 1;
head.count = 0;
head.offsXY = 0;
head.sign = SIGN_IMAGE;
}
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
static BYTE syms[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0x80, 4 + 8 + 16 + 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
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

ImgModify::ImgModify(Image* img, const String& img_name, Xml* xml, HXML hroot)
{
String def;
type = hlp->cnvValue<Types>(xml->attr(hroot, "type", def), m_img_modifiers, Types::Undefine);
address = hlp->cnvValue<Addresses>(xml->attr(hroot, "address", def), m_img_addresses, Addresses::LinearClamp);
typeOps = hlp->cnvValue<TypeOperations>(xml->attr(hroot, "typeOps", def), m_img_type_ops, TypeOperations::None);
pixOps = hlp->cnvValue<PixelOperations>(xml->attr(hroot, "ops", def), m_img_pix_ops, PixelOperations::Set);
hlp->explode<uint_t>(",", xml->attr(hroot, "bar", def), bar, 4, 0);
hlp->explode<uint_t>(",", xml->attr(hroot, "mask", def), (uint_t*)&msks, 2, 0x00FFFFFF, nullptr, true),
hlp->explode<uint_t>(",", xml->attr(hroot, "value", def), (uint_t*)&vals, 2, 0, nullptr, true);
hlp->explode<uint_t>(",", xml->attr(hroot, "color", def), (uint_t*)&colors, 2, 0, nullptr, true);
hlp->explode<uint_t>(",", xml->attr(hroot, "cells", def), whCells, 2, 1);
hlp->explode<uint_t>(",", xml->attr(hroot, "range", def), range, 2, 0);
hlp->explode<uint_t>(",", xml->attr(hroot, "wh", def), wh, 2, 1);
hlp->explode<float>(",", xml->attr(hroot, "vec", def), fltVec, 4, 1.0f);
hlp->explode<uint_t>(",", xml->attr(hroot, "count", def), array_count, 2, 1);
nRepeat = xml->attr(hroot, "repeat", 1);
imgRel = xml->attr(hroot, "rel", 1);
widthBorder = xml->attr(hroot, "width", 1);
whMtx = xml->attr(hroot, "mtx", 3);
coord = hlp->cnvValue<Coordinates>(xml->attr(hroot, "coord", def), m_img_coords, Coordinates::Absolute);
filter = hlp->cnvValue<Filters>(xml->attr(hroot, "filter", def), m_img_filters, Filters::None);
figure = hlp->cnvValue<Figures>(xml->attr(hroot, "figure", def), m_img_figures, Figures::Ellipse);
sideBorder = hlp->cnvValue<Borders>(xml->attr(hroot, "side", def), m_img_borders, Borders::All);
pixOpsEx = hlp->cnvValue<PixelOperations>(xml->attr(hroot, "opsEx", def), m_img_pix_ops, PixelOperations::Set);
terrain = hlp->cnvValue<TerrainTypes>(xml->attr(hroot, "terrain", def), m_img_terrain, TerrainTypes::Island);
histogramm = hlp->cnvValue<Histogramms>(xml->attr(hroot, "histogramm", def), m_img_histogramms, Histogramms::Rgb);
radius = xml->attr<uint_t>(hroot, "radius", 1);
shadow = Hex<uint_t>(xml->attr<String>(hroot, "shadow", "0"));
scale = xml->attr<float>(hroot, "scale", 1.0f);
alpha = xml->attr(hroot, "alpha", 1.0f);
if(xml->is_attr(hroot, "array_texture"))
{
ImgTexture* t(img->get(xml->attr(hroot, "array_texture", img_name)));
arrayBGRA.move(t->getPixels(), Ptr<BYTE>::opsMovePtr);
array_count = t->iBar.range;
}
else
{
arrayBGRA = Ptr<BYTE>(array_count.w * array_count.h);
hlp->explode<BYTE>(",", xml->attr(hroot, "array_values", def), arrayBGRA, arrayBGRA.count(), 0);
}
if(xml->is_attr(hroot, "array_names"))
{
arrayNames = Ptr<String>(array_count.w * array_count.h);
hlp->split(",", xml->attr(hroot, "array_names", def), arrayNames, arrayNames.count(), "null");
}
}

void ImgModify::apply(Image* img, const String& name)
{
try
{
ImgTexture* tex(img->get(name));
if(!tex) throw ExceptionImage(ExceptionImage::NOTFOUND_TEXTURE, name, OSTD_FLD);
// проверить на корректность параметров
// 1. области
// 2.
BYTE* src(tex->getPixels());
Range<uint_t> clip(tex->getBar().range);
if(bar.is_empty()) bar = clip;
switch(type)
{
case Types::Flip:
switch(typeOps)
{
case TypeOperations::FlipH:
OSTD_TRACE("%s(%s,%s)", __FUNCTION__, name, "flipH");
asmFlipH(absoluteBar(bar, clip, coord), clip, src);
break;
case TypeOperations::FlipV:
OSTD_TRACE("%s(%s,%s)", __FUNCTION__, name, "flipV");
asmFlipV(absoluteBar(bar, clip, coord), clip, src);
break;
case TypeOperations::Flip90:
OSTD_TRACE("%s(%s,%s)", __FUNCTION__, name, "flip90");
Ptr<BYTE> dst(tex->pixels.size());
asmFlip90(clip, dst, src);
tex->pixels.move(dst, Ptr<BYTE>::opsMove);
tex->setBar(Bar<uint_t>(0, 0, clip.h, clip.w), clip);
break;
}
break;
case Types::Copy:
OSTD_TRACE("%s(%s,%s)", __FUNCTION__, name, "copy");
asmCopy(absoluteBar(bar, clip, coord), clip, src, arrayBGRA, Bar<uint_t>(array_count), array_count, pixOps, address, filter, msks.w, nRepeat, fltVec, alpha, whMtx);
break;
case Types::Border:
switch(typeOps)
{
case TypeOperations::BrdOut3d:
case TypeOperations::BrdIn3d:
OSTD_TRACE("%s(%s,%s)", __FUNCTION__, name, "border3d");
asmBorder3d(absoluteBar(bar, clip, coord), clip, src, widthBorder, vals.w, msks.w, sideBorder, typeOps);
break;
case TypeOperations::GroupBox:
OSTD_TRACE("%s(%s,%s)", __FUNCTION__, name, "groupBox");
asmGroupBox(absoluteBar(bar, clip, coord), clip, src, widthBorder, vals.w, msks.w, sideBorder, typeOps);
break;
case TypeOperations::Table:
case TypeOperations::Table3d:
case TypeOperations::TableGrp:
OSTD_TRACE("%s(%s,%s)", __FUNCTION__, name, "table");
asmTable(absoluteBar(bar, clip, coord), clip, src, arrayBGRA, array_count.w, widthBorder, vals.w, msks.w, pixOps, pixOpsEx, typeOps);
break;
default:
OSTD_TRACE("%s(%s,%s)", __FUNCTION__, name, "border2d");
asmBorder(absoluteBar(bar, clip, coord), clip, src, widthBorder, vals.w, msks.w, sideBorder, pixOps);
break;
}
break;
case Types::Noise:
switch(typeOps)
{
case TypeOperations::Perlin:
// scale(масштаб в диапазоне 0.005 до 1.0), vals.w(минимальное значение)
OSTD_TRACE("%s(%s,%s)", __FUNCTION__, name, "noisePerlin");
asmNoisePerlin(clip, vals.w, src, scale);
break;
case TypeOperations::Terrain:
// range(диапазон по высоте), wh(габариты холмов), nRepeat(количество итераций), terrain(тип ланшафта), coordSet(система координат)
OSTD_TRACE("%s(%s,%s)", __FUNCTION__, name, "noiseTerrain");
asmNoiseTerrain(absoluteBar(bar, clip, coord), clip, src, vals, wh, pixOps, nRepeat);
break;
}
break;
case Types::Correct:
// range(диапазон значений для обрезки от 0 до 255), histogramm(тип коррекции - отдельно по каналам или сразу по всем)
OSTD_TRACE("%s(%s,%s)", __FUNCTION__, name, "correct");
asmCorrect(clip, range, src, histogramm);
break;
case Types::Resize:
case Types::Mosaik:
{
Range<uint_t> _wh(wh);
if(coord == Coordinates::Percent) {_wh.w *= (float)clip.w / 100.0f; _wh.h *= (float)clip.h / 100.0f;}
if(pixOpsEx == PixelOperations::Pow2) {_wh.w = hlp->pow2<uint_t>(_wh.w, true); _wh.h = hlp->pow2<uint_t>(_wh.h, true);}
Ptr<BYTE> ptr(asmTargetSize(_wh.w, _wh.h, ImgIO::ImageFormats::BGRA8));
if(type == Types::Mosaik)
{
// arrayNames(массив имён изображений в мозаике), array_count(размерность массива), whCells(расстояние между смежными изображениями), imgRel(признак учёта пропорции изображений)
OSTD_TRACE("%s(%s,%s)", __FUNCTION__, name, "mosaik");
asmMosaik(_wh, img->getCells(), ptr, arrayNames, array_count, whCells, imgRel);
}
else
{
OSTD_TRACE("%s(%s,%s)", __FUNCTION__, name, "resize");
asmCopy(Bar<uint_t>(_wh), _wh, ptr, src, tex->iBar, tex->iBar.range, pixOps, address, Filters::None, 0, nRepeat, fltVec);
}
tex->pixels.move(ptr, Ptr<BYTE>::opsMove);
tex->setBar(Bar<uint_t>(_wh), _wh);
}
break;
case Types::Figure:
// vals.h(значение за пределами фигуры), vals.w(значение внутри фигуры), msks.h(маска запределами фигуры), msks.w(маска внутри фигуры), arrayARGB(массив линий(в процентах) для произвольной фигуры),
// pixOpsEx(операция за пределами фигуры), pixOps(операция внутри фигуры), coords.w(система координат), figure(тип фигуры FigureTypes), radius(радиус углов), shadow(цвет тени - 0 = отсутствие тени)
OSTD_TRACE("%s(%s,%s)", __FUNCTION__, name, "figure");
asmFigure(absoluteBar(bar, clip, coord), clip, src, arrayBGRA, array_count, vals, msks, pixOps, pixOpsEx, figure, radius, shadow);
break;
case Types::Gradient:
OSTD_TRACE("%s(%s,%s)", __FUNCTION__, name, "gradient");
asmGradient(absoluteBar(bar, clip, coord), clip, src, vals, msks.w, pixOps, address, nRepeat);
break;
case Types::Replace:
OSTD_TRACE("%s(%s,%s)", __FUNCTION__, name, "replace");
asmReplace(vals, msks, src, clip);
break;
case Types::Histogramm:
{
OSTD_TRACE("%s(%s,%s)", __FUNCTION__, name, "histogramm");
Range<uint_t> tmp(CAST(histogramm) >= CAST(ImgModify::Histogramms::ValRgb) ? Range<uint_t>(256, 1) : wh);
Ptr<BYTE> buf(tmp.w * tmp.h * 4);
asmHistogramm(tmp, array_count, buf, arrayBGRA, colors.w, colors.h, histogramm);
tex->setBar(tmp, tmp);
tex->pixels.move(buf, Ptr<BYTE>::opsMove);
}
break;
default: throw ExceptionImage(ExceptionImage::UNDEFINED_MODIFY, "", OSTD_FLD);
}
}
catch(...) {throw ExceptionCommon(ExceptionCommon::UNEXPECTED, OSTD_FLD);}
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

ImgIO::DxFormats Image::dxFormat() const
{
static ImgIO::DxFormats m_dx[] = {  ImgIO::DxFormats::Undefine, ImgIO::DxFormats::BGRA8, ImgIO::DxFormats::RGBA8, ImgIO::DxFormats::RGB8, ImgIO::DxFormats::BGR8, ImgIO::DxFormats::B5G6R5,
ImgIO::DxFormats::BGR5A1, ImgIO::DxFormats::BGRA4, ImgIO::DxFormats::Alpha, ImgIO::DxFormats::Luminance, ImgIO::DxFormats::Font,
ImgIO::DxFormats::BC1, ImgIO::DxFormats::BC2, ImgIO::DxFormats::BC3};
OSTD_TRACE(__FUNCTION__);
return m_dx[CAST(head.fmt)];
}

Ptr<BYTE> Image::histogramm(const String& name, const Range<uint_t>& wh, ImgModify::Histogramms type, const color& background, const color& foreground)
{
OSTD_TRACE("%s(%s,%s,%Ii,%Ii)", __FUNCTION__, name, wh.w, wh.h);
ImgTexture* tex(get(name));
if(!tex) throw ExceptionImage(ExceptionImage::NOTFOUND_TEXTURE, name, OSTD_FLD);
Ptr<BYTE> buf(CAST(type) >= CAST(ImgModify::Histogramms::ValRgb) ? 1028 : wh.w * wh.h * 4);
asmHistogramm(wh, tex->getBar().range, buf, tex->getPixels(), background.BGRA(), foreground.BGRA(), type);
return buf;
}

uint_t Image::computeCountMips(Range<uint_t>& rangeTex)
{
OSTD_TRACE("%s(%Ii,%Ii)", __FUNCTION__, rangeTex.w, rangeTex.h);
uint_t flags;
int_t nMips(0), nMaxMips(!head.mips ? 1000 : head.mips);
rangeTex = range(rangeTex);
while(nMips < nMaxMips && asmTargetSize(rangeTex.w >> nMips, rangeTex.h >> nMips, ImgIO::ImageFormats::BGRA8, &flags) && flags == 0) nMips++;
return nMips;
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

uint_t Image::count(bool is_texture)
{
OSTD_TRACE("%s(%s)", __FUNCTION__, is_texture ? "true" : "false");
auto n(map.getNull());
uint_t c(0);
ImgTexture* tex;
while(n = map.value(n))
{
tex = n->value;
if(tex->numMip == 0) c++;
}
switch(type())
{
case Types::Atlas:
if(is_texture) break;
case Types::Texture:
if(c > 1) c = 1;
break;
case Types::Cube:
if(c > 6) c = 6;
break;
}
return c;
}

void Image::save(const String& name, const String& path, ImgIO::TypesIO type, ImgIO::ImageFormats fmt)
{
OSTD_TRACE("%s(%s,%s)", __FUNCTION__, name, path);
ImgTexture* tex(get(name));
if(!tex) throw ExceptionImage(ExceptionImage::NOTFOUND_TEXTURE, name, OSTD_FLD);
ImgIO::save(path, tex->getPixels(), tex->iBar.w, tex->iBar.h, type, fmt);
}

void Image::saveDds(const String& path, const Ptr<BYTE>& pixels, const Range<uint_t> range, ImgIO::ImageFormats fmt, uint_t mips)
{
OSTD_TRACE("%s(%s,%Ii,%Ii)", __FUNCTION__, path, range.w, range.h);
bool is_cnv(format() == ImgIO::ImageFormats::BGRA8);
ImgIO::saveDDS(hlp->extractPathFileFromFullPath(path) + ".dds", range.w, range.h, is_cnv ? fmt : format(), pixels, (ImgIO::TypesDds)type(), mips, count(false), is_cnv);
}

Range<uint_t> Image::packedAtlas(const Range<uint_t>& max)
{
OSTD_TRACE("%s(%Ii,%Ii)", __FUNCTION__, max.w, max.h);
struct TextureNode
{
TextureNode() : type(1) {}
TextureNode(const Bar<uint_t>& bar, uint_t type) : bar(bar), type(type) {}
Bar<uint_t> bar;
uint_t type;
};
static Bar<uint_t> bar1;
static Bar<uint_t> bar2;
ImgTexture* tex(nullptr);
Array<ImgTexture*, OSTD_TYPE> sortTexW;
Array<TextureNode*> nodes;
uint_t wmax(0), hmax(0), i, j, tp1, tp2;
// 1. отсортировать все текстуры по убыванию площади
nodes += new TextureNode(Bar<uint_t>(0, 0, max.w, max.h), 1);
// 1.1. скопировать во временный массив с сортировкой по ширине
auto n(map.getNull());
while(n = map.value(n)) {tex = n->value; if(!tex->numMip) sortTexW += tex;}
for(i = 0 ; i < sortTexW.size() ; i++)
{
uint_t w1(sortTexW[i]->iBar.w), _i(i);
for(j = i + 1 ; j < sortTexW.size() ; j++)
{
uint_t w2(sortTexW[j]->iBar.w);
if(w2 > w1) {w1 = w2 ; _i = j;}
}
if(_i != i) hlp->swap<ImgTexture*>(sortTexW[_i], sortTexW[i]);
}
// 1.2. сортируем по высоте
while(sortTexW.size() > 0)
{
tex = nullptr;
uint_t h1(sortTexW[0]->iBar.h), _i(0);
for(i = 1 ; i < sortTexW.size() ; i++)
{
uint_t h2(sortTexW[i]->iBar.h);
if(h2 > h1) {h1 = h2 ; _i = i;}
}
// 2. упаковать
Bar<uint_t>* barA(&sortTexW[_i]->iBar);
uint_t aw((barA->w + head.offsXY * 2)), ah((barA->h + head.offsXY * 2));
for(i = 0 ; i < nodes.size() ; i++)
{
TextureNode* tn(nodes[i]);
Bar<uint_t>* barN(&tn->bar);
if(aw <= barN->w && ah <= barN->h)
{
barA->x = barN->x + head.offsXY ; barA->y = barN->y + head.offsXY;
if(tn->type == 1)
{
tp1 = 1 ; tp2 = 2;
bar1.set(barN->x, barN->y + ah, barN->w, barN->h - ah);
bar2.set(barN->x + aw, barN->y, barN->w - aw, ah);
}
else
{
tp1 = 2 ; tp2 = 1;
bar1.set(barN->x + aw,barN->y, barN->w - aw, barN->h);
bar2.set(barN->x, barN->y + ah, aw, barN->h - ah);
}
if(bar2.w > 0 && bar2.h > 0)
{
barN->x = bar2.x ; barN->y = bar2.y;
barN->w = bar2.w ; barN->h = bar2.h;
tn->type = tp2;
if(bar1.w > 0 && bar1.h > 0) nodes.insert(i + 1, new TextureNode(bar1, tp1));
}
else
{
if(bar1.w > 0 && bar1.h > 0)
{
barN->x = bar1.x ; barN->y = bar1.y;
barN->w = bar1.w ; barN->h = bar1.h;
tn->type = tp1;
}
else
{
nodes.remove(i, 1);
}
}
tex = sortTexW[_i];
break;
}
}
if(tex)
{
wmax = hlp->Max<uint_t>(wmax, tex->iBar.right());
hmax = hlp->Max<uint_t>(hmax, tex->iBar.bottom());
}
sortTexW.remove(_i, 1);
}
Range<uint_t> r(hlp->pow2<size_t>(wmax, false), hlp->pow2<size_t>(hmax, false));
// сформировать у всех текстур плавающую область
auto nt(map.getNull());
while(nt = map.value(nt)) {tex = nt->value; tex->setBar(tex->getBar(), r);}
// вернуть результат
return r;
}

Range<uint_t> Image::range(const Range<uint_t>& max)
{
OSTD_TRACE("%s(%Ii,%Ii)", __FUNCTION__, max.w, max.h);
Range<uint_t> range(max);
auto n(map.getNull());

while(n = map.value(n))
{
ImgTexture* tex(n->value);
if(tex->numMip) continue;
switch(type())
{
case Types::Atlas:
return packedAtlas(max);
case Types::Cube:
case Types::Texture:
case Types::Array:
case Types::Volume:
range.w = hlp->Min<uint_t>(range.w, tex->iBar.w);
range.h = hlp->Min<uint_t>(range.h, tex->iBar.h);
if(type() == Types::Cube) range.w = hlp->Min<uint_t>(range.w, range.h);
break;
}
}
return range.set(hlp->pow2<uint_t>(range.w, true), hlp->pow2<uint_t>(range.h, true));
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

ImgTexture* Image::addImg(const String& name, const String& path, uint_t nMip, uint_t nLayer, Array<ImgTexture*>* texs)
{
OSTD_TRACE("%s(%s)", __FUNCTION__, name);
ImgIO::IMAGE* img(nullptr);
ImgTexture* tex(nullptr);
uint_t idx(1);
// формируем имя
String fmtName, nm(name);
if(nm.is_empty()) nm = hlp->extractFileTitleFromFullPath(path);
// проверить - такая текстура уже есть?
if(map[nm]) throw ExceptionImage(ExceptionImage::DUPLICATE_NAME, name, OSTD_FLD);
// грузим с диска
ImgIO imgFile(path);
// перечисляем все полученные изображения (может быть несколько, например, для GIF)
while((img = imgFile.enumerate(img == nullptr)))
{
// проверка на имя
while(map[nm]) {nm = fmtName.format("%s(%Ii)", nm, idx++);}
// создаем новый
map[nm] = (tex = new ImgTexture(Bar<uint_t>(0, 0, img->width, img->height), 0, 0, nMip, nLayer));
tex->pixels.move(img->pixels, Ptr<BYTE>::opsMove);
// проверить на дублирование слоя
checkImg(tex);
// добавить в массив, если надо
if(texs) *texs += tex;
nMip = 0; nLayer = -1;
}
return tex;
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
static BYTE syms_def[32] = {0, 0, 0, 0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0, 0, 0, 0, 0, 1, 0, 1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
// проверить на дублирование имени
if(font) throw ExceptionImage(ExceptionImage::DUPLICATE_NAME, name, OSTD_FLD);
// добавим пробел
if(!syms) syms = syms_def;
syms[4] |= 1; // пробел
// рассчитать габариты текстуры в зависимости от высоты шрифта
uint_t nTexHW(hf > 96 ? 4096 : hf > 62 ? 3072 : hf > 47 ? 2048 : hf > 35 ? 1536 : hf > 16 ? 768 : hf > 10 ? 512 : 384), i, x(0), y(0), wmax(0), hf2(0);
// заполняем структуру для отрисовки всех символов
BITMAPINFO bmi{{sizeof(BITMAPINFOHEADER), nTexHW, nTexHW, 1, (WORD)32, BI_RGB, 0, 0, 0, 0, 0}, {0, 0, 0, 0}};
// заполняем структуру для оценки ширины символа
BITMAPINFO bmiSym{{sizeof(BITMAPINFOHEADER), 256, 128, 1, (WORD)32, BI_RGB, 0, 0, 0, 0, 0}, {0, 0, 0, 0}};
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
::SelectObject(hdc, hFont[0]);::SelectObject(hdcSym, hFont[0]);
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
for(uint_t j = 0; j < 3 ; j++)
{
// выбираем шрифт
::SelectObject(hdc, hFont[j]);
::SelectObject(hdcSym, hFont[j]);
// какие символы создавать - единичный бит указывает на признак наличия
for(i = 0 ; i < 256 ; i++)
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
if((x + wImg) >= nTexHW) {x = 0 ; y += hf;}
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
for(i = 0 ; i < 768 ; i++) font->fontSyms[i] /= fbar;
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
catch(...) {throw ExceptionImage(ExceptionImage::ERROR_ADD_FONT, name, OSTD_FLD);}
// возвращаем
return font;
}

Ptr<BYTE> Image::make(Range<uint_t>& rangeTex, uint_t* nMips)
{
OSTD_TRACE("%s(%Ii,%Ii)", __FUNCTION__, rangeTex.w, rangeTex.h);
bool is_atlas(type() == Types::Atlas);
// 1. расчитать габариты текстуры и число мип уровней
uint_t countMips(computeCountMips(rangeTex)), count(count(true));
if(nMips) *nMips = countMips;
// 2. расчитать размер буферов
uint_t width(rangeTex.w), height(rangeTex.h), numMip, sz_mipFmt(0);
uint_t sz_BGRA8(asmTargetSize(width, height, ImgIO::ImageFormats::BGRA8)), sz_Fmt(asmTargetSize(width, height, format()));
for(numMip = 0; numMip < countMips; numMip++)
{
auto n(map.getNull());
sz_mipFmt += asmTargetSize(width >> numMip, height >> numMip, format());
}
uint_t sz_bufBGRA8(sz_BGRA8 * (is_atlas ? 1 : count)), sz_bufFmt(sz_mipFmt * (is_atlas ? 1 : count));
// 2.1. создать буферы под текстуры
Ptr<BYTE> textureBGRA8(sz_bufBGRA8), textureFmt(sz_bufFmt);
BYTE* texBGRA8(textureBGRA8), *texFmt(textureFmt);
OSTD_MEMZERO(texBGRA8, sz_bufBGRA8);
//OSTD_MEMZERO(textureFmt, sz_bufFmt);
// 2.2. копировать текстуры 0 мипа в буфер BGRA8 и формировать мип уровни
// одновременно, паковать их оттуда в целевой формат, с расчетом целевого адреса
uint_t szImgFmt(sz_Fmt);
Range<uint_t> clip(rangeTex);
BYTE* dstBGRA8, *dstFmt;
for(numMip = 0; numMip < countMips; numMip++)
{
auto n(map.getNull());
for(uint_t idx = 0 ; idx < count ; idx++)
{
ImgTexture* tex(get(numMip, idx));
if(!tex)
{
while(n = map.value(n))
{
if(n->value->numLayer == -1) {tex = n->value; break;}
}
}
uint_t texMip(tex ? tex->numMip : -1);
Bar<uint_t> bar(clip);
switch(type())
{
case Types::Atlas:
if(tex) bar = Bar<uint_t>(tex->iBar / (1 << numMip));
if(numMip > 0) texMip = -1;
case Types::Texture:
dstBGRA8 = texBGRA8;
break;
case Types::Cube:
case Types::Array:
dstBGRA8 = texBGRA8 + idx * sz_BGRA8;
dstFmt = texFmt + idx * sz_mipFmt;
break;
case Types::Volume:
dstBGRA8 = texBGRA8 + idx * sz_BGRA8;
dstFmt = texFmt + idx * szImgFmt;
break;
}
if(tex && numMip == texMip)
{
asmCopy(bar, clip, dstBGRA8, tex->pixels, Bar<uint_t>(tex->iBar.range), tex->iBar.range, ImgModify::PixelOperations::Set, ImgModify::Addresses::LinearClamp, ImgModify::Filters::None, 0, 1, vec4());
// добавить копирайт
BYTE* copyRight(dstBGRA8 + ((bar.y * (rangeTex.w * 4)) + (bar.x * 4)));
checkCopyright(copyRight, bar.range, Range<uint_t>(clip.w, bar.h));
}
// мип уровни и внутренний формат
switch(type())
{
case Types::Atlas:
case Types::Texture:
if(idx == 0 && numMip > 0 && numMip != texMip) asmMakeMipmapLevel(clip.w << 1, clip.h << 1, texBGRA8);
break;
default:
if(numMip > 0 && numMip != texMip) asmMakeMipmapLevel(clip.w << 1, clip.h << 1, dstBGRA8);
asmDecode(clip.w, clip.h, dstFmt, dstBGRA8, CAST(format()), 1);
}
}
// действие при переходе на следующий мип уровень
// адрес следующего мип уровня
switch(type())
{
case Types::Atlas:
case Types::Texture:
asmDecode(clip.w, clip.h, texFmt, texBGRA8, CAST(format()), 1);
case Types::Cube:
case Types::Array:
texFmt += szImgFmt;
break;
case Types::Volume:
texFmt += szImgFmt * count;
break;
}
clip /= 2;
szImgFmt /= 4;
}
return textureFmt;
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
for(uint_t j = 0 ; j < 768 ; j++)
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
for(uint_t i = 0 ; i < head.count ; i++)
{
// для каждой текстуры (name, bar, fbar, hsym, [syms])
String name((PCC)tmp); tmp += (name.length() + 1);
map[name] = new ImgTexture(	Bar<uint_t>((uint_t*)tmp), Bar<float>((float*)(tmp + 32)),
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
tex = addFont(	name, xml.attr(hops, "face", def), xml.attr(hops, "height", 10), hlp->cnvValue<DWORD>(xml.attr<String>(hops, "charset", "ANSI"), m_img_charset, ANSI_CHARSET),
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
ImgText text(	this, name, xml.attr(hops, "font", def), xml.attr(hops, "msg", def), Bar<uint_t>(hlp->explode<uint_t>(",", xml.attr(hops, "clip", def), tmp, 4, 0)),
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
while(hsym = xml.node(hnode,"sym", idx++))
{
int_t num(xml.attr(hsym, "num", -1));
if(num == -1 || num >= 768) continue;
hlp->explode<float>(",", xml.attr(hsym, "coords", def), tex->fontSyms[num], 4, 0.0f);
}
}
}
}
}

ImgTexture* Image::get(uint_t nMip, uint_t nLayer)
{
ImgTexture* tex;
auto n(map.getNull());
while(n = map.value(n))
{
tex = n->value;
if(tex->numMip == nMip && nLayer == tex->numLayer) return tex;
}
return nullptr;
}

void Image::checkImg(ImgTexture* img)
{
ImgTexture* tex;
uint_t l(img->numLayer);
if(l != -1)
{
uint_t m(img->numMip);
auto n(map.getNull());
while(n = map.value(n))
{
tex = n->value;
if(tex->numMip == m && l == tex->numLayer)
{
OSTD_TRACE("Дублирование текстуры(%s,%Ii,%Ii)", map.key(), l, m);
img->numLayer = -1;
break;
}
}
}
}

Bar<uint_t>* Image::clipBar(const Bar<uint_t>& bar, const Bar<uint_t>& clip, Bar<uint_t>* out)
{
return asmGetClipBar(bar, out, clip);
}

void Image::copy(const String& name, BYTE* buf, const Bar<uint_t>& bar, const Range<uint_t>& range, ImgModify::Addresses address, ImgModify::PixelOperations pixOps, ImgModify::Coordinates coord, ImgModify::Filters filter, uint_t mask, uint_t rep, float alpha, const vec4& fltVec, uint_t mtx)
{
ImgTexture* tex(get(name));
if(!tex) throw ExceptionImage(ExceptionImage::NOTFOUND_TEXTURE, name, OSTD_FLD);
Range<uint_t> clip(tex->getBar().range);
asmCopy(ImgModify::absoluteBar(bar, clip, coord), clip, tex->getPixels(), buf, Bar<uint_t>(range), range, pixOps, address, filter, mask, rep, fltVec, alpha, mtx);
}
}
*/