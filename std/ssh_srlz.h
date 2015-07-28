
#pragma once

#include "ssh_xml.h"

struct SSH_2VAL
{
	ssh_u hash;
	ssh_u offs;
};

#define SC_NODE			0x0001// ���� - ������ �� ��������� �� ������
#define SC_OBJ			0x0002// ����� ��� ��� ���������� ��� �����
#define SC_ARRAY		0x0004// ������ �� POD �����
// �����
#define SC_ENUM			0x0008// ������������
#define SC_FLAGS		0x0010// �����
#define SC_T_HEX		0x0020// ������ � hex
#define SC_T_BOOL		0x0040// ������ bool

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
			// ��� (��� ������� - ��������, ��� ��������� - ����)
			ssh_wcs name;
			// ��� ����
			ssh_u type;
			// �������� ����������
			ssh_u offs;
			// ���������� ��������� � ����������
			ssh_u count;
			// ������ � ����������
			ssh_u flags;
			// �������� �� ���������
			ssh_wcs def;
			// ��������� ��� �������������� ������ � ������������
			ENUM_DATA* stk;
			// ID - ��� ��������� �������� ��� �����(�����������, ���� ��������� �������� ���� � �����)
			ssh_u ID;
			// ������ ���� � ������
			ssh_u width;
		};
		Serialize() {}
		virtual ~Serialize() {}
		virtual SCHEME* get_scheme() const = 0;
		void openXml(const Buffer<ssh_cs>& buf)
		{
			Xml xml(buf);
			readXml(&xml, xml.root(), get_scheme());
		}
		void openBin(const Buffer<ssh_cs>& buf)
		{
			//readBin(buf);
		}
		void saveXml(const Buffer<ssh_cs>& buf, const String& path, ssh_wcs code)
		{
			Xml xml(buf);
			writeXml(&xml, xml.root(), get_scheme());
			xml.save(path, code);
		}
		void saveBin(const String& path)
		{
		}
	protected:
		String getVal(ssh_u flgs, ssh_u offs, SCHEME* sc);
		virtual SCHEME* readXml(Xml* xml, HXML h, SCHEME* sc, ssh_u p_offs = 0);
		virtual SCHEME* writeXml(Xml* xml, HXML h, SCHEME* sc, ssh_u p_offs = 0);
		//virtual void writeBin(File* f, base_scheme::SCHEME_DESC* parent);
		//virtual void readBin(BYTE* ptr, base_scheme::SCHEME_DESC* parent);
		//virtual void specWriteXml(Xml* xml, HXML h, uint_t idx, uint_t id) {}
		//virtual void specReadXml(Xml* xml, HXML h, uint_t idx, uint_t id) {}
		//virtual void specWriteBin(File* f, uint_t idx, uint_t id) {}
		//virtual void specReadBin(BYTE* ptr, uint_t idx, uint_t id) {}
	};
}