
#pragma once

#include "ssh_tree.h"
#include "ssh_list.h"

namespace ssh
{
	struct XmlNode
	{
		// конструктор
		XmlNode() : attrs(nullptr), next(nullptr) {}
		XmlNode(ssh_wcs name, const String& v) : XmlNode() { nm = name; val = v; }
		~XmlNode() { SSH_DEL(attrs); SSH_DEL(next); }
		// вернуть атрибут по имени
		XmlNode* attr(ssh_wcs name) const
		{
			auto h(ssh_hash(name));
			auto n(attrs);
			while(n)
			{
				if(n->nm.hash() == h) return n;
				n = n->next;
			}
			return nullptr;
		}
		void add_attr(XmlNode* n)
		{
			if(attrs) last->next = n; else attrs = n;
			last = n;
		}
		// вернуть имя
		const String& name() const { return nm; }
		//const String& type() const { return nm; }
		// имя узла/атрибута
		String nm;
		// значение узла/атрибута
		String val;
		// список атрибутов - корень
		XmlNode* attrs;
		// последний в списке
		XmlNode* last;
		// следующий атрибут
		XmlNode* next;
	};

	typedef Tree<XmlNode*>::Node* HXML;

	class SSH Xml
	{
	public:
		// конструкторы
		Xml() { }
		// конструктор загрузки
		Xml(const String& path) { open(path); }
		// конструктор из памяти
		Xml(const Buffer<ssh_cs>& buf);
		// деструктор
		virtual ~Xml() { close(); }
		// открыть
		void open(const String& path);
		// закрыть
		void close() { tree.reset(); }
		// сохранить
		void save(const String& path, ssh_wcs cod);
		// вернуть количество дочерних
		ssh_u count(HXML h) const { return h->count; }
		// установить имя узла
		void set_name(HXML h, ssh_wcs name) { h->value->nm = name; }
		// вернуть имя узла
		String get_name(HXML h) const { return h->value->nm; }
		// получить родительский узел
		HXML parent(HXML h) const { return h->parent; }
		// получить корневой узел
		HXML root() const { return tree.get_root(); }
		// установить значение узла
		template <typename T> void set_val(HXML h, const T& val)
		{
			h->value->val = val;
		}
		// получить значение узла
		template <typename T> T val(HXML h) const
		{
			return h->value->val;
		}
		// установить значение атрибута
		template <typename T> void set_attr(HXML h, ssh_wcs name, const T& val)
		{
			auto n(h->value->attr(name));
			if(n) n->val = String(val); else h->value->add_attr(new XmlNode(name, String(val)));
		}
		// получить значение атрибута
		template <typename T> T attr(HXML h, ssh_wcs name, const T& def) const
		{
			auto n(h->value->attr(name));
			return (n ? n->val : def);
		}
		// вернуть признак наличия атрибута
		bool is_attr(HXML h, ssh_wcs name) const
		{
			return (h->value->attr(name) != nullptr);
		}
		// добавить узел
		HXML add_node(HXML h, ssh_wcs name, ssh_wcs val)
		{
			return tree.add(h, new XmlNode(name, val));
		}
		// вернуть/добавить узел
		HXML node(HXML h, ssh_wcs name, ssh_l index = -1) const
		{
			if(name) return tree.get_node_hash(h, ssh_hash(name));
			return tree.get_node_index(h->fchild, index);
		}
	protected:
		// декодировка
		String encode(const Buffer<ssh_cs>& buf);
		// формирование дерева узлов
		void make(HXML hp, ssh_u lev);
		void _make(const Buffer<ssh_cs>& buf);
		// сохранение
		String _save(HXML h, ssh_l level);
		// дерево узлов
		Tree<XmlNode*, SSH_PTR> tree{ID_TREE_XML};
		// указатель на текст при формировании дерева
		static ssh_ws* _xml;
	};
}
