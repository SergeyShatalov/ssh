
#pragma once

#include "ssh_xml.h"

struct SSH_2VAL
{
	ssh_u hash;
	ssh_u offs;
};

#define SC_NODE			0x0001// ���� - ������ �� ��������� �� ������
#define SC_VAR			0x0002// ������� POD ���
#define SC_ARRAY		0x0004// ������ �� POD �����
#define SC_ENUM			0x0008// ������������
#define SC_FLAGS		0x0010// �����
#define SC_OBJ			0x0020// ����� ��� ��� ���������� ��� �����

#define AOV(cls, offs) return asm_offset_var(#cls, &offs);
#define SCHEME_BEGIN(cls) static SCHEME cls##_scheme[] = {
#define SCHEME_END(cls) }; return cls##_scheme;
#define SCHEME_VAR(cls, var, name, flgs, def, stk) {name, typeid(var).hash_code(), offsetof(cls, var), 1, flgs, def, stk, 0}
#define SCHEME_ARR(cls, var, name, count, flgs, def, stk) {name, (SSH_2VAL)offset_val<decltype(var), cls>(&cls::var), count, flgs, def, stk, 0}
#define SCHEME_NOD(cls, var, name) {name, (SSH_2VAL)offset_val<decltype(var), cls>(&cls::var), 1, SC_NODE, L"", nullptr, 0}
#define SCHEME_CLS(cls, var, name, id) {name, (SSH_2VAL)offset_val<decltype(var), cls>(&cls::var), 1, SC_OBJ, L"", nullptr, id}

namespace ssh
{
	class SSH Serialize
	{
	public:
		struct SCHEME
		{
			// ��� (��� ������� - ��������, ��� ��������� - ����)
			String name;
			// ��� ����
			ssh_u hash;
			// �������� ����������
			ssh_u offs;
			// ���������� ��������� � ����������
			ssh_u count;
			// ������ � ����������
			ssh_u flags;
			// �������� �� ���������
			String def;
			// ��������� ��� �������������� ������ � ������������
			ENUM_DATA* stk;
			// ID - ��� ��������� �������� ��� �����(�����������, ���� ��������� �������� ���� � �����)
			ssh_u ID;
		};
		Serialize() {}
		virtual ~Serialize() {}
		virtual SCHEME* get_scheme() const = 0;
		void openXml(const String& tag, const Buffer<ssh_cs>& buf)
		{
			Xml xml(buf);
			get_scheme();
			scheme_root.name = tag;
			readXml(&xml, xml.root(), &scheme_root);
		}
		void openBin(const Buffer<ssh_cs>& buf)
		{
			//readBin(buf);
		}
		void saveXml(const String& tag, const Buffer<ssh_cs>& buf, const String& path, ssh_wcs code)
		{
			Xml xml(buf);
			scheme_root.name = tag;
			writeXml(&xml, xml.root(), &scheme_root);
			xml.save(path, code);
		}
		void saveBin(const String& path)
		{
		}
	protected:
		virtual void readXml(Xml* xml, HXML h, SCHEME* sc, ssh_u p_offs = 0);
		virtual void writeXml(Xml* xml, HXML h, SCHEME* sc, ssh_u p_offs = 0);
		//virtual void writeBin(File* f, base_scheme::SCHEME_DESC* parent);
		//virtual void readBin(BYTE* ptr, base_scheme::SCHEME_DESC* parent);
		//virtual void specWriteXml(Xml* xml, HXML h, uint_t idx, uint_t id) {}
		//virtual void specReadXml(Xml* xml, HXML h, uint_t idx, uint_t id) {}
		//virtual void specWriteBin(File* f, uint_t idx, uint_t id) {}
		//virtual void specReadBin(BYTE* ptr, uint_t idx, uint_t id) {}
		static SCHEME scheme_root;
	};
}