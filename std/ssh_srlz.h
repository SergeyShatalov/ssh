
#pragma once

#include "ssh_xml.h"

#define SC_NODE			0x0001// узел - ссылка на вложенный со схемой
#define SC_OBJ			0x0002// класс или его переменная без схемы
//#define SC_ARRAY		0x0004// массив из POD типов
// опции
#define SC_ENUM			0x0008// перечисление
#define SC_FLAGS		0x0010// флаги
#define SC_HEX			0x0020// данные в hex
#define SC_BOOL			0x0040// данные bool
#define SC_BIN			0x0080// данные в bin

#define SCHEME_BEGIN(cls) static SCHEME cls##_scheme[] = {
#define SCHEME_END(cls) {nullptr, nullptr, nullptr, 0, 0, 0, 0, 0, 0}}; return cls##_scheme;
#define SCHEME_VAR(cls, var, name, count, flgs, def, stk) {name, def, stk, ssh::ssh_hash_type(typeid(var).raw_name()), offsetof(cls, var), count, flgs, 0, (sizeof(decltype(cls::var)) / count)},
#define SCHEME_OBJ_VAR(cls, cls_var, var, name, count, flgs, def, stk, id) {name, def, stk, ssh::ssh_hash_type(typeid(cls_var.var).raw_name()), offsetof(cls, var), count, flgs | SC_OBJ, id, sizeof(decltype(cls::var))},
#define SCHEME_OBJ_VAR1(cls1, cls1_var, cls2, cls2_var, var, name, count, flgs, def, stk, id) {name, def, stk, ssh::ssh_hash_type(typeid(cls1_var.cls2_var.var).raw_name()), offsetof(cls2, var), count, flgs | SC_OBJ, id, sizeof(decltype(cls2::var))},
#define SCHEME_NOD(cls, var, name, def, count) {name, def, nullptr, 0, offsetof(cls, var), count, SC_NODE, 0, 0},
#define SCHEME_OBJ_BEGIN(cls, var, name, count, id) {name, nullptr, nullptr, 0, offsetof(cls, var), count, SC_OBJ, id, 0},
#define SCHEME_OBJ_END() {L"<!-- -->", nullptr, nullptr, 0, 0, 1, 0, -1, 0},

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
			short ID;
			// размер типа в байтах
			ssh_w width;
		};
		Serialize() {}
		virtual ~Serialize() {}
		virtual SCHEME* get_scheme() const = 0;
		void openXml(const Buffer<ssh_cs>& buf, void* srlz);
		void openBin(const Buffer<ssh_cs>& buf, void* srlz);
		void saveXml(const String& path, ssh_wcs code, void* srlz);
		void saveBin(const String& path, void* srlz);
	protected:
		String getVal(ssh_u flgs, ssh_u offs, SCHEME* sc);
		void setVal(HXML h, ssh_u flg, ssh_u offs, SCHEME* sc);
		void writeVal(ssh_u flg, ssh_u offs, SCHEME* sc);
		//void readVal(ssh_u flg, ssh_u offs, SCHEME* sc);
		virtual void readXml(HXML h, ssh_u p_offs);
		virtual void writeXml(HXML h, ssh_l p_offs);
		virtual void writeBin(ssh_u p_offs);
		virtual void readBin(ssh_u p_offs);
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
		static Xml* _xml;
		static File* _fl;
	};
}