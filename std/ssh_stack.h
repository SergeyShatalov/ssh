
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
			// конструктор узла
			Node() : next(nullptr) {}
			// конструктор узла
			Node(const T& t, Node* n) : next(n), value(t) {}
			// следующий узел
			Node* next;
			// значение
			T value;
		};
		// конструктор
		Stack() : ID(-1), root(nullptr) {}
		// конструктор переноса
		Stack(Stack&& src) { ID = src.ID; root = src.root; src.root = nullptr; }
		// деструктор
		~Stack()
		{
			reset();
			Node::get_MemArrayNode()->Reset();
		}
		// оператор переноса
		const Stack& operator = (Stack&& src) { reset(); ID = src.ID; root = src.root; src.root = nullptr; return *this; }
		// добавление элемента
		const Stack& operator += (const T& t) { push(t); return *this; }
		// установка идентификатора
		void setID(ssh_u _ID) { ID = _ID; }
		// добавление элемента
		void push(const T& t) {root = new Node(t, root);}
		// извлечение
		T pop()
		{
			if(!root) SSH_THROW(L"Ошибка. Стек пуст!");
			T t(root->value);
			auto n(root->next);
			delete root;
			root = n;
			return t;
		}
		// освобождение стэка
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
		// проверка на наличие элементов
		bool is_empty() const {return (root == nullptr);}
	protected:
		// вершина
		Node* root;
		// идентификатор
		ssh_u ID;
	};
}
