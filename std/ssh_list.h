
#pragma once

#include "ssh_mem.h"

namespace ssh
{
	template <typename TYPE, ssh_u ops = SSH_PTR> class List
	{
		friend class Base;
	public:
		struct Node
		{
			SSH_NEW_DECL(Node, 128);
			Node() : prev(nullptr), next(nullptr) {}
			// конструктор
			Node(const TYPE& t, Node* p, Node* n) : prev(p), next(n), value(t) {}
			// следующий
			Node* next;
			// предыдущий
			Node* prev;
			// значение
			TYPE value;
		};
		// конструктор
		List() : ID(-1) {init();}
		List(ssh_u _ID) : ID(_ID) { init(); }
		// конструктор копии
		List(ssh_u _ID, const List<TYPE, ops>& src) : ID(_ID) { init(); *this = src; }
		// конструктор переноса
		List(List<TYPE, ops>&& src) { nroot = src.nroot; nlast = src.nlast; node = src.node; src.init(); }
		// деструктор
		~List()
		{
			reset(true);
			Node::get_MemArrayNode()->Reset(); 
		}
		// установить идентификатор
		void setID(ssh_u _ID) { ID = _ID; }
		// очистить
		void clear() { init(); }
		// освободить
		void free() { reset(true); }
		// приращение
		const List& operator += (const List<TYPE, ops>& src) {auto n(src.root()); while(n = src.next()) add(n->value); return *this;}
		const List& operator += (const TYPE& t) { add(t); return *this; }
		// присваивание
		const List& operator = (const List<TYPE, ops>& src) {reset(true); return (*this += src);}
		const List& operator = (List<TYPE, ops>&& src) { reset(true); nroot = src.nroot; nlast = src.nlast; node = src.node; src.init(); return *this; }
		// добавление
		Node* add(const TYPE& t)
		{
			node = new Node(t, nlast, nullptr);
			if(nroot) nlast->next = node; else nroot = node;
			return (nlast = node);
		}
		// вставка элемента
		Node* insert(const TYPE& t, Node* n = nullptr)
		{
			n = (n ? node : nroot);
			auto nn(n ? n->next : nullptr);
			auto np(n ? n->prev : nullptr);
			auto nd(new Node(t, np, nn));
			if(nn) nn->prev = nd;
			if(np) np->next = nd;
			if(n == nroot || !n) nroot = nd;
			if(!nlast) nlast = nd;
		}
		// удалить
		Node* remove(Node* nd = nullptr)
		{
			nd = (nd ? nd : node);
			if(nd)
			{
				Node* n(nd->next);
				Node* p(nd->prev);
				if(nd == nroot) nroot = n;
				if(nd == nlast) nlast = p;
				if(n) n->prev = p;
				if(p) p->next = n;
				node = (n ? n : p);
				BaseNode<TYPE, ops>::release(nd->value);
				delete nd;
			}
			return node;
		}
		// перейти в "корень"
		Node* root() const { return (node = nullptr); }
		// следующий
		Node* next() const { return (node = (node ? node->next : nroot)); }
		// предыдущий
		Node* prev() const { return (node = (node ? node->prev : nlast)); }
		// найти
		Node* find(const TYPE& value) const { root(); while(next() && node->value != value) {} return node; }
		// найти по имени
		Node* find(const String& name, const String& type) const
		{
			root();
			while(next())
			{
				if(BaseNode<TYPE, ops>::hash(node->value, true) == name.hash())
					if(BaseNode<TYPE, ops>::hash(node->value, false) == type.hash()) break;
			}
			return node;
		}
		bool is_empty() const { return (nroot == nullptr); }
	protected:
		// инициализация
		void init() { nroot = nlast = node = nullptr; }
		// сброс
		void reset(bool is_rel)
		{
			if(Node::get_MemArrayNode()->Valid())
			{
				while(nroot)
				{
					auto nr(nroot);
					if(is_rel) BaseNode<TYPE, ops>::release(nroot->value);
					if(nr != nroot) continue;
					auto n(nroot->next);
					SSH_DEL(nroot);
					nroot = n;
				}
			}
			init();
		}
		// корень
		Node* nroot;
		// последний
		Node* nlast;
		// идентификатор
		ssh_u ID;
		// текущий
		mutable Node* node;
	};
}
