
#include "stdafx.h"
#include "ssh_img.h"

namespace ssh
{

}

/*

#include "stdafx.h"
#include <ostd\oimg.h>
#include <ostd\ofile.h>
#include <ostd\ostkImg.h>
#include <jpeglib\jpeglib.h>

#include <ostd\odxt.h>
#ifdef _DEBUG
#pragma comment(lib, "jpeglibd.lib")
#else
#pragma comment(lib, "jpeglib.lib")
#endif

namespace ostd
{
#define OIMG_COUNT_FMT_DDS		12

static void correctWH(uint_t& w, uint_t& h, ImgIO::ImageFormats fmt)
{
if(fmt == ImgIO::ImageFormats::BC1 || fmt == ImgIO::ImageFormats::BC2 || fmt == ImgIO::ImageFormats::BC3)
{
OSTD_TRACE("%s(%Ii,%Ii)", __FUNCTION__, w, h);
// корректировать габариты изображения
w &= ~3;
h &= ~3;
}
}

void ImgIO::IMAGE::decompress()
{
if(fmt != ImageFormats::BGRA8)
{
OSTD_TRACE(__FUNCTION__);
correctWH(width, height, fmt);
uint_t sz(width * height * 4);
Ptr<BYTE> tmp(sz);
asmDecode(width, height, tmp, pixels, CAST(fmt), 0);
pixels.move(tmp, Ptr<BYTE>::opsMove);
fmt = ImgIO::ImageFormats::BGRA8;
}
}

ImgIO::ImageFormats ImgIO::formatFromBpp(uint_t bpp, uint_t greenMask)
{
static ImageFormats m_fmts[] = {ImageFormats::Undefine, ImageFormats::Luminance, ImageFormats::B5G6R5, ImageFormats::BGR8, ImageFormats::BGRA8};
ImageFormats fmt(m_fmts[bpp]);
if(bpp == 2 && greenMask != 2016) fmt = (greenMask != 240 ? ImageFormats::BGR5A1 : ImageFormats::BGRA4);
return fmt;
}

ImgIO::ImgIO(const String& path, bool is_decompress)
{
try
{
OSTD_TRACE("%s(%s)", __FUNCTION__, path);
IMAGE* img(nullptr);
// извлекаем расширение
String ext(hlp->extractFileExtFromFullPath(path, false));
ext.makeLower();
// открываем файл на чтение
File file(path, File::open_read);
// читаем содержимое во временный буфер
Ptr<BYTE> ptr(file.read<BYTE>());
// в зависимости от расширения запускаем соответствующий парсер
if(ext == "dds") makeDDS(path, ptr);
else if(ext == "tga") makeTGA(path, ptr);
else if(ext == "bmp") makeBMP(path, ptr);
else if(ext == "jpg" || ext == "jpeg") makeJPG(path, ptr);
else if(ext == "gif") makeGIF(path, ptr);
else if(ext == "fse") makeFSE(path, ptr);
else if(ext == "bfs") makeBFS(path, ptr);
else throw ExceptionImage(ExceptionImage::UNDEFINED_FORMAT, path, OSTD_FLD);
// перечислить все полученные изображения и перекодировать их в стандартный формат
if(is_decompress) while(img = enumerate(img == nullptr)) {img->decompress();}
}
catch(const ExceptionFile& ef) {throw ExceptionImage(ExceptionImage::FILE_OPERATION, ef.path, ef.file, ef.line, ef.data);}
}

void ImgIO::save(const String& path, IMAGE* img, TypesIO type, ImageFormats fmt)
{
if(!img) throw ExceptionCommon(ExceptionCommon::NULL_PTR, OSTD_FLD);
save(path, img->pixels, img->width, img->height, type, fmt);
}

void ImgIO::save(const String& path, const Ptr<BYTE>& pix, uint_t width, uint_t height, TypesIO type, ImageFormats fmt)
{
static PCC m_ext[] = {".tga", ".bmp", ".fse", ".bfs", ".jpg", ".gif", ".dds"};
try
{
if(!pix) throw ExceptionCommon(ExceptionCommon::NULL_PTR, OSTD_FLD);
// сформировать путь
String pth(hlp->extractPathFileFromFullPath(path) + m_ext[CAST(type)]);
// преобразовать в целевой формат (+ все мип уровни и слои)
uint_t sz(asmTargetSize(width, height, fmt));
Ptr<BYTE> tmp(sz);
asmDecode(width, height, tmp, pix, CAST(fmt), 1);
// корректировать габариты
correctWH(width, height, fmt);
switch(type)
{
case TypesIO::Bfs:
saveBFS(pth, width, height, fmt, tmp);
break;
case TypesIO::Bmp:
saveBMP(pth, width, height, fmt, tmp);
break;
case TypesIO::Tga:
saveTGA(pth, width, height, fmt, tmp);
break;
case TypesIO::Fse:
saveFSE(pth, width, height, fmt, tmp);
break;
case TypesIO::Dds:
saveDDS(pth, width, height, fmt, tmp, TypesDds::Texture, 1, 1, false);
break;
default: throw ExceptionImage(ExceptionImage::UNDEFINED_FORMAT, pth, OSTD_FLD);
}
}
catch(const ExceptionFile& ef) {throw ExceptionImage(ExceptionImage::FILE_OPERATION, ef.path, ef.file, ef.line, ef.data);}
}

void ImgIO::makeBMP(const String& path, BYTE* buf)
{
OSTD_TRACE("%s(%s)", __FUNCTION__, path);
IMAGE* img;
// заголовок
HEADER_BMP* head((HEADER_BMP*)buf);
// определить байт на пиксель
uint_t bpp(head->dwBitPerPixel / 8);
if(head->dwPacking == 1 || bpp < 1 || bpp > 4) throw ExceptionImage(ExceptionImage::INVALID_BMP, path, OSTD_FLD);
uint_t flags(CAST(bpp == 1 ? FlagsIO::Indexed : FlagsIO::RGB));
// параметры
uint_t height(head->dwHeight), width(head->dwWidth), sz(height * width * 4);
// создать изображение
imgs.add(img = new IMAGE(width, height, ImageFormats::BGRA8));
BYTE* ptr(img->pixels.move(nullptr, sz, Ptr<BYTE>::opsMove));
// преобразовать
asmCnv(width, height, ptr, (buf + head->offsSurface), (buf + head->szCaption + 14), (FlagsIO)flags, bpp, 1, CAST(formatFromBpp(bpp, head->dwGreenMask)));
// развернуть
asmFlipV(Bar<uint_t>(0, 0, width, height), Range<uint_t>(width, height), ptr);
}

void ImgIO::saveBMP(const String& path, uint_t width, uint_t height, ImageFormats fmt, const Ptr<BYTE>& pix)
{
OSTD_TRACE("%s(%s,%Ii,%Ii)", __FUNCTION__, path, width, height);
// создаем файл
uint_t sz_head(54);
DWORD redMask(0), greenMask(0), blueMask(0), dwPacking(0);
BYTE* ppix(pix);
if(fmt == ImageFormats::B5G6R5)
{
sz_head = 70;
dwPacking = 3;
redMask = 0xf800;
greenMask = 0x07e0;
blueMask = 0x1f;
}
else if(fmt == ImageFormats::BGRA4)
{
sz_head = 70;
dwPacking = 3;
redMask = 0x0f00;
greenMask = 0xf0;
blueMask = 0x0f;
}
// заголовок
HEADER_BMP head{'MB', pix.size() + sz_head, 0, sz_head, sz_head - 14, width, height, 1, asmTargetSize(1, 1, fmt) * 8, dwPacking, pix.size(), 0, 0, 0, 0, redMask, greenMask, blueMask, 0};
File file(path, File::create_write);
// записываем
file.write(&head, sz_head);
uint_t pitch(width * (head.dwBitPerPixel / 8));
for(int_t i = height - 1; i >= 0 ; i--) {file.write(pix + i * pitch, pitch);}
}

void ImgIO::makeTGA(const String& path, BYTE* buf)
{
OSTD_TRACE("%s(%s)", __FUNCTION__, path);
IMAGE* img;
// заголовок
HEADER_TGA* head((HEADER_TGA*)buf);
// флаги
uint_t flags(CAST(FlagsIO::Null)), bpp(head->bBitesPerPixel / 8);
// определить тип
switch(head->bType)
{
case HEADER_TGA::RLE_INDEXED:
flags = CAST(FlagsIO::RLE);
case HEADER_TGA::INDEXED:
if(head->bColorMapType != 1 || head->bCmEntrySize != 24 || head->wCmLength > 256) throw ExceptionImage(ExceptionImage::INVALID_TGA, path, OSTD_FLD);
flags |= CAST(FlagsIO::Indexed);
break;
case HEADER_TGA::RLE_RGB:
flags = CAST(FlagsIO::RLE);
case HEADER_TGA::RGB:
flags |= CAST(FlagsIO::RGB);
break;
case HEADER_TGA::RLE_GREY:
flags = CAST(FlagsIO::RLE);
case HEADER_TGA::GREY:
flags |= CAST(FlagsIO::Grey);
break;
default: throw ExceptionImage(ExceptionImage::INVALID_TGA, path, OSTD_FLD);
}
// параметры
uint_t height(head->wHeight), width(head->wWidth), sz(width * height * 4);
// создаем изображение
imgs.add(img = new IMAGE(width, height, ImageFormats::BGRA8));
BYTE* ptr(img->pixels.move(nullptr, sz, Ptr<BYTE>::opsMove));
// внутренние указатели
BYTE* surface(buf + sizeof(HEADER_TGA)), *palette(surface);
if(flags & CAST(FlagsIO::Indexed)) surface = palette + 3 * head->wCmLength;
// преобразовать
asmCnv(width, height, ptr, surface, palette, (FlagsIO)flags, bpp, 0, CAST(formatFromBpp(bpp, 15)));
// развернуть
Range<uint_t> wh(width, height);
if(head->bFlags & HEADER_TGA::RIGHT) asmFlipH(Range<uint_t>(wh), wh, ptr);
if(!(head->bFlags & HEADER_TGA::UPPER)) asmFlipV(Bar<uint_t>(wh), wh, ptr);
}

void ImgIO::saveTGA(const String& path, uint_t width, uint_t height, ImageFormats fmt, const Ptr<BYTE>& pix)
{
OSTD_TRACE("%s(%s,%Ii,%Ii)", __FUNCTION__, path, width, height);
if(fmt != ImageFormats::Luminance && fmt != ImageFormats::BGR5A1 && fmt != ImageFormats::BGR8 && fmt != ImageFormats::BGRA8)
throw ExceptionImage(ExceptionImage::WRONG_FORMAT_TGA, path, OSTD_FLD);
// создаем
File file(path, File::create_write);
// заголовок
HEADER_TGA head{0, 0, (fmt == ImageFormats::Luminance ? HEADER_TGA::GREY : HEADER_TGA::RGB), 0, 0, 0, 0, 0, width, height, asmTargetSize(1, 1, fmt) * 8, HEADER_TGA::UPPER | HEADER_TGA::ALPHA};
// записываем заголовок
file.write(&head, sizeof(HEADER_TGA));
//записываем изображение
file.write(pix, pix.size());
}

void ImgIO::makeFSE(const String& path, BYTE* buf)
{
OSTD_TRACE("%s(%s)", __FUNCTION__, path);
IMAGE* img;
// проверить на новый формат
if(*(DWORD*)buf == 'GRES')
{
// новый формат
uint_t width(*(WORD*)(buf + 4)), height(*(WORD*)(buf + 6)), sz(width * height * 4);
// добавляем в массив
imgs.add(img = new IMAGE(width, height, ImageFormats::BGRA8));
img->pixels.copy(buf + 8, sz);
}
else
{
// старый формат
uint_t width(*(WORD*)buf), height(*(WORD*)(buf + 2)), sz(width * height * 2);
// добавляем в массив
imgs.add(img = new IMAGE(width, height, ImageFormats::B5G6R5));
img->pixels.copy(buf + 4, sz);
}
}

void ImgIO::saveFSE(const String& path, uint_t width, uint_t height, ImageFormats fmt, const Ptr<BYTE>& pix)
{
OSTD_TRACE("%s(%s,%Ii,%Ii)", __FUNCTION__, path, width, height);
if(fmt != ImageFormats::BGRA8) throw ExceptionImage(ExceptionImage::WRONG_FORMAT_FSE, path, OSTD_FLD);
// создаем файл
File file(path, File::create_write);
// записать заголовок
file.write("SERG", false);
file.write(&width, 2);
file.write(&height, 2);
// записать пиксели
file.write(pix, pix.size());
}

void ImgIO::makeBFS(const String& path, BYTE* buf)
{
OSTD_TRACE("%s(%s)", __FUNCTION__, path);
IMAGE* img;
// параметры
BYTE* buf1(buf);
uint_t width(*(WORD*)buf), height(*(WORD*)(buf + 2)), sz(width * height * 64), i, j, ii;
buf += 4;
// создаём изображение
imgs.add(img = new IMAGE(width * 4, height * 4, ImageFormats::BGRA8));
BYTE* pdst(img->pixels.move(nullptr, sz, Ptr<BYTE>::opsMove));
// заполняем
for(i = 0 ; i < height ; i++)
{
BYTE* pdst1(pdst);
for(j = 0 ; j < width ; j++)
{
BYTE* pdst2(pdst1);
long is(*(long*)buf); buf += 4;
for(ii = 0 ; ii < 4 ; ii++)
{
if(is) {memcpy(pdst2, buf, 16); buf += 16;} else memset(pdst2, 0, 16);
pdst2 += width * 16;
}
pdst1 += 16;
}
pdst += width * 64;
}
}

void ImgIO::saveBFS(const String& path, uint_t width, uint_t height, ImageFormats fmt, const Ptr<BYTE>& src)
{
OSTD_TRACE("%s(%s,%Ii,%Ii)", __FUNCTION__, path, width, height);
if(fmt != ImageFormats::BGRA8) throw ExceptionImage(ExceptionImage::WRONG_FORMAT_BFS, path, OSTD_FLD);
uint_t offs(width * 4), size(4), i, j, ii, jj;
// корректируем габариты
correctWH(width, height, ImageFormats::BC1);
height /= 4; width /= 4;
// выделяем буфер под результат
Ptr<BYTE> dst(height * width * 68 + 4);
BYTE* pdst(dst), *psrc(src);
*(WORD*)(pdst) = (WORD)width ; *(WORD*)(pdst + 2) = (WORD)height;
pdst += 4;
for(i = 0 ; i < height ; i++)
{
BYTE* psrc1(psrc);
for(j = 0 ; j < width ; j++)
{
uint_t result(4);
BYTE* pdstBlk(pdst), *psrc2(psrc1);
*(DWORD*)pdstBlk = 0; pdst += 4;
for(ii = 0 ; ii < 4 ; ii++)
{
for(jj = 0 ; jj < 4 ; jj++)
{
DWORD val(asmDecodeBFS((psrc2 + jj * 4)));
//DWORD val(*(DWORD*)(psrc2 + jj * 4)), val1(val & 0x00f0f0f0);
if(val && result == 4) {*(DWORD*)pdstBlk = 64; result = 4 * 4 * 4 + 4;}
*(DWORD*)pdst = val;//(val1 ?  val : val1);
pdst += 4;
}
psrc2 += offs;
}
pdst = pdstBlk + result;
size += result;
psrc1 += 16;
}
psrc += offs * 4;
}
File file(path, File::create_write);
file.write(dst, size);
}

static DDS_FMT dds_fmt[OIMG_COUNT_FMT_DDS] =
{
{HEADER_DDS::Alpha, 8, 0, 0, 0, 0xff, ImgIO::ImageFormats::Alpha, ImgIO::DxFormats::Alpha},
{HEADER_DDS::Lum, 8, 0, 0xff, 0xff, 0xff, ImgIO::ImageFormats::Luminance, ImgIO::DxFormats::Luminance},
{HEADER_DDS::RGB, 16, 0x0000001f, 0x000007e0, 0x0000f800, 0, ImgIO::ImageFormats::B5G6R5, ImgIO::DxFormats::B5G6R5},
{HEADER_DDS::AlphaPixels | HEADER_DDS::RGB, 16, 0x0000001f, 0x000003e0, 0x00007c00, 0x00008000, ImgIO::ImageFormats::BGR5A1, ImgIO::DxFormats::BGR5A1},
{HEADER_DDS::AlphaPixels | HEADER_DDS::RGB, 16, 0x00000f00, 0x000000f0, 0x0000000f, 0x0000f000, ImgIO::ImageFormats::BGRA4, ImgIO::DxFormats::BGRA4},
{HEADER_DDS::RGB, 24, 0x000000ff, 0x0000ff00, 0x00ff0000, 0, ImgIO::ImageFormats::BGR8, ImgIO::DxFormats::BGR8},
{HEADER_DDS::RGB, 24, 0x00ff0000, 0x0000ff00, 0x000000ff, 0, ImgIO::ImageFormats::RGB8, ImgIO::DxFormats::RGB8},
{HEADER_DDS::AlphaPixels | HEADER_DDS::RGB, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000, ImgIO::ImageFormats::BGRA8, ImgIO::DxFormats::BGRA8},
{HEADER_DDS::AlphaPixels | HEADER_DDS::RGB, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000, ImgIO::ImageFormats::RGBA8, ImgIO::DxFormats::RGBA8},
{HEADER_DDS::FourCC, 0, 0, 0, 0, 0, ImgIO::ImageFormats::BC1, ImgIO::DxFormats::dx9DXT1},
{HEADER_DDS::FourCC, 0, 0, 0, 0, 0, ImgIO::ImageFormats::BC2, ImgIO::DxFormats::dx9DXT3},
{HEADER_DDS::FourCC, 0, 0, 0, 0, 0, ImgIO::ImageFormats::BC3, ImgIO::DxFormats::dx9DXT5}
};

void ImgIO::makeDDS(const String& path, BYTE* buf)
{
OSTD_TRACE("%s(%s)", __FUNCTION__, path);
IMAGE* img;
String nm(hlp->extractFileTitleFromFullPath(path));
uint_t i;
// заголовок
HEADER_DDS* head((HEADER_DDS*)buf);
// проверка на сигнатуру
if(head->dwMagic != 0x20534444) throw ExceptionImage(ExceptionImage::INVALID_DDS, path, OSTD_FLD);
int flags(head->ddDesc.dwFlags);
// проверка на флаги
if(!(flags && HEADER_DDS::Caps) || !(flags && HEADER_DDS::PixelFormat) || !(flags && HEADER_DDS::Width) || !(flags && HEADER_DDS::Height)) throw ExceptionImage(ExceptionImage::INVALID_DDS, path, OSTD_FLD);
// параметры
uint_t width(head->ddDesc.dwWidth), height(head->ddDesc.dwHeight);
ImageFormats fmt(ImageFormats::Undefine);
flags = head->ddDesc.ddpfPixelFormat.dwFlags & (HEADER_DDS::Alpha | HEADER_DDS::AlphaPixels | HEADER_DDS::RGB | HEADER_DDS::Lum | HEADER_DDS::FourCC | HEADER_DDS::MipMapCount);
// определяем формат
for(i = 0 ; i < OIMG_COUNT_FMT_DDS ; i++)
{
DDS_FMT* df(&dds_fmt[i]);
if(df->bpp != head->ddDesc.ddpfPixelFormat.dwRGBBitCount) continue;
if(df->flags != flags) continue;
if(flags == HEADER_DDS::FourCC)
{
if(head->ddDesc.ddpfPixelFormat.dwFourCC != CAST(df->dfmt)) continue;
fmt = df->ifmt; break;
}
if(df->bmask != head->ddDesc.ddpfPixelFormat.dwBBitMask) continue;
if(df->gmask != head->ddDesc.ddpfPixelFormat.dwGBitMask) continue;
if(df->rmask != head->ddDesc.ddpfPixelFormat.dwRBitMask) continue;
if(df->amask == head->ddDesc.ddpfPixelFormat.dwRGBAlphaBitMask) {fmt = df->ifmt; break;}
}
if(fmt == ImageFormats::Undefine)  throw ExceptionImage(ExceptionImage::INVALID_DDS, path, OSTD_FLD);
uint_t count(1), sz(0);
flags = head->ddDesc.ddsCaps.dwCaps2;
uint_t mips(flags & HEADER_DDS::Volume ? 1 : head->ddDesc.dwMipMapCount);
if(mips == 0) mips = 1;
// расчитать размер буфера под мип уровни
for(i = 0 ; i < mips ; i++) sz += asmTargetSize(width >> i, height >> i, fmt);
// определяем количество текстур
if(flags & HEADER_DDS::CubeMap) count = 6;
else if(flags & HEADER_DDS::Volume) count = head->ddDesc.dwDepth;
// указатель на поверхность
BYTE* surface(buf + sizeof(HEADER_DDS));
// проходим по всем текстурам
for(i = 0 ; i < count ; i++)
{
// создаем изображение
imgs.add(img = new IMAGE(width, height, fmt));
img->pixels.copy(surface, sz);
surface += sz;
}
}

void ImgIO::saveDDS(const String& path, uint_t width, uint_t height, ImageFormats f, const Ptr<BYTE>& pix, TypesDds type_dds, uint_t mips, uint_t layers, bool is_cnv)
{
OSTD_TRACE("%s(%s,%Ii,%Ii,%Ii,%Ii)", __FUNCTION__, path, width, height, mips, layers);
/*
uint_t sz(0), sz1, i, j;
// 1. расчитать размер буфера
if(is_cnv)
{
for(i = 0; i < mips; i++) sz += asmTargetSize(width >> i, height >> i, f);
sz *= layers;
} else sz = pix.size();
// 1. создать буфер и заполнить его
Ptr<BYTE> tmp(sz);
if(!is_cnv)
{
OSTD_MEMCPY(tmp, pix, sz);
}
else
{
OSTD_MEMZERO(tmp, sz);
BYTE* ptmp(tmp), *ppix(pix);
// 2. преобразовать в целевой формат
switch(type_dds)
{
case TypesDds::Volume:
for(i = 0; i < mips; i++)
{
for(j = 0 ; j < layers ; j++)
{
sz = asmTargetSize(width >> i, height >> i, f);
sz1 = asmTargetSize(width >> i, height >> i, ImageFormats::BGRA8);
asmDecode(width >> i, height >> i, ptmp, ppix, CAST(f), 1);
ptmp += sz; ppix += sz1;
}
}
break;
case TypesDds::Cube:
case TypesDds::Array:
for(j = 0 ; j < layers ; j++)
{
for(i = 0; i < mips; i++)
{
sz = asmTargetSize(width >> i, height >> i, f);
sz1 = asmTargetSize(width >> i, height >> i, ImageFormats::BGRA8);
asmDecode(width >> i, height >> i, ptmp, ppix, CAST(f), 1);
ptmp += sz; ppix += sz1;
}
}
break;
case TypesDds::Texture:
case TypesDds::Atlas:
for(i = 0; i < mips; i++)
{
sz = asmTargetSize(width >> i, height >> i, f);
sz1 = asmTargetSize(width >> i, height >> i, ImageFormats::BGRA8);
asmDecode(width >> i, height >> i, ptmp, ppix, CAST(f), 1);
ptmp += sz; ppix += sz1;
}
break;
}
}
// ищем нужный формат
for(uint_t i = 0; i < OIMG_COUNT_FMT_DDS; i++)
{
	DDS_FMT* fmt(&dds_fmt[i]);
	if(CAST(fmt->ifmt) == CAST(f))
	{
		// заголовок
		DWORD caps2(0), flags(HEADER_DDS::Caps | HEADER_DDS::Height | HEADER_DDS::Width | HEADER_DDS::PixelFormat | (mips > 1 ? HEADER_DDS::MipMapCount : 0));
		switch(type_dds)
		{
			case TypesDds::Texture:
			case TypesDds::Atlas:
				break;
			case TypesDds::Volume:
				caps2 = HEADER_DDS::Volume;
				flags |= HEADER_DDS::Depth;
				break;
			case TypesDds::Cube:
			case TypesDds::Array:
				caps2 = HEADER_DDS::Cube;
				break;
		}
		HEADER_DDS head{ 0x20534444,{ sizeof(HEADER_DDS::DDS_SURFACE), flags, height, width, 0, layers, mips, 0, 0, 0,{ 0, 0 },{ 0, 0 },{ 0, 0 },{ 0, 0 },
		{ sizeof(HEADER_DDS::DDS_FORMAT), fmt->flags, (fmt->flags & HEADER_DDS::FourCC ? CAST(fmt->dfmt) : 0), fmt->bpp, fmt->rmask, fmt->gmask, fmt->bmask, fmt->amask },
		{ HEADER_DDS::Texture | HEADER_DDS::Pitch | HEADER_DDS::Complex, caps2, 0, 0 }, 0 } };
		// создаем файл
		File file(path, File::create_write);
		// записываем заголовок
		file.write(&head, sizeof(HEADER_DDS));
		// записываем изображение
		file.write(pix, pix.size());
		return;
	}
}
throw ExceptionImage(ExceptionImage::WRONG_FORMAT_DDS, path, OSTD_FLD);
	}

	static GIF_FMT gif_fmt[] =
	{
		{ 0,0,0,0,0 },			// 0
		{ 3,4,3,2,3 },			// 1
		{ 7,8,3,4,5 },			// 2
		{ 15,16,4,8,9 },			// 3
		{ 31,32,5,16,17 },		// 4
		{ 63,64,6,32,33 },		// 5
		{ 127,128,7,64,65 },		// 6
		{ 255,256,8,128,129 },	// 7
		{ 511,512,9,256,257 }		// 8
	};

	void ImgIO::makeGIF(const String& path, BYTE* buf)
	{
		OSTD_TRACE("%s(%s)", __FUNCTION__, path);
		String nm(hlp->extractFileTitleFromFullPath(path));
		BYTE* globalPalette(nullptr);
		BYTE nChunk;
		uint_t iBackGround = -1, iTrans = -1, sz, count(0);
		// внутренние структуры
		GIF_BLOCK_IMAGE_EX* blockImgEx;
		GIF_BLOCK_APPLICATION* blockApp;
		// заголовок
		HEADER_GIF* head((HEADER_GIF*)buf);
		// проверка на сигнатуру
		if(head->dwMagic != '8FIG') throw ExceptionImage(ExceptionImage::INVALID_GIF, path, OSTD_FLD);
		if(head->wMagic != 'a9' && head->wMagic != 'a7') throw ExceptionImage(ExceptionImage::INVALID_GIF, path, OSTD_FLD);
		buf += sizeof(HEADER_GIF);
		long bpp((head->flags & (16 + 32 + 64) >> 4) + 1);
		if(head->flags & 128)
		{
			globalPalette = buf;
			sz = (1 << ((head->flags & 7) + 1)) * 3;
			buf += sz;
		}
		// распаковываем
		while((nChunk = (*buf)) != 0x3b)
		{
			if(nChunk == 0x2c)
			{
				BYTE* pal, *buf1, *lzw;
				IMAGE* img;
				GIF_BLOCK_IMAGE* blockImg((GIF_BLOCK_IMAGE*)buf);
				buf += sizeof(GIF_BLOCK_IMAGE);
				long spec(blockImg->flags & 64);
				// параметры
				uint_t width(blockImg->wWidth), height(blockImg->wHeight), szImg(width * height * 4);
				// создаем изображение
				// добавляем
				imgs.add(img = new IMAGE(width, height, ImageFormats::BGRA8));
				BYTE* ptr(img->pixels.move(nullptr, szImg, Ptr<BYTE>::opsMove));
				if(blockImg->flags & 128) { sz = (blockImg->flags & 7) + 1; pal = buf; sz = (1 << sz) * 3; buf += sz; }
				else pal = globalPalette;
				// формат
				GIF_FMT* stk(&gif_fmt[*buf++]);
				sz = 0;
				buf1 = buf;
				while((szImg = (*buf1++)) != 0) { buf1 += szImg; sz += szImg; }
				// буфер распаковки
				Ptr<BYTE> plzw(sz);
				lzw = plzw;
				while((szImg = (*buf++)) != 0) { memcpy(lzw, buf, szImg); lzw += szImg; buf += szImg; }
				// декодируем
				asmDecodeGIF(ptr, pal, stk, plzw, iTrans);
				count++;
				if(spec)
				{
					// перераспределение строк
					Ptr<BYTE> pix(img->pixels.size());
					BYTE* pixels1(ptr), *pix1(pix);
					uint_t pitch(width * 4), c, i;
					// pass 1
					c = height / 8;
					// каждая 8 строка
					for(i = 0; i < c; i++) { memcpy(pix1, pixels1, pitch); pix1 += 8 * pitch; pixels1 += pitch; }
					// pass 2
					pix1 = pix + 4 * pitch;
					for(i = 0; i < c; i++) { memcpy(pix1, pixels1, pitch); pix1 += 8 * pitch; pixels1 += pitch; }
					// pass 3
					c = height / 4;
					pix1 = pix + 2 * pitch;
					for(i = 0; i < c; i++) { memcpy(pix1, pixels1, pitch); pix1 += 4 * pitch; pixels1 += pitch; }
					// pass 4
					c = height / 2;
					pix1 = pix + pitch;
					for(i = 0; i < c; i++) { memcpy(pix1, pixels1, pitch); pix1 += 2 * pitch; pixels1 += pitch; }
					img->pixels.move(pix, Ptr<BYTE>::opsMove);
				}
			}
			else
			{
				switch(*(buf + 1))
				{
					case 0xf9:
						blockImgEx = (GIF_BLOCK_IMAGE_EX*)buf;
						buf += sizeof(GIF_BLOCK_IMAGE_EX);
						if(blockImgEx->flags & 1) iTrans = blockImgEx->iTransparent;
						break;
					case 0xfe:
						buf += sizeof(GIF_BLOCK_COMMENT);
						// перемотать байты комментария
						while((sz = *buf++)) buf += sz;
						break;
					case 0xff:
						blockApp = (GIF_BLOCK_APPLICATION*)buf;
						buf += (3 + blockApp->bSize);
						// перемотать байты приложения
						while((sz = *buf++)) buf += sz;
						break;
					case 0x01:
						buf += sizeof(GIF_BLOCK_TEXT);
						// перемотать байты текста
						while((sz = *buf++)) buf += sz;
						break;
					default:  throw ExceptionImage(ExceptionImage::INVALID_GIF, path, OSTD_FLD);
				}
			}
		}
	}

	void ImgIO::makeJPG(const String& path, BYTE* buf)
	{
		try
		{
			OSTD_TRACE("%s(%s)", __FUNCTION__, path);
			IMAGE* img;
			jpeg_decompress_struct cinfo;
			jpeg_source_mgr src;
			// заполняем структуру jpeg
			jpeg_create_decompress(&cinfo);
			src.next_input_byte = buf;
			src.bytes_in_buffer = 0;
			cinfo.src = &src;
			// инициализация библиотеки
			jpeg_read_header(&cinfo, true);
			jpeg_start_decompress(&cinfo);
			// параметры
			uint_t width(cinfo.output_width), height(cinfo.output_height), num(cinfo.num_components), sz(width * height * num);
			// создаём
			imgs.add(img = new IMAGE(width, height, (num == 3 ? ImageFormats::BGR8 : ImageFormats::Luminance)));
			BYTE* tmp(img->pixels.move(nullptr, sz, Ptr<BYTE>::opsMove));
			// считываем изображение по скан линиям
			while(cinfo.output_scanline < (DWORD)height) tmp += jpeg_read_scanlines(&cinfo, &tmp, 1) * width * num;
			// деинициализируем библиотеку
			jpeg_finish_decompress(&cinfo);
			jpeg_destroy_decompress(&cinfo);
		}
		catch(...) { throw ExceptionImage(ExceptionImage::INVALID_JPG, path, OSTD_FLD); }
	}
}
*/