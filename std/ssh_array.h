
#pragma once

namespace ssh
{
	template <typename T, int ops = SSH_PTR> class Array
	{
	public:
		// конструктор по умолчанию
		Array() : ID(-1), grow(256), data(nullptr) { reset(); }
		// конструктор по значениям
		Array(ssh_u _ID, ssh_u _count, ssh_u _grow) : ID(_ID), grow(_grow), data(nullptr)  { reset(); alloc(_count); }
		// конструктор копии
		Array(ssh_u _ID, const Array<T, ops>& src) : ID(_ID), data(nullptr) { *this = src; }
		// конструктор переноса
		Array(Array<T, ops>&& src) { ID = src.ID; data = src.data; count = src.count; max_count = src.max_count; grow = src.grow; src.data = nullptr; }
		// деструктор
		~Array() {reset();}
		// установить идентификатор
		void setID(ssh_u _ID) { ID = _ID; }
		// очистить
		void clear() { count = max_count = 0; data = nullptr; }
		// сброс
		void reset() { remove(0, count); delete data; clear(); }
		// добавить элемент
		const Array& operator += (const T& elem) { return insert(count, elem); }
		// добавить массив
		const Array& operator += (const Array<T, ops>& src) {return insert(count, src);}
		// заместить массив
		const Array& operator = (const Array<T, ops>& src) { reset(); grow = src.grow; return insert(0, src); }
		// оператор индекса
		const T& operator [] (ssh_u idx) const { return get(idx); }
		// оператор переноса
		const Array& operator = (Array<T, ops>&& src) { reset(); ID = src.ID; data = src.data; count = src.count; max_count = src.max_count; grow = src.grow; src.data = nullptr; return *this; }
		// установка элемента по индексу
		void set(ssh_u idx, const T& elem)
		{
			if(idx < count)
			{
				BaseNode<T, ops>::release(data[idx]);
				data[idx] = elem;
			}
		}
		// вставка элемента по индексу
		const Array& insert(ssh_u idx, const T& elem)
		{
			if(idx <= count)
			{
				alloc(1);
				memmove(data + idx + 1, data + idx, (count - idx) * sizeof(T));
				data[idx] = elem;
				count++;
			}
			return *this;
		}
		// вставка массива по индексу
		const Array& insert(ssh_u idx, const Array<T, ops>& _array)
		{
			if(idx <= count)
			{
				// сначало расширяем массив на количество элементов вставляемого массива
				ssh_u _count(_array.size());
				if((max_count - count) < _count) alloc(_count);
				memmove(data + idx + _count, data + idx, (count - idx) * sizeof(T));
				// теперь устанавливаем все элементы из этого массива
				for(ssh_u i = 0; i < _count; i++) data[i + idx] = _array[i];
				count += _count;
			}
			return *this;
		}
		// удаление элемента
		void remove(ssh_u idx, ssh_u _count)
		{
			if(idx < count)
			{
				if((idx + _count) > count) _count = (count - idx);
				for(ssh_u i = 0; i < _count; i++) BaseNode<T, ops>::release(data[i + idx]);
				ssh_u ll(idx + _count);
				SSH_MEMCPY(data + idx, data + ll, (count - ll) * sizeof(T));
				count -= _count;
			}
		}
		// найти элемент
		ssh_l find(const T& t) const
		{
			for(ssh_u i = 0 ; i < count ; i++)
			{
				if(get(i) == t) return i;
			}
			return -1;
		}
		// вернуть размер
		ssh_u size() const { return count; }
		// вернуть по индексу
		const T& get(ssh_u idx) const
		{
			if(idx < count) return data[idx];
			SSH_THROW(L"Индекс (%i) за пределами (%i) массива (%i)!", idx, count, ID);
		}
		// вернуть указатель на данные
		const T* getData() const {return (const T*)data;}
		T* getData() {return (T*)data;}
	protected:
		// выделение памяти
		void alloc(ssh_u _grow)
		{
			if((count + _grow) >= max_count)
			{
				max_count += (_grow + grow);
				// выделяем блок
				T* ptr((T*)new ssh_b[max_count * sizeof(T)]);
				// копируем старые данные, если есть
				if(data)
				{
					SSH_MEMCPY(ptr, data, count * sizeof(T));
					delete data;
				}
				// инициализируем новые
				for(ssh_u i = count; i < max_count; i++) ::new((void*)(ptr + i)) T();
				data = ptr;
			}
		}
		// данные
		T* data;
		// количество элементов
		ssh_u count;
		// выделено элементов
		ssh_u max_count;
		// приращение
		ssh_u grow;
		// идентификатор массива
		ssh_u ID;
	};
}
