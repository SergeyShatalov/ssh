
#pragma once

namespace ssh
{
	template<typename T> class Buffer
	{
	public:
		// �� ���������
		Buffer() : sz(0), data(nullptr), is_owner(false) {}
		// ����������� �����
		Buffer(const Buffer& buf, ssh_u size = 0) : sz(size ? size : buf.size()), is_owner(true) { const_cast<Buffer<T>*>(&buf)->is_owner = false; data = buf.data; }
		// ����������� ��������
		Buffer(Buffer&& buf) : sz(buf.sz), is_owner(buf.is_owner), data(buf.data) {buf.data = nullptr;}
		// ������� ����� ������������ �������
		Buffer(ssh_u count) : sz(count * sizeof(T)), is_owner(true), data(new T[count]) {}
		Buffer(const Range<int>& wh, int bpp) : sz(wh.w * wh.h * bpp), is_owner(true), data(new T[sz]) {}
		// ������� �� ������������� �������������� ������
		Buffer(T* p, ssh_u count, bool is_own = true) : sz(count * sizeof(T)), is_owner(is_own), data(p) {}
		// ����������
		~Buffer() { release(); }
		// �������� ������������
		const Buffer& operator = (const Buffer& buf) { release(); is_owner = true; sz = buf.sz; const_cast<Buffer<T>*>(&buf)->is_owner = false; data = buf.data; return *this; }
		// �������� ��������
		const Buffer& operator = (Buffer&& buf) { release(); data = buf.data; sz = buf.sz; is_owner = buf.is_owner; buf.data = nullptr; return *this; }
		// ������������ ������
		void free() { release(); }
		// ������� ���������� ���������
		ssh_u count() const { return sz / sizeof(T); }
		// ������� ������
		ssh_u size() const { return sz; }
		// ����������� ��������
		T* operator->() { return data; }
		T* operator->() const { return data; }
		// ���������� ����
		operator T*() const { return data; }
		// ������������� ����������� ������
		template<typename TYPE> TYPE* to() const  { return (TYPE*)data; }
	protected:
		void release() { if(data && is_owner) delete data; data = nullptr; }
		void move(T* p)
		{
			if(!is_owner) data = p;
			else if(p && sz)
			{
				data = new T[sz / sizeof(T)];
				SSH_MEMCPY(data, p, sz);
			}
			else
			{
				sz = 0;
				data = nullptr;
			}
		}
		ssh_u sz;
		bool is_owner;
		T* data;
	};

#define buf_ws	Buffer<ssh_ws> 
#define buf_cs	Buffer<ssh_cs> 
#define buf_b	Buffer<ssh_b> 
}