
#pragma once

#include "ssh_mem.h"

namespace ssh
{
	template <typename T, ssh_u ops = SSH_PTR> class Tree
	{
	public:
		struct Node
		{
			SSH_NEW_DECL(Node, 512);
			Node() : fchild(nullptr), lchild(nullptr), next(nullptr), prev(nullptr), parent(nullptr), count(0){}
			Node(const T& t, Node* p) : fchild(nullptr), lchild(nullptr), next(nullptr), prev(nullptr), parent(p), count(0), value(t) {}
			// значение
			T value;
			// первый дочерний
			Node* fchild;
			// последний дочерний
			Node* lchild;
			// следующий на уровне
			Node* next;
			// предыдущий на уровне
			Node* prev;
			// родитель (уровень иерархии -1 от текущего)
			Node* parent;
			// количество дочерних
			ssh_u count;
		};
		// конструктор
		Tree() {clear(); }
		// конструктор копии
		Tree(const Tree<T, ops>& src) { clear(); *this = src; }
		Tree(Tree<T, ops>&& src) {root = src.root; src.root = nullptr;}
		// деструктор
		~Tree()
		{
			reset();
			Node::get_MemArrayNode()->Reset();
		}
		// очистить
		void clear() { root = nullptr; }
		// сброс
		void reset()
		{
			if(Node::get_MemArrayNode()->Valid())
				reset(root);
		}
		// присваивание
		const Tree& operator = (const Tree<T, ops>& src) { reset(); return *this += src; }
		const Tree& operator = (Tree<T, ops>&& src) { reset(root); root = src.root; src.root = nullptr; return *this; }
		// приращение
		const Tree& operator += (const Tree<T, ops>& src) { add(root, src); return *this; }
		// удаление узла
		void remove(Node* n) {remove(n, true);}
		// установить значение узла
		void set(Node* n, const T& t) { BaseNode<T, ops>::release(n->value); n->value = t; }
		// возможно ли перемещение узла? в первом случае p = перемещаемый узел->fchild
		bool is_move(Node* p, Node* n) const
		{
			// пройтись по всем дочерним от родительского
			while(p)
			{
				if(p == n) return false;
				if(p->fchild) {if(!is_move(p->fchild, n)) return false;}
				p = p->next;
			}
			return true;
		}
		// переместить
		Node* move(Node* n, bool is_last) {remove(n, false); return (is_last ? add(n->parent, n) : insert(n->parent, n)); }
		// добавить дочерний
		Node* add(Node* n, const T& t) { return add(n, new Node(t, n)); }
		// добавить дерево
		Node* add(Node* n, const Tree<T, ops>& src) { return add(n, src.root); }
		// вставить дочерний		
		Node* insert(Node* n, const T& t) { return insert(n, new Node(t, n)); }
		// вставить другое дерево
		Node* insert(Node* n, const Tree<T, ops>& src) { return insert(n, src.get_root()); }
		// вернуть узел по индексу
		Node* get_node(Node* n, ssh_u idx) const { while(n) { if(--idx < 0) return n; n = n->next; } return nullptr; }
		// найти дочерний по значению
		Node* findChild(Node* n, const T& t) const
		{
			Node* nn;
			while(n)
			{
				if(n->value == t) return n;
				if(n->fchild) {if((nn = findChild(n->fchild, t))) return nn;}
				n = n->next;
			}
			return nullptr;
		}
		// найти дочерний по имени
		Node* findChild(Node* n, const String& name) const
		{
			Node* nn;
			while(n)
			{
				if(BaseNode<T, ops>::hash(n->value, true) == name.hash()) return n;
				if(n->fchild) { if((nn = findChild(n->fchild, name))) return nn; }
				n = n->next;
			}
			return nullptr;
		}
		// корень
		Node* get_root() const { return root; }
	protected:
		void reset(Node* n)
		{
			while(n)
			{
				if(n->fchild) reset(n->fchild);
				BaseNode<T, ops>::release(n->value);
				auto nn(n->next); delete n;
				n = nn;
			}
		}
		void remove(Node* n, bool is_del)
		{
			Node* p(n->parent), *nn(n->next), *pn(n->prev);
			n->next = n->prev = nullptr;
			if(p) {p->count--; if(n == p->fchild) p->fchild = nn; if(n == p->lchild) p->lchild = pn;}
			if(is_del) { BaseNode<T, ops>::release(n->value); delete n; }
			if(nn) nn->prev = pn;
			if(pn) pn->next = nn;
			if(n == root) root = nullptr;
		}
		Node* add(Node* p, Node* n)
		{
			if(p)
			{
				if(p->fchild) {p->lchild->next = n; n->prev = p->lchild;} else p->fchild = n;
				p->count++;
				p->lchild = n;
				n->next = nullptr;
			} else root = n;
			return n;
		}
		Node* insert(Node* p, Node* n)
		{
			is_root(p);
			if(p)
			{
				if(p->fchild) {p->fchild->prev = n; n->next = p->fchild;} else p->lchild = n;
				p->count++;
				p->fchild = n;
				n->prev = nullptr;
			}
			else if(!root) root = n;
			return n;
		}
		// корень
		Node* root;
	};
}
