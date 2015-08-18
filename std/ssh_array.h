
#pragma once

namespace ssh
{
	template <typename T, int ops = SSH_PTR> class Array
	{
	public:
		// ����������� �� ���������
		Array() : ID(-1), grow(256) { clear(); }
		// ����������� �� ���������
		Array(ssh_u _ID, ssh_u _count, ssh_u _grow) : ID(_ID), grow(_grow) { clear(); set_size(_count); }
		// ����������� �����
		Array(ssh_u _ID, const Array<T, ops>& src) : ID(_ID), data(nullptr) { *this = src; }
		// ����������� ��������
		Array(Array<T, ops>&& src)
		{
			ID = src.ID;
			data = src.data;
			count = src.count;
			max_count = src.max_count;
			grow = src.grow;
			src.clear();
		}
		// ����������
		~Array() {reset();}
		// ���������� �������������
		void setID(ssh_u _ID) { ID = _ID; }
		// ��������
		void clear() { count = max_count = 0; data = nullptr; }
		// �����
		void reset() { remove(0, count); SSH_DEL(data); clear(); }
		// �������� �������
		const Array& operator += (const T& elem) { return insert(count, elem); }
		// �������� ������
		const Array& operator += (const Array<T, ops>& src) {return insert(count, src);}
		// ��������� ������
		const Array& operator = (const Array<T, ops>& src) { reset(); grow = src.grow; return insert(0, src); }
		// �������� �������
		T& operator [] (ssh_u idx) const { return get(idx); }
		// �������� ��������
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
		// ��������� �������� �� �������
		void set(ssh_u idx, const T& elem)
		{
			if(idx < count)
			{
				BaseNode<T, ops>::release(data[idx]);
				data[idx] = elem;
			}
		}
		// ������� �������� �� �������
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
		// ������� ������� �� �������
		const Array& insert(ssh_u idx, const Array<T, ops>& _array)
		{
			if(idx <= count)
			{
				// ������� ��������� ������ �� ���������� ��������� ������������ �������
				ssh_u _count(_array.size());
				if((max_count - count) < _count) alloc(_count);
				memmove(data + idx + _count, data + idx, (count - idx) * sizeof(T));
				// ������ ������������� ��� �������� �� ����� �������
				for(ssh_u i = 0; i < _count; i++) data[i + idx] = _array[i];
				count += _count;
			}
			return *this;
		}
		// �������� ��������
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
		// ����� �������
		ssh_l find(const T& t) const
		{
			for(ssh_u i = 0 ; i < count ; i++)
			{
				if(get(i) == t) return i;
			}
			return -1;
		}
		// ���������� ������
		void set_size(ssh_u size) { reset(); alloc(size); count = size; }
		// ������� ������
		ssh_u size() const { return count; }
		// ������� �� �������
		T& get(ssh_u idx) const
		{
			if(idx < count) return data[idx];
			SSH_THROW(L"������ (%i) �� ��������� (%i) ������� (%i)!", idx, count, ID);
		}
		// ������� ��������� �� ������
		const T* getData() const {return (const T*)data;}
		T* getData() {return (T*)data;}
	protected:
		// ��������� ������
		void alloc(ssh_u _grow)
		{
			if((count + _grow) >= max_count)
			{
				max_count += (_grow + grow);
				// �������� ����
				T* ptr((T*)new ssh_b[max_count * sizeof(T)]);
				// �������� ������ ������, ���� ����
				if(data)
				{
					SSH_MEMCPY(ptr, data, count * sizeof(T));
					delete data;
				}
				// �������������� �����
				for(ssh_u i = count; i < max_count; i++) ::new((void*)(ptr + i)) T();
				data = ptr;
			}
		}
		// ������������� �������
		ssh_u ID;
		// ���������� ���������
		ssh_u count;
		// ����������
		ssh_u grow;
		// ������
		T* data;
		// �������� ���������
		ssh_u max_count;
	};
}
