
#pragma once

#include "ssh_list.h"

typedef void(__cdecl * _ssh_signal)(int);

namespace ssh
{
	template <typename T, ssh_u N> class MemArray
	{
	public:
		struct Block
		{
			union
			{
				Block* next;
				ssh_b t[sizeof(T)];
			};
		};

		struct BlockFix
		{
			BlockFix() : next(nullptr) {}
			~BlockFix() { SSH_DEL(next); }
			BlockFix* next;
			Block arr[N];
		};
		MemArray() : free(nullptr), arrs(nullptr), count(0) {}
		~MemArray() { Reset(); }
		
		bool Valid() const
		{
			return (arrs != nullptr);
		}

		void Reset()
		{
			if(!count)
			{
				SSH_DEL(arrs);
				free = nullptr;
			}
		}
		T* Alloc()
		{
			if(!free)
			{
				BlockFix* tmp(new BlockFix);
				tmp->next = arrs; arrs = tmp;
				for(ssh_u i = 0; i < N; i++)
				{
					arrs->arr[i].next = free;
					free = &(arrs->arr[i]);
				}
			}
			count++;
			Block* b(free);
			free = free->next;
			::new(((T*)b->t)) T();
			return (T*)(b->t);
		}
		void Free(T* t)
		{
			Block* b((Block*)t);
			t->~T();
			b->next = free;
			free = b;
			count--;
		}
		ssh_u count;
		Block* free;
		BlockFix* arrs;
	};

	template <typename TYPE, ssh_u ops> class List;

	class SSH MemMgr
	{
	public:
		struct NodeMem
		{
			SSH_NEW_DECL(NodeMem, 256);
			NodeMem() : sz(0), p(nullptr) {}
			NodeMem(ssh_u _sz, ssh_b* _p, NodeMem* np, NodeMem* nn) : next(nn), prev(np), sz(_sz), p(_p) {}
			~NodeMem() { SSH_DEL(next); }
			// адрес блока
			ssh_b* p;
			// размер блока
			ssh_u sz;
			NodeMem* next;
			NodeMem* prev;
		};
		static MemMgr* instance()
		{
			static MemMgr mem;
			return &mem;
		}
		// вернуть дамп всей используемой памяти
		bool fault(int type, ssh_wcs fn, ssh_wcs fl, int ln, EXCEPTION_POINTERS* except = nullptr, ssh_wcs msg_ex = nullptr);
		// вернуть информацию об сеансе работы менеджера
		void output();
		// выделение памяти
		void* alloc(ssh_u sz);
		// освобождение
		void free(ssh_b* p);
		// вернуть признак блокировки
		bool is_started() const { return !is_disabled; }
		// установить признак блокировки
		void start() { is_disabled = false; }
		void stop() { is_disabled = true; }
	protected:
		// конструктор
		MemMgr() : root(nullptr), max_alloc(0), total_alloc(0), count_alloc(0), total_free(0), size_mem(0), size_max_mem(0), is_disabled(true) {}
		~MemMgr() { clear(); }
		void clear()
		{
			auto m(NodeMem::get_MemArrayNodeMem());
			if(m->Valid())
			{
				SSH_DEL(root);
				m->Reset();
			}
		}

		// вернуть массив не освобожденных узлов
		void leaks();
		int dump(ssh_ccs path, EXCEPTION_POINTERS* except);
		// максиму выделенных
		ssh_u max_alloc;
		// выделено блоков
		ssh_u count_alloc;
		// всего было выделено
		ssh_u total_alloc;
		// всего было освобождено
		ssh_u total_free;
		// максимум единовременно выделенной памяти
		ssh_u size_mem, size_max_mem;
		// признак блокировки
		bool is_disabled;
		// список узлов
		NodeMem* root;
	};
}

#ifdef SSH_MEM
	inline void* operator new(ssh_u sz){return ssh::MemMgr::instance()->alloc(sz); }
	inline void operator delete(void* p) { ssh::MemMgr::instance()->free((ssh_b*)p); }
	inline void* operator new[](ssh_u sz){return ssh::MemMgr::instance()->alloc(sz); }
	inline void operator delete[](void* p) { ssh::MemMgr::instance()->free((ssh_b*)p); }
#endif
