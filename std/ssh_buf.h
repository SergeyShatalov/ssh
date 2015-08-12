
#pragma once

namespace ssh
{
	template<typename T> class Buffer
	{
	public:
		// по умолчанию
		Buffer() : sz(0), data(nullptr), is_owner(false) {}
		// конструктор копии
		Buffer(const Buffer& buf, ssh_u size) : sz(size), is_owner(true) { const_cast<Buffer<T>*>(&buf)->is_owner = false; data = buf.data; }
		// конструктор переноса
		Buffer(Buffer&& buf) : data(buf.data), sz(buf.sz), is_owner(buf.is_owner) {buf.data = nullptr;}
		// создать буфер определённого размера
		Buffer(ssh_u count) : is_owner(true), sz(count * sizeof(T)), data(new T[count]) {}
		// создать из существующего неопределённого буфера
		Buffer(T* p, ssh_u count, bool is_copy) : sz(count * sizeof(T)), is_owner(true), data(p) { if(is_copy) move(p); }
		// деструктор
		~Buffer() { release(); }
		// оператор присваивание
		const Buffer& operator = (const Buffer& buf) { release(); is_owner = true; sz = buf.sz; const_cast<Buffer<T>*>(&buf)->is_owner = false; data = buf.data; return *this; }
		// оператор переноса
		const Buffer& operator = (Buffer&& buf) { release(); data = buf.data; sz = buf.sz; is_owner = buf.is_owner; buf.data = nullptr; return *this; }
		// освобождение буфера
		void free() { release(); }
		// вернуть количество элементов
		ssh_u count() const { return sz / sizeof(T); }
		// вернуть размер
		ssh_u size() const { return sz; }
		// селекторный оператор
		T* operator->() { return data; }
		T* operator->() const { return data; }
		// привидение типа
		operator T*() const { return data; }
		// интерпретация содержимого буфера
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