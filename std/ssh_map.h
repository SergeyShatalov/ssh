
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
			Cursor(Map<TYPE, KEY, ops_val, ops_key>* arr, const KEY& k) : node(new Node(k, TYPE(), arr->cells)) { arr->cells = node; }
			// �������
			Cursor(Node* n) : node(n){}
			Cursor& operator = (const TYPE& value) { BaseNode<TYPE, ops_val>::release(node->value); node->value = value; return *this; }
			operator const TYPE() const { return node->value; }
			TYPE operator->() const { return node->value; }
		protected:
			// ����
			Node* node;
		};
	public:
		// ����������� �� ���������
		Map() : ID(-1) { clear(); }
		Map(ssh_u _ID) : ID(_ID) { clear(); }
		// ����������� �����
		Map(ssh_u _ID, const Map<TYPE, KEY, ops_val, ops_key>& src) : ID(_ID) { clear(); *this = src; }
		// ����������� ��������
		Map(Map<TYPE, KEY, ops_val, ops_key>&& src) { ID = src.ID; cells = src.cells; src.cells = nullptr; }
		// ����������
		~Map()
		{
			reset();
			Node::get_MemArrayNode()->Reset();
		}
		void setID(ssh_u _ID) { ID = _ID; }
		// ��������
		void clear() { cells = nullptr; }
		// ������������
		const Map& operator = (const Map<TYPE, KEY, ops_val, ops_key>& src) { reset(); return *this += src; }
		const Map& operator = (Map<TYPE, KEY, ops_val, ops_key>&& src)
		{
			reset();
			ID = src.ID;
			cells = src.cells;
			src.cells = nullptr;
			return *this;
		}
		// ����������
		const Map& operator += (const Map<TYPE, KEY, ops_val, ops_key>& src)
		{
			auto n(src.root());
			while(n) { operator[](n->key) = n->value; n = n->next; }
			return *this;
		}
		// ���������� ���������
		ssh_u count() const
		{
			ssh_u c(0);
			auto n(cells);
			while(n) { n = n->next; c++; }
			return c;
		}
		// ���������/�������
		Cursor operator[](const KEY& key)
		{
			auto n(cells);
			while(n) { if(n->key == key) return Cursor(n); n = n->next; }
			return Cursor(this, key);
		}
		// ������� ��� �����
		Map<KEY, ssh_u, SSH_TYPE, SSH_TYPE> keys() const
		{
			auto n(cells);
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
			auto n(cells);
			while(n) { if(n->value == value) return n->key; n = n->next; }
			return BaseNode<KEY, ops_key>::dummy();
		}
		// �������� - ����� ���� ����������?
		bool is_key(const KEY& key) const
		{
			auto n(cells);
			while(n) { if(n->key == key) return true; n = n->next; }
			return false;
		}
		// �������� ��������
		void remove(const KEY& key)
		{
			Node* n, *p;
			if((n = get_key(key, &p)))
			{
				// �������
				auto nn(n->next); n->next = nullptr;
				cells == n ? cells = nn : p->next = nn;
				BaseNode<TYPE, ops_val>::release(n->value);
				BaseNode<KEY, ops_key>::release(n->key);
				delete n;
			}
		}
		// ������� ������
		Node* root() const {return cells;}
		// ������� ������
		Node* is_empty() const { return (cells != nullptr); }
	protected:
		// �������� �����
		void reset()
		{
			if(Node::get_MemArrayNode()->Valid())
			{
				while(cells)
				{
					BaseNode<TYPE, ops_val>::release(cells->value);
					BaseNode<KEY, ops_key>::release(cells->key);
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
			auto n(cells);
			while(n) {if(n->key == key) return n; if(p) *p = n; n = n->next;}
			return nullptr;
		}
		// �������������
		ssh_u ID;
		// �������� �������
		Node* cells;
	};
}
