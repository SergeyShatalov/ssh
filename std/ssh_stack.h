
#pragma once

#include "ssh_mem.h"

namespace ssh
{
	template <typename T, ssh_u ops = SSH_PTR> class Stack
	{
		Stack(const Stack& src) {}
		const Stack& operator = (const Stack& src) { return *this; }
	public:
		struct Node
		{
			SSH_NEW_DECL(Node, 128);
			// ����������� ����
			Node() : next(nullptr) {}
			// ����������� ����
			Node(const T& t, Node* n) : next(n), value(t) {}
			// ��������� ����
			Node* next;
			// ��������
			T value;
		};
		// �����������
		Stack() : ID(-1), root(nullptr) {}
		// ����������� ��������
		Stack(Stack&& src) { ID = src.ID; root = src.root; src.root = nullptr; }
		// ����������
		~Stack()
		{
			reset();
			Node::get_MemArrayNode()->Reset();
		}
		// �������� ��������
		const Stack& operator = (Stack&& src) { reset(); ID = src.ID; root = src.root; src.root = nullptr; return *this; }
		// ���������� ��������
		const Stack& operator += (const T& t) { push(t); return *this; }
		// ��������� ��������������
		void setID(ssh_u _ID) { ID = _ID; }
		// ���������� ��������
		void push(const T& t) {root = new Node(t, root);}
		// ����������
		T pop()
		{
			if(!root) SSH_THROW(L"������. ���� ����!");
			T t(root->value);
			auto n(root->next);
			delete root;
			root = n;
			return t;
		}
		// ������������ �����
		void reset()
		{
			auto m(Node::get_MemArrayNode());
			if(m->Valid())
			{
				while(root)
				{
					BaseNode<T, ops>::release(root->value);
					auto n(root->next);
					delete root;
					root = n;
				}
			}
		}
		// �������� �� ������� ���������
		bool is_empty() const {return (root == nullptr);}
	protected:
		// �������
		Node* root;
		// �������������
		ssh_u ID;
	};
}
