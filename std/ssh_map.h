
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
			// ������������
			Node() : next(nullptr) {}
			Node(const KEY& k, const TYPE& t, Node* n) : next(n), key(k), value(t) {}
			// �����
			KEY key;
			// ��������
			TYPE value;
			// ��������� ����
			Node* next;
		};
	protected:
		class Cursor
		{
		public:
			// ���������� ������
			Cursor(Map<TYPE, KEY, ops_val, ops_key>* arr, const KEY& k) : node(new Node(k, TYPE(), arr->cells)) {arr->cells = node;}
			// �������
			Cursor(Node* n) : node(n){}
			Cursor& operator = (const TYPE& value) {BaseNode<TYPE, ops_val>::release(node->value); node->value = value; return *this;}
			operator const TYPE() const {return node->value;}
			TYPE operator->() const {return node->value;}
		protected:
			// ����
			Node* node;
		};
	public:
		// ����������� �� ���������
		Map() {clear();}
		// ����������� �����
		Map(const Map<TYPE, KEY, ops_val, ops_key>& src) { clear(); *this = src; }
		// ����������� ��������
		Map(Map<TYPE, KEY, ops_val, ops_key>&& src) { cells = src.cells; node = src.node; src.cells = src.node = nullptr; }
		// ����������
		~Map()
		{
			reset(true);
			Node::get_MemArrayNode()->Reset();
		}
		// ��������
		void clear() { cells = node = nullptr; }
		// ������������
		const Map& operator = (const Map<TYPE, KEY, ops_val, ops_key>& src) { reset(true); return *this += src; }
		const Map& operator = (Map<TYPE, KEY, ops_val, ops_key>&& src) { reset(true); cells = src.cells; node = src.node; src.cells = src.node = nullptr; return *this; }
		// ����������
		const Map& operator += (const Map<TYPE, KEY, ops_val, ops_key>& src)
		{
			auto n(src.root());
			while(n = src.next()) {operator[](n->key) = n->value;}
			return *this;
		}
		// ���������� ���������
		ssh_u count() const
		{
			ssh_u c(0);
			Node* n(cells);
			while(n) {n = n->next; c++;}
			return c;
		}
		// ���������/�������
		Cursor operator[](const KEY& key)
		{
			Node* n(cells);
			while(n) {if(n->key == key) return Cursor(n); n = n->next;}
			return Cursor(this, key);
		}
		// ������� ��� �����
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
		// ������� ����� �� ��������
		KEY get_key(const TYPE& value) const
		{
			Node* n(cells);
			while(n) {if(n->value == value) return n->key; n = n->next;}
			return BaseNode<KEY, ops_key>::dummy();
		}
		// �������� - ����� ���� ����������?
		bool is_key(const KEY& key) const
		{
			Node* n(cells);
			while(n) {if(n->key == key) return true; n = n->next;}
			return false;
		}
		// �������� ��������
		void remove(const KEY& key)
		{
			Node* n, *p;
			if((n = get_key(key, &p)))
			{
				// �������
				Node* nn(n->next); n->next = nullptr;
				cells == n ? cells = nn : p->next = nn;
				BaseNode<TYPE, ops_val>::release(n->value);
				BaseNode<KEY, ops_key>::release(n->key);
				delete n;
			}
		}
		// ������� ���������
		Node* value(Node* n) const { return (n ? n->next : cells); }
		// ������� ������
		Node* getCells() {return cells;}
		// ������� ������� ����� 
		const KEY& key() const {return (node ? node->key : cells->key);}
		// ������� ������
		Node* root() const { return (node = nullptr); }
		// ���������� ��������� �������
		Node* next() const { return (node = node ? node->next : cells); }
	protected:
		// �������� �����
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
		// ������� ���� �� �����
		Node* get_key(const KEY& key, Node** p) const
		{
			Node* n(cells);
			while(n) {if(n->key == key) return n; if(p) *p = n; n = n->next;}
			return nullptr;
		}
		// �������� �������
		Node* cells;
		// ������� �������
		mutable Node* node;
	};
}
