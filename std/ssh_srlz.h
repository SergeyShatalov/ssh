
#pragma once

#include "ssh_xml.h"

struct SSH_2VAL
{
	ssh_u hash;
	ssh_u offs;
};

#define SC_NODE			0x0001// узел - ссылка на вложенный со схемой
#define SC_OBJ			0x0002// класс или его переменная без схемы
#define SC_ARRAY		0x0004// массив из POD типов
// опции
#define SC_ENUM			0x0008// перечисление
#define SC_FLAGS		0x0010// флаги
#define SC_T_HEX		0x0020// данные в hex
#define SC_T_BOOL		0x0040// данные bool

#define AOV(cls, offs) return asm_offset_var(#cls, &offs);
#define SCHEME_BEGIN(cls) static SCHEME cls##_scheme[] = {
#define SCHEME_END(cls) {nullptr, 0, 0, 1, 0, nullptr, nullptr, 0, 0}}; return cls##_scheme;
#define SCHEME_VAR(cls, var, name, flgs, count, def, stk, id) {name, typeid(var).hash_code(), offsetof(cls, var), count, flgs, def, stk, id, sizeof(decltype(var))},
#define SCHEME_NOD(cls, var, name) {name, typeid(var).hash_code(), offsetof(cls, var), 1, SC_NODE, nullptr, nullptr, 0, 0},
#define SCHEME_OBJ(cls, var, name) {name, typeid(var).hash_code(), offsetof(cls, var), 1, SC_OBJ, nullptr, nullptr, 0, 0},

namespace ssh
{
	class SSH Serialize
	{
	public:
		struct SCHEME
		{
			// имя (для простых - атрибута, для вложенных - узла)
			ssh_wcs name;
			// имя типа
			ssh_u type;
			// смещение переменной
			ssh_u offs;
			// количество элементов в переменной
			ssh_u count;
			// доступ к переменной
			ssh_u flags;
			// значение по умолчанию
			ssh_wcs def;
			// структуры для преобразования флагов и перечислений
			ENUM_DATA* stk;
			// ID - для вложенных объектов без схемы(разделитель, если несколько вложенны друг в друга)
			ssh_u ID;
			// размер типа в байтах
			ssh_u width;
		};
		Serialize() {}
		virtual ~Serialize() {}
		virtual SCHEME* get_scheme() const = 0;
		void openXml(const Buffer<ssh_cs>& buf)
		{
			Xml xml(buf);
			SCHEME* sc(get_scheme());
			readXml(&xml, xml.root(), &sc);
		}
		void openBin(const Buffer<ssh_cs>& buf)
		{
			SCHEME* sc(get_scheme());
			ssh_cs* _buf(buf);
			readBin(&_buf, &sc);
		}
		void saveXml(const String& path, ssh_wcs code)
		{
			Xml xml;
			SCHEME* sc(get_scheme());
			writeXml(&xml, xml.root(), &sc);
			xml.save(path, code);
		}
		void saveBin(const String& path)
		{
			File f(path, File::create_write);
			SCHEME* sc(get_scheme());
			writeBin(&f, &sc);
		}
	protected:
		String getVal(ssh_u flgs, ssh_u offs, SCHEME* sc);
		virtual void readXml(Xml* xml, HXML h, SCHEME** arr, ssh_u p_offs = 0);
		virtual void writeXml(Xml* xml, HXML h, SCHEME** arr, ssh_u p_offs = 0);
		virtual void writeBin(File* f, SCHEME** arr, ssh_u p_offs = 0);
		virtual void readBin(ssh_cs** buf, SCHEME** arr, ssh_u p_offs = 0);
		static const ssh_u _hash_string = 0x6a979454ce7cff60;
		static const ssh_u _hash_int	= 0x2b9fff19004b3727;
		static const ssh_u _hash_uint	= 0xbaaedcffb89ab934;
		static const ssh_u _hash_long	= 0xcde8c9adbd39ae9e;
		static const ssh_u _hash_ulong	= 0xa8aa3869cd5eb495;
		static const ssh_u _hash_char	= 0xf2a39391f9f8ad2c;
		static const ssh_u _hash_uchar	= 0xf9b6d9fdbc918e1b;
		static const ssh_u _hash_short	= 0xf69155480110981d;
		static const ssh_u _hash_ushort = 0x27a436e029489774;
		static const ssh_u _hash_wchar	= 0xcd07488533365414;
		static const ssh_u _hash_float	= 0xa00a62a9e2b863cc;
		static const ssh_u _hash_double = 0xa0880a9c41b9d434;
		static const ssh_u _hash_half	= 0x384de03da92b42f5;
		static const ssh_u _hash_ll		= 0xf36f7584055d450a;
		static const ssh_u _hash_ull	= 0x4422422121b2ac2a;
		static const ssh_u _hash_wcs	= 0x667978a73e944305;
		static const ssh_u _hash_ccs	= 0xb5f5c54e0cef1cc0;
		static const ssh_u _hash_time	= 0x7f407f2070a5d6d0;
	};
}