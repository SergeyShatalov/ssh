
#pragma once

namespace ssh
{
	template <typename T, int ops = SSH_PTR> class Array
	{
	public:
		// конструктор по умолчанию
		Array() : ID(-1), grow(256) { clear(); }
		// конструктор по значени€м
		Array(ssh_u _ID, ssh_u _count, ssh_u _grow) : ID(_ID), grow(_grow) { clear(); set_size(_count); }
		// конструктор копии
		Array(ssh_u _ID, const Array<T, ops>& src) : ID(_ID), data(nullptr) { *this = src; }
		// конструктор переноса
		Array(Array<T, ops>&& src)
		{
			ID = src.ID;
			data = src.data;
			count = src.count;
			max_count = src.max_count;
			grow = src.grow;
			src.clear();
		}
		// деструктор
		~Array() {reset();}
		// установить идентификатор
		void setID(ssh_u _ID) { ID = _ID; }
		// очистить
		void clear() { count = max_count = 0; data = nullptr; }
		// сброс
		void reset() { remove(0, count); SSH_DEL(data); clear(); }
		// добавить элемент
		const Array& operator += (const T& elem) { return insert(count, elem); }
		// добавить массив
		const Array& operator += (const Array<T, ops>& src) {return insert(count, src);}
		// заместить массив
		const Array& operator = (const Array<T, ops>& src) { reset(); grow = src.grow; return insert(0, src); }
		// оператор индекса
		T& operator [] (ssh_u idx) const { return get(idx); }
		// оператор переноса
		const Array& operator = (Array<T, ops>&& src)
		{
			reset();
			ID = src.ID;
			data = src.data;
			count = src.count;
			max_count = src.max_count;
			grow = src.grow;
			src.clear();
			return *this;
		}
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
				// сначало расшир€ем массив на количество элементов вставл€емого массива
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
		// установить размер
		void set_size(ssh_u size) { reset(); alloc(size); count = size; }
		// вернуть размер
		ssh_u size() const { return count; }
		// вернуть по индексу
		T& get(ssh_u idx) const
		{
			if(idx < count) return data[idx];
			SSH_THROW(L"»ндекс (%i) за пределами (%i) массива (%i)!", idx, count, ID);
		}
		// вернуть указатель на данные
		const T* getData() const {return (const T*)data;}
		T* getData() {return (T*)data;}
	protected:
		// выделение пам€ти
		void alloc(ssh_u _grow)
		{
			if((count + _grow) >= max_count)
			{
				max_count += (_grow + grow);
				// выдел€ем блок
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
		// идентификатор массива
		ssh_u ID;
		// количество элементов
		ssh_u count;
		// приращение
		ssh_u grow;
		// данные
		T* data;
		// выделено элементов
		ssh_u max_count;
	};
}
