
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

#define SCHEME_BEGIN(cls) static SCHEME cls##_scheme[] = {
#define SCHEME_END(cls) {nullptr, nullptr, nullptr, 0, 0, 0, 0, 0, 0}}; return cls##_scheme;
#define SCHEME_VAR(cls, var, name, flgs, count, def, stk, id) {name, def, stk, typeid(var).hash_code(), offsetof(cls, var), count, flgs, id, sizeof(decltype(cls::var))},
#define SCHEME_OBJ_VAR(cls, cls_var, var, name, flgs,  count, def, stk, id) {name, def, stk, typeid(cls_var.var).hash_code(), offsetof(cls, var), count, flgs, id, sizeof(decltype(cls::var))},
#define SCHEME_NOD(cls, var, name) {name, nullptr, nullptr, 0, offsetof(cls, var), 0, SC_NODE, 0, 0},
#define SCHEME_OBJ(cls, var, name, id) {name, nullptr, nullptr, 0, offsetof(cls, var), 0, SC_OBJ, id, 0},

namespace ssh
{
	class SSH Serialize
	{
	public:
		struct SCHEME
		{
			// имя (для простых - атрибута, для вложенных - узла)
			ssh_wcs name;
			// значение по умолчанию
			ssh_wcs def;
			// структуры для преобразования флагов и перечислений
			ENUM_DATA* stk;
			// хэш типа
			ssh_u hash;
			// смещение переменной
			ssh_u offs;
			// количество элементов в переменной
			ssh_w count;
			// доступ к переменной
			ssh_w flags;
			// ID - для вложенных объектов без схемы(разделитель, если несколько вложенны друг в друга)
			ssh_w ID;
			// размер типа в байтах
			ssh_w width;
		};
		Serialize() {}
		virtual ~Serialize() {}
		virtual SCHEME* get_scheme() const = 0;
		void openXml(const Buffer<ssh_cs>& buf, ssh_b* srlz)
		{
			Xml xml(buf);
			_sc = get_scheme();
			readXml(&xml, xml.root(), srlz - (ssh_b*)this);
		}
		void openBin(const Buffer<ssh_cs>& buf, ssh_b* srlz)
		{
			SCHEME* sc(get_scheme());
			_sc = get_scheme();
			ssh_cs* _buf(buf);
			readBin(&_buf, srlz - (ssh_b*)this);
		}
		void saveXml(const String& path, ssh_wcs code, ssh_b* srlz)
		{
			Xml xml;
			_sc = get_scheme();
			writeXml(&xml, xml.root(), srlz - (ssh_b*)this);
			xml.save(path, code);
		}
		void saveBin(const String& path, ssh_b* srlz)
		{
			File f(path, File::create_write);
			_sc = get_scheme();
			writeBin(&f, srlz - (ssh_b*)this);
		}
	protected:
		String getVal(ssh_u flgs, ssh_u offs, SCHEME* sc);
		virtual void readXml(Xml* xml, HXML h, ssh_u p_offs);
		virtual void writeXml(Xml* xml, HXML h, ssh_l p_offs);
		virtual void writeBin(File* f, ssh_u p_offs);
		virtual void readBin(ssh_cs** buf, ssh_u p_offs);
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
		static SCHEME* _sc;
	};
}