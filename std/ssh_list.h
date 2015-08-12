
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
			// �����������
			Node(const TYPE& t, Node* p, Node* n) : prev(p), next(n), value(t) {}
			// ���������
			Node* next;
			// ����������
			Node* prev;
			// ��������
			TYPE value;
		};
		// �����������
		List() : ID(-1) { clear(); }
		List(ssh_u _ID) : ID(_ID) { clear(); }
		// ����������� �����
		List(ssh_u _ID, const List<TYPE, ops>& src) : ID(_ID) { clear(); *this = src; }
		// ����������� ��������
		List(List<TYPE, ops>&& src) { ID = src.ID; nroot = src.nroot; nlast = src.nlast; src.clear(); }
		// ����������
		~List()
		{
			reset();
			Node::get_MemArrayNode()->Reset(); 
		}
		// ���������� �������������
		void setID(ssh_u _ID) { ID = _ID; }
		// ��������
		void clear() { nroot = nlast = nullptr; }
		// ����������
		const List& operator += (const List<TYPE, ops>& src) { auto n(src.root()); while(n) { add(n->value); n = n->next; } return *this; }
		const List& operator += (const TYPE& t) { add(t); return *this; }
		// ������������
		const List& operator = (const List<TYPE, ops>& src) {reset(); return (*this += src);}
		const List& operator = (List<TYPE, ops>&& src) { reset(); ID = src.ID; nroot = src.nroot; nlast = src.nlast; src.clear(); return *this; }
		// ����������
		Node* add(const TYPE& t)
		{
			auto n(new Node(t, nlast, nullptr));
			if(nroot) nlast->next = n; else nroot = n;
			return (nlast = n);
		}
		// ������� ��������
		Node* insert(const TYPE& t, Node* n = nullptr)
		{
			if(!n) n = nroot;
			auto np(n ? n->prev : nullptr);
			auto nd(new Node(t, np, n));
			if(np) np->next = nd;
			if(n) n->prev = nd;
			if(n == nroot) nroot = nd;
			if(!nlast) nlast = nd;
			return nd;
		}
		// �������
		Node* remove(Node* nd)
		{
			auto ret(nd);
			if(nd)
			{
				auto n(nd->next);
				auto p(nd->prev);
				if(nd == nroot) nroot = n;
				if(nd == nlast) nlast = p;
				if(n) n->prev = p;
				if(p) p->next = n;
				ret = (n ? n : p);
				BaseNode<TYPE, ops>::release(nd->value);
				delete nd;
			}
			return ret;
		}
		// ������� "������"
		Node* root() const { return nroot; }
		// ������� ���������
		Node* last() const { return nlast; }
		// �����
		Node* find(const TYPE& value) const
		{
			auto n(root());
			while(n && n->value != value) n = n->next;
			return n;
		}
		// ����� �� �����
		Node* find(const String& name, const String& type) const
		{
			auto n(root());
			while(n)
			{
				if(n->value->name() == name && n->value->type() == type) break;
				n = n->next;
			}
			return n;
		}
		bool is_empty() const { return (nroot == nullptr); }
		// �����
		void reset()
		{
			if(Node::get_MemArrayNode()->Valid())
			{
				while(nroot)
				{
					auto nr(nroot);
					BaseNode<TYPE, ops>::release(nroot->value);
					if(nr != nroot) continue;
					auto n(nroot->next);
					SSH_DEL(nroot);
					nroot = n;
				}
			}
			clear();
		}
	protected:
		// �������������
		ssh_u ID;
		// ������
		Node* nroot;
		// ���������
		Node* nlast;
	};
}
