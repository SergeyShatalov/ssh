
#pragma once

#include "ssh_mem.h"

namespace ssh
{
	template <typename TYPE, typename KEY, ssh_u ops_val = SSH_PTR, ssh_u ops_key = SSH_TYPE> class Map
	{
	public:
		struct Node
		{
			SSH_NEW_DECL(Node, 128);
			// конструкторы
			Node() : next(nullptr) {}
			Node(const KEY& k, const TYPE& t, Node* n) : next(n), key(k), value(t) {}
			// ключь
			KEY key;
			// значение
			TYPE value;
			// следующий узел
			Node* next;
		};
	protected:
		class Cursor
		{
		public:
			// добавление нового
			Cursor(Map<TYPE, KEY, ops_val, ops_key>* arr, const KEY& k) : node(new Node(k, TYPE(), arr->cells)) {arr->cells = node;}
			// возврат
			Cursor(Node* n) : node(n){}
			Cursor& operator = (const TYPE& value) {BaseNode<TYPE, ops_val>::release(node->value); node->value = value; return *this;}
			operator const TYPE() const {return node->value;}
			TYPE operator->() const {return node->value;}
		protected:
			// узел
			Node* node;
		};
	public:
		// конструктор по умолчанию
		Map() {clear();}
		// конструктор копии
		Map(const Map<TYPE, KEY, ops_val, ops_key>& src) { clear(); *this = src; }
		// конструктор переноса
		Map(Map<TYPE, KEY, ops_val, ops_key>&& src) { cells = src.cells; node = src.node; src.cells = src.node = nullptr; }
		// деструктор
		~Map()
		{
			reset(true);
			Node::get_MemArrayNode()->Reset();
		}
		// очистить
		void clear() { cells = node = nullptr; }
		// присваивание
		const Map& operator = (const Map<TYPE, KEY, ops_val, ops_key>& src) { reset(true); return *this += src; }
		const Map& operator = (Map<TYPE, KEY, ops_val, ops_key>&& src) { reset(true); cells = src.cells; node = src.node; src.cells = src.node = nullptr; return *this; }
		// приращение
		const Map& operator += (const Map<TYPE, KEY, ops_val, ops_key>& src)
		{
			auto n(src.root());
			while(n = src.next()) {operator[](n->key) = n->value;}
			return *this;
		}
		// количество элементов
		ssh_u count() const
		{
			ssh_u c(0);
			Node* n(cells);
			while(n) {n = n->next; c++;}
			return c;
		}
		// установка/возврат
		Cursor operator[](const KEY& key)
		{
			Node* n(cells);
			while(n) {if(n->key == key) return Cursor(n); n = n->next;}
			return Cursor(this, key);
		}
		// вернуть все ключи
		Map<KEY, ssh_u, SSH_TYPE, SSH_TYPE> keys() const
		{
			Node* n(cells);
			Map<KEY, ssh_u, SSH_TYPE, SSH_TYPE> keys;
			ssh_u i = 0;
			while(n)
			{
				keys[i++] = n->key;
				n = n->next;
			}
			return keys;
		}
		// вернуть ключь по значению
		KEY get_key(const TYPE& value) const
		{
			Node* n(cells);
			while(n) {if(n->value == value) return n->key; n = n->next;}
			return BaseNode<KEY, ops_key>::dummy();
		}
		// проверка - такой ключ существует?
		bool is_key(const KEY& key) const
		{
			Node* n(cells);
			while(n) {if(n->key == key) return true; n = n->next;}
			return false;
		}
		// удаление элемента
		void remove(const KEY& key)
		{
			Node* n, *p;
			if((n = get_key(key, &p)))
			{
				// удалить
				Node* nn(n->next); n->next = nullptr;
				cells == n ? cells = nn : p->next = nn;
				BaseNode<TYPE, ops_val>::release(n->value);
				BaseNode<KEY, ops_key>::release(n->key);
				delete n;
			}
		}
		// перебор элементов
		Node* value(Node* n) const { return (n ? n->next : cells); }
		// вернуть корень
		Node* getCells() {return cells;}
		// вернуть текущий ключь 
		const KEY& key() const {return (node ? node->key : cells->key);}
		// вернуть корень
		Node* root() const { return (node = nullptr); }
		// возвращает следующий элемент
		Node* next() const { return (node = node ? node->next : cells); }
	protected:
		// удаление всего
		void reset(bool is_rel)
		{
			auto m(Node::get_MemArrayNode());
			if(m->Valid())
			{
				while(cells)
				{
					if(is_rel)
					{
						BaseNode<TYPE, ops_val>::release(cells->value);
						BaseNode<KEY, ops_key>::release(cells->key);
					}
					auto n(cells->next);
					delete cells;
					cells = n;
				}
			}
			clear();
		}
		// вернуть узел по ключу
		Node* get_key(const KEY& key, Node** p) const
		{
			Node* n(cells);
			while(n) {if(n->key == key) return n; if(p) *p = n; n = n->next;}
			return nullptr;
		}
		// корневой элемент
		Node* cells;
		// текущий элемент
		mutable Node* node;
	};
}
