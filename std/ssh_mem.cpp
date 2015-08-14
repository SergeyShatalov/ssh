
#include "stdafx.h"
#include "ssh_mem.h"
#include <DbgHelp.h>

namespace ssh
{
	int MemMgr::dump(ssh_ccs path, EXCEPTION_POINTERS* except)
	{
		MINIDUMP_EXCEPTION_INFORMATION eInfo;
		eInfo.ThreadId = GetCurrentThreadId();
		eInfo.ExceptionPointers = except;
		eInfo.ClientPointers = FALSE;
		MINIDUMP_CALLBACK_INFORMATION cbMiniDump;
		cbMiniDump.CallbackRoutine = NULL;
		cbMiniDump.CallbackParam = 0;
//		BOOL bWriteDump = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, except ? &eInfo : NULL, NULL, &cbMiniDump);
		return 0;
	}

	bool MemMgr::fault(int type, ssh_wcs fn, ssh_wcs fl, int ln, EXCEPTION_POINTERS* except, ssh_wcs msg_ex)
	{
		//if(!except)
		{
			CONTEXT ContextRecord;
			EXCEPTION_RECORD ExceptionRecord;
			memset(&ContextRecord, 0, sizeof(CONTEXT));
			memset(&ExceptionRecord, 0, sizeof(EXCEPTION_RECORD));

			RtlCaptureContext(&ContextRecord);

			ExceptionRecord.ExceptionCode = 0;
			ExceptionRecord.ExceptionAddress = _ReturnAddress();

			except = new EXCEPTION_POINTERS;
			except->ContextRecord = new CONTEXT;
			except->ExceptionRecord = new EXCEPTION_RECORD;

			memcpy(except->ContextRecord, &ContextRecord, sizeof(CONTEXT));
			memcpy(except->ExceptionRecord, &ExceptionRecord, sizeof(EXCEPTION_RECORD));
		}
		String msg(L"\r\nКонтекст на момент возбуждения исключения: \r'n");
		String caption;
		switch(type)
		{
			case SIGNAL_INTERRUPT: caption = L"Прерывание. "; break;
			case SIGNAL_INSTRUCTION: caption = L"Недопустимая инструкция. "; break;
			case SIGNAL_FLOATING: caption = L"Недопустимая операция с вещественными числами. "; break;
			case SIGNAL_FAULT: caption = L"Недопустимый указатель. "; break;
			case SIGNAL_TERMINATE: caption = L"SIGNAL TERMINATE. "; break;
			case SIGNAL_ABORT: caption = L"SIGNAL ABORT. "; break;
			case UNHANDLED_EXCEPTION: caption = L"Необработанное исключение. "; break;
			case TERMINATE_CALL: caption = L"Вызов функции terminate(). "; break;
			case UNEXPECTED_CALL: caption = L"Неожиданное исключение. "; break;
			case PURE_CALL: caption = L"Вызов чистой виртуальной функции. "; break;
			case SECURITY_ERROR: caption = L"Переполнение буфера. "; break;
			case NEW_OPERATOR_ERROR: caption = L"Не удалось выделить память оператором new. "; break;
			case INVALID_PARAMETER_ERROR: caption = L"Недопустимый параметр CRT функции, при выполнении операции "; caption += msg_ex; caption += L". "; break;
		}
		if(except)
		{
			msg.fmt(L"\r\nадрес: %016I64X flags: %08X\r\n"
					L"rax: %016I64X rcx: %016I64X rdx: %016I64X rbx: %016I64X rbp: %016I64X rsp: %016I64X rsi: %016I64X rdi: %016I64X\r\n"
					L"r8: %016I64X r9: %016I64X r10: %016I64X r11: %016I64X r12: %016I64X r13: %016I64X r14: %016I64X r15: %016I64X\r\n"
					L"xmm0: %016I64X%016I64X xmm1: %016I64X%016I64X xmm2: %016I64X%016I64X xmm3: %016I64X%016I64X xmm4: %016I64X%016I64X xmm5: %016I64X%016I64X xmm6: %016I64X%016I64X xmm7: %016I64X%016I64X\r\n"
					L"xmm8: %016I64X%016I64X xmm9: %016I64X%016I64X xmm10: %016I64X%016I64X xmm11: %016I64X%016I64X xmm12: %016I64X%016I64X xmm13: %016I64X%016I64X xmm14: %016I64X%016I64X xmm15: %016I64X%016I64X",
					except->ExceptionRecord->ExceptionAddress, except->ContextRecord->EFlags,
					except->ContextRecord->Rax, except->ContextRecord->Rcx, except->ContextRecord->Rdx, except->ContextRecord->Rbx,
					except->ContextRecord->Rbp, except->ContextRecord->Rsp, except->ContextRecord->Rsi, except->ContextRecord->Rdi,
					except->ContextRecord->R8, except->ContextRecord->R9, except->ContextRecord->R10, except->ContextRecord->R11,
					except->ContextRecord->R12, except->ContextRecord->R13, except->ContextRecord->R14, except->ContextRecord->R15,
					except->ContextRecord->Xmm0.Low, except->ContextRecord->Xmm0.High,
					except->ContextRecord->Xmm1.Low, except->ContextRecord->Xmm1.High,
					except->ContextRecord->Xmm2.Low, except->ContextRecord->Xmm2.High,
					except->ContextRecord->Xmm3.Low, except->ContextRecord->Xmm3.High,
					except->ContextRecord->Xmm4.Low, except->ContextRecord->Xmm4.High,
					except->ContextRecord->Xmm5.Low, except->ContextRecord->Xmm5.High,
					except->ContextRecord->Xmm6.Low, except->ContextRecord->Xmm6.High,
					except->ContextRecord->Xmm7.Low, except->ContextRecord->Xmm7.High,
					except->ContextRecord->Xmm8.Low, except->ContextRecord->Xmm8.High,
					except->ContextRecord->Xmm9.Low, except->ContextRecord->Xmm9.High,
					except->ContextRecord->Xmm10.Low, except->ContextRecord->Xmm10.High,
					except->ContextRecord->Xmm11.Low, except->ContextRecord->Xmm11.High,
					except->ContextRecord->Xmm12.Low, except->ContextRecord->Xmm12.High,
					except->ContextRecord->Xmm13.Low, except->ContextRecord->Xmm13.High,
					except->ContextRecord->Xmm14.Low, except->ContextRecord->Xmm14.High,
					except->ContextRecord->Xmm15.Low, except->ContextRecord->Xmm15.High,
					except->ContextRecord->Xmm10.Low, except->ContextRecord->Xmm10.High);
		}
		log->add(Log::mException, fn, fl, ln, caption + msg);
		return true;
	}

