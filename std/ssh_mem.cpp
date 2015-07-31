
#include "stdafx.h"
#include "ssh_mem.h"

namespace ssh
{
	void MemMgr::leaks()
	{
		if(count_alloc)
		{
			Section cs;
			String tmp, txt;
			log->add(tmp.fmt(L"ќбнаружено %I64i потер€нных блоков пам€ти...\r\n", count_alloc));
			auto n(root);
			while(n)
			{
				String bytes(hlp->make_hex_string<ssh_b>(n->p + sizeof(NodeMem*), n->sz > 48 ? 48 : n->sz, txt, n->sz > 48));
				log->add(tmp.fmt(L"node <0x%X, %i, <%s>, <%s>", n->p + sizeof(NodeMem*), n->sz, bytes, txt));
				n = n->next;
			}
		}
	}
	
	void MemMgr::fault()
	{
		void* _pp = _pxcptinfoptrs;
//		void** _pp(__pxcptinfoptrs());
		log->add(L"fault!");
	}

	void MemMgr::output()
	{
		String tmp;
		leaks();
		log->add(tmp.fmt(L"\r\n«а данный сеанс было выделено %i(~%s) байт пам€ти ..., освобождено %i(~%s) ...:%c, максимум - %i блоков, %i(~%s)\r\n", total_alloc, hlp->num_volume(total_alloc),
																													  total_free, hlp->num_volume(total_free), (total_alloc != total_free ? L'(' : L')'),
																													  max_alloc, size_max_mem, hlp->num_volume(size_max_mem)));
	}

	void* MemMgr::alloc(ssh_u sz)
	{
		Section cs;
		
		ssh_b* p((ssh_b*)_aligned_malloc(sz + sizeof(NodeMem*), 16));
		if(!is_disabled)
		{
			is_disabled = true;
			// добавить статистику
			count_alloc++;
			max_alloc = count_alloc;
			total_alloc += sz;
			size_mem += sz;
			if(size_max_mem < size_mem) size_max_mem = size_mem;
			NodeMem* nd(new NodeMem(sz, p, nullptr, root));
			if(root) root->prev = nd;
			root = nd;
			*(NodeMem**)p = nd;
			is_disabled = false;
		}
		else *(ssh_u*)p = 0;
		return (void*)(p + sizeof(NodeMem*));
	}
	// освобождение
	void MemMgr::free(ssh_b* p)
	{
		Section cs;
		p -= sizeof(NodeMem*);
		auto nd(*(NodeMem**)p);
		if(nd)
		{
			is_disabled = true;
			count_alloc--;
			total_free += nd->sz;
			size_mem -= nd->sz;
			// удал€ем узел из списка
			auto nn(nd->next);
			auto np(nd->prev);
			if(nd == root) root = nn;
			if(nn) nn->prev = np;
			if(np) np->next = nn;
			nd->next = nullptr;
			delete nd;
			is_disabled = false;
		}
		// освобождаем пам€ть
		_aligned_free(p);
	}
}
