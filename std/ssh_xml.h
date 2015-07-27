
#pragma once

#include "ssh_tree.h"
#include "ssh_list.h"

namespace ssh
{
	struct XmlNode
	{
		// �����������
		XmlNode(ssh_wcs name, ssh_wcs v) : nm(name), val(v) {}
		// ������� ������� �� �����
		XmlNode* attr(const String& name) const { auto n(attrs.find(name, name)); return (n ? n->value : nullptr); }
		// ������� ���
		const String& name() const { return nm; }
		// ������� ���
		const String& type() const { return nm; }
		// ��� ����/��������
		String nm;
		// �������� ����/��������
		String val;
		// ������ ���������
		List<XmlNode*> attrs;
	};

	typedef Tree<XmlNode*>::Node* HXML;

	class SSH Xml
	{
	public:
		enum Cod
		{
			_utf8 = 0,
			_utf16le,
			_utf16be,
			_ansi
		};
		// ������������
		Xml() { init(); }
		// ����������� ��������
		Xml(const String& path, Cod cod) { open(path, cod); }
		// ����������� �� ������
		Xml(const Buffer<ssh_cs>& buf, Cod cod);
		// ����������
		virtual ~Xml() { close(); }
		// �������
		void open(const String& path, Cod cod);
		// �������
		void close() { init(); }
		// ���������
		void save(const String& path);
		// ������� ���������� ��������
		ssh_u count(HXML h) const { return h->count; }
		// ���������� ��� ����
		void set_name(HXML h, ssh_wcs name) { h->value->nm = name; }
		// ������� ��� ����
		const String& get_name(HXML h) const { return h->value->nm; }
		// �������� ������������ ����
		HXML parent(HXML h) const { return h->parent; }
		// �������� �������� ����
		HXML root() const { return tree.get_root(); }
		// ���������� �������� ����
		template <typename T> void set_val(HXML h, const T& val) const
		{
			h->value->val = val;
		}
		// �������� �������� ����
		template <typename T> T val(HXML h) const
		{
			return h->value->val;
		}
		// ���������� �������� ��������
		template <typename T> void set_attr(HXML h, ssh_wcs name, const T& val) const
		{
			auto n(h->value->attr(name));
			if(n) n->val = val; else h->value->attrs.add(new XmlNode(name, val));
		}
		// �������� �������� ��������
		template <typename T> T get_attr(HXML h, ssh_wcs name, const T& def) const
		{
			auto n(h->value->attr(name));
			return (n ? n->val : def);
		}
		// ������� ������� ������� ��������
		bool is_attr(HXML h, ssh_wcs name) const
		{
			return (h->value->attr(name) != nullptr);
		}
		// �������� ����
		HXML add_node(HXML h, ssh_wcs name, ssh_wcs val)
		{
			return tree.add(h, new XmlNode(name, val));
		}
		// �������/�������� ����
		HXML get_node(HXML h, ssh_wcs name, ssh_l index = -1) const
		{
			if(name) return tree.findChild(h, name);
			return tree.get_node(h, index);
		}
	protected:
		// ��������� �������������
		void init();
		// �����������
		String encode(const Buffer<ssh_cs>& buf);
		// ���������� �����
		ssh_ws* _word(int sub = 0);
		// ������� ��������� ��������� ��� ���������� XML
		ssh_wcs get_param_coder(ssh_w& bom);
		// ������� "������" ��������
		void _skip_spc();
		// ������������ ������ �����
		void make(regx* rx, HXML hp, ssh_u lev);
		void make(HXML h, ssh_u _lev);
		// ����������
		String _save(HXML h, ssh_l level);
		// ������ �����
		Tree<XmlNode*> tree;
		static ssh_ws* _xml;
		Cod coder;
	};
}
