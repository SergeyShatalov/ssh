
#pragma once

namespace ssh
{
	#define BUFFER_COPY		1
	#define BUFFER_RESET	2

	template<typename T> class Buffer
	{
	public:
		// �� ���������
		Buffer() : sz(0), data(nullptr), is_owner(false) {}
		// ����������� �����
		Buffer(const Buffer& buf, ssh_u ops, ssh_u size) : sz(size)
		{
			if(ops == (BUFFER_COPY | BUFFER_RESET))
			{
				const_cast<Buffer<T>*>(&buf)->is_owner = false;
				is_owner = true;
				data = buf.data;
			}
			else
			{
				is_owner = (ops & BUFFER_COPY);
				move(buf.data);
			}
		}
		// ����������� ��������
		Buffer(Buffer&& buf) : data(buf.data), sz(buf.sz), is_owner(buf.is_owner) {buf.data = nullptr;}
		// ������� ����� ������������ �������
		Buffer(ssh_u count) : is_owner(true), sz(count * sizeof(T)), data(new T[count]) {}
		// ������� �� ������������� �������������� ������
		Buffer(T* p, ssh_u ops, ssh_u count) : sz(count * sizeof(T)), is_owner((ops & BUFFER_COPY)) { move(p); }
		// ������� �� ������
		Buffer(const String& str, ssh_u ops = 0) : sz(str.length() * 2), is_owner((ops & BUFFER_COPY)) { move((T*)(ssh_wcs)str); }
		// ����������
		~Buffer() { release(); }
		// �������� ������������
		const Buffer& operator = (const Buffer& buf) { release(); is_owner = buf.is_owner; sz = buf.sz; move(buf.data); return *this; }
		const Buffer& operator = (const String& str) { release(); is_owner = true; sz = str.length() * 2; move((T*)(ssh_wcs)str); return *this; }
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
		T* data;
		ssh_u sz;
		bool is_owner;
	};

#define buf_ws	Buffer<ssh_ws> 
#define buf_cs	Buffer<ssh_cs> 
#define buf_b	Buffer<ssh_b> 
}