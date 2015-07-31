
#pragma once

#include "ssh_tree.h"
#include "ssh_list.h"

namespace ssh
{
	struct XmlNode
	{
		// �����������
		XmlNode() : attrs(nullptr), next(nullptr) {}
		XmlNode(ssh_wcs name, ssh_wcs v) : XmlNode() { nm = name; val = v; }
		~XmlNode() { SSH_DEL(attrs); SSH_DEL(next); }
		// ������� ������� �� �����
		XmlNode* attr(const String& name) const
		{
			auto n(attrs);
			while(n)
			{
				if(n->nm == name) return n;
				n = n->next;
			}
			return nullptr;
		}
		void add_attr(XmlNode* n)
		{
			if(attrs) last->next = n; else attrs = n;
			last = n;
		}
		// ������� ���
		const String& name() const { return nm; }
		//const String& type() const { return nm; }
		// ��� ����/��������
		String nm;
		// �������� ����/��������
		String val;
		// ������ ��������� - ������
		XmlNode* attrs;
		// ��������� � ������
		XmlNode* last;
		// ��������� �������
		XmlNode* next;
	};

	typedef Tree<XmlNode*>::Node* HXML;

	class SSH Xml
	{
	public:
		// ������������
		Xml() { }
		// ����������� ��������
		Xml(const String& path) { open(path); }
		// ����������� �� ������
		Xml(const Buffer<ssh_cs>& buf);
		// ����������
		virtual ~Xml() { close(); }
		// �������
		void open(const String& path);
		// �������
		void close() { tree.reset(); }
		// ���������
		void save(const String& path, ssh_wcs cod);
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
			if(n) n->val = val; else h->value->add_attr(new XmlNode(name, val));
		}
		// �������� �������� ��������
		template <typename T> T attr(HXML h, ssh_wcs name, const T& def) const
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
		HXML node(HXML h, ssh_wcs name, ssh_l index = -1) const
		{
			if(name) return tree.findChild(h, name);
			return tree.get_node(h, index);
		}
		static regx* get_regx()
		{
			static ssh_wcs ptrs[] = {LR"serg((?mUs)<(?:(?:([/]{0,1})([\w_]+[\w\d_-]*)>)|(!--.*-->)|(?:([A-Za-z�-��-�_]+[A-Za-z�-��-�\d_-]*)\s+([A-Za-z�-��-�_]+.*)([/]{0,1})>)))serg", LR"serg((?sm)([A-Za-z�-��-�_]+[A-Za-z�-��-�\d_]*)\s*=\s*(?:"(.*?)")\s*)serg",
									 LR"serg((?ms)()?<=>"(.*?)")serg", LR"((?im)<\?xml\s+version=.+encoding=["]?(.*?)["]?\s*\?>)"};
			static regx rx(ptrs, 4);
			return &rx;
		}
	protected:
		// �����������
		String encode(const Buffer<ssh_cs>& buf);
		// ������� BOM � ����������� �� �������� ���������
		ssh_w bom_coder() const;
		// ������������ ������ �����
		void _make(const Buffer<ssh_cs>& buf);
		void make(HXML hp, ssh_u lev);
		//void make(HXML h, ssh_u _lev);
		// ����������
		String _save(HXML h, ssh_l level);
		// ������ �����
		Tree<XmlNode*, SSH_PTR> tree;
		// ��������� �� ����� ��� ������������ ������
		static ssh_ws* _xml;
	};
}
