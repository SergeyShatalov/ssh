
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
		List() : ID(-1) {init();}
		List(ssh_u _ID) : ID(_ID) { init(); }
		// ����������� �����
		List(ssh_u _ID, const List<TYPE, ops>& src) : ID(_ID) { init(); *this = src; }
		// ����������� ��������
		List(List<TYPE, ops>&& src) { nroot = src.nroot; nlast = src.nlast; node = src.node; src.init(); }
		// ����������
		~List()
		{
			reset(true);
			Node::get_MemArrayNode()->Reset(); 
		}
		// ���������� �������������
		void setID(ssh_u _ID) { ID = _ID; }
		// ��������
		void clear() { init(); }
		// ����������
		void free() { reset(true); }
		// ����������
		const List& operator += (const List<TYPE, ops>& src) {auto n(src.root()); while(n = src.next()) add(n->value); return *this;}
		const List& operator += (const TYPE& t) { add(t); return *this; }
		// ������������
		const List& operator = (const List<TYPE, ops>& src) {reset(true); return (*this += src);}
		const List& operator = (List<TYPE, ops>&& src) { reset(true); nroot = src.nroot; nlast = src.nlast; node = src.node; src.init(); return *this; }
		// ����������
		Node* add(const TYPE& t)
		{
			node = new Node(t, nlast, nullptr);
			if(nroot) nlast->next = node; else nroot = node;
			return (nlast = node);
		}
		// ������� ��������
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
		// �������
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
		// ������� � "������"
		Node* root() const { return (node = nullptr); }
		// ���������
		Node* next() const { return (node = (node ? node->next : nroot)); }
		// ����������
		Node* prev() const { return (node = (node ? node->prev : nlast)); }
		// �����
		Node* find(const TYPE& value) const { root(); while(next() && node->value != value) {} return node; }
		// ����� �� �����
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
		// �������������
		void init() { nroot = nlast = node = nullptr; }
		// �����
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
		// ������
		Node* nroot;
		// ���������
		Node* nlast;
		// �������������
		ssh_u ID;
		// �������
		mutable Node* node;
	};
}