	void MemMgr::leaks()
	{
		if(count_alloc)
		{
			Section cs;
			String tmp, txt;
			log->add(tmp.fmt(L"Обнаружено %I64i потерянных блоков памяти...\r\n", count_alloc));
			auto n(root);
			while(n)
			{
				String bytes(ssh_make_hex_string<ssh_b>(n->p + sizeof(NodeMem*), n->sz > 48 ? 48 : n->sz, txt, n->sz > 48));
				log->add(tmp.fmt(L"node <0x%X, %i, <%s>, <%s>", n->p + sizeof(NodeMem*), n->sz, bytes, txt));
				n = n->next;
			}
		}
	}

	void MemMgr::output()
	{
		String tmp;
		leaks();
		log->add(tmp.fmt(L"\r\nЗа данный сеанс было выделено %i(~%s) байт памяти ..., освобождено %i(~%s) ...:%c, максимум - %i блоков, %i(~%s)\r\n",
				 total_alloc, ssh_num_volume(total_alloc), total_free, ssh_num_volume(total_free), (total_alloc != total_free ? L'(' : L')'), max_alloc, size_max_mem, ssh_num_volume(size_max_mem)));
	}

	void* MemMgr::alloc(ssh_u sz)
	{
		Section cs;
		
		ssh_b* p((ssh_b*)::malloc(sz + sizeof(NodeMem*)));
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
			memset(p, 0xaa, nd->sz);
			is_disabled = true;
			count_alloc--;
			total_free += nd->sz;
			size_mem -= nd->sz;
			// удаляем узел из списка
			auto nn(nd->next);
			auto np(nd->prev);
			if(nd == root) root = nn;
			if(nn) nn->prev = np;
			if(np) np->next = nn;
			nd->next = nullptr;
			delete nd;
			is_disabled = false;
		}
		// освобождаем память
		::free(p);
	}
}
