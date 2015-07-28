
#pragma once

#include "ssh_tree.h"
#include "ssh_list.h"

namespace ssh
{
	struct XmlNode
	{
		// конструктор
		XmlNode(ssh_wcs name, ssh_wcs v) : nm(name), val(v) {}
		// вернуть атрибут по имени
		XmlNode* attr(const String& name) const { auto n(attrs.find(name, name)); return (n ? n->value : nullptr); }
		// вернуть имя
		const String& name() const { return nm; }
		// вернуть тип
		const String& type() const { return nm; }
		// имя узла/атрибута
		String nm;
		// значение узла/атрибута
		String val;
		// список атрибутов
		List<XmlNode*> attrs;
	};

	typedef Tree<XmlNode*>::Node* HXML;

	class SSH Xml
	{
	public:
		// конструкторы
		Xml() : code(L"utf-8") { init(); }
		// конструктор загрузки
		Xml(const String& path, ssh_wcs cod) { open(path, cod); }
		// конструктор из памяти
		Xml(const Buffer<ssh_cs>& buf, ssh_wcs cod);
		// деструктор
		virtual ~Xml() { close(); }
		// открыть
		void open(const String& path, ssh_wcs cod);
		// закрыть
		void close() { init(); }
		// сохранить
		void save(const String& path);
		// вернуть количество дочерних
		ssh_u count(HXML h) const { return h->count; }
		// установить имя узла
		void set_name(HXML h, ssh_wcs name) { h->value->nm = name; }
		// вернуть имя узла
		const String& get_name(HXML h) const { return h->value->nm; }
		// получить родительский узел
		HXML parent(HXML h) const { return h->parent; }
		// получить корневой узел
		HXML root() const { return tree.get_root(); }
		// установить значение узла
		template <typename T> void set_val(HXML h, const T& val) const
		{
			h->value->val = val;
		}
		// получить значение узла
		template <typename T> T val(HXML h) const
		{
			return h->value->val;
		}
		// установить значение атрибута
		template <typename T> void set_attr(HXML h, ssh_wcs name, const T& val) const
		{
			auto n(h->value->attr(name));
			if(n) n->val = val; else h->value->attrs.add(new XmlNode(name, val));
		}
		// получить значение атрибута
		template <typename T> T get_attr(HXML h, ssh_wcs name, const T& def) const
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
		HXML get_node(HXML h, ssh_wcs name, ssh_l index = -1) const
		{
			if(name) return tree.findChild(h, name);
			return tree.get_node(h, index);
		}
	protected:
		// начальная инициализация
		void init();
		// декодировка
		String encode(const Buffer<ssh_cs>& buf);
		// вернуть BOM в зависимости от выходной кодировки
		ssh_w bom_coder() const;
		// формирование дерева узлов
		void _make(const Buffer<ssh_cs>& buf);
		void make(regx* rx, HXML hp, ssh_u lev);
		//void make(HXML h, ssh_u _lev);
		// сохранение
		String _save(HXML h, ssh_l level);
		// дерево узлов
		Tree<XmlNode*> tree;
		// указатель на текст при формировании дерева
		static ssh_ws* _xml;
		// выходная кодировка
		String code;
	};
}
