
/*
*	Автор:		Шаталов С. В.
*	Создано:	Владикавказ, 19 июля 2015, 8:00
*	Модификация:--
*	Описание:	Класс управления архивом
*/

#pragma once

#include "ssh_singl.h"
#include "ssh_list.h"
#include "ssh_srlz.h"

namespace ssh
{
	class Archive;
	class SSH Resource : public Base, public Serialize
	{
		friend class Archive;
	public:
		// октрыть
		virtual void open(ssh_wcs path);
		// сохранить
		virtual void save(ssh_wcs path, bool is_xml) = 0;
	protected:
		// сформировать из памяти
		virtual void make(const Buffer<ssh_cs>& buf) = 0;
	};

	class SSH Archive final
	{
		friend class Resource;
		friend class Singlton<Archive>;
	public:
		struct ARCHIVE
		{
			// версия
			ssh_u version;
			// хэш сигнатуры
			ssh_u hash_signature;
			// хэш имени
			ssh_u hash_name;
			// дата создания
			Time tm_create;
		};
		struct RESOURCE
		{
			// позиция в архиве
			ssh_u position;
			// длина имени ресурса
			ssh_u length_name;
			// длина тела ресурса
			ssh_u length_body;
			// длина заголовка
			ssh_u length_caption;
			// длина всего ресурса
			ssh_u length_resource;
			// хэш имени
			ssh_u hash_name;
			// время добавления
			Time tm_create;
		};
		// конструктор доступа к архиву
		Archive(const String& path, const String& sign) { open(path, sign); }
		// конструктор создания нового архива
		Archive(const String& path, const String& sign, const String& xml_list) { make(path, sign, xml_list); }
		// открыть архив
		void open(const String& path, const String& sign);
		// открыть архив
		void make(const String& path, const String& sign, const String& xml_list);
		// закрытие
		void close() { resources.reset(); file.close(); }
		// удалить из архива ресурс
		void remove(const String& path);
		// добавить новый ресурс
		void add(const String& path, const String& name);
		// переименование ресурса
		void rename(const String& _old, const String& _new);
		// вернуть путь к архиву
		String path() const { return file.get_path(); }
		// перечислить все доступные ресурсы
		String enumerate(bool is_begin);
		// вернуть неструктурированные данные
		Buffer<ssh_cs> get(const String& name);
		// индекс для синглтона
	protected:
		// конструктор по умолчанию
		Archive() {}
		// деструктор
		virtual ~Archive() { close(); }
		// возвращает ресурс из архива(по его имени)
		void get(Resource* res);
		// поиск ресурса
		List<RESOURCE, SSH_TYPE>::Node* find(const String& name) const;
		// файл ресурса
		File file;
		// дерево ресурсов
		List<RESOURCE, SSH_TYPE> resources{ID_RESOURCES_ARCHIVE};
		// заголовок архива
		ARCHIVE caption;
		// индекс синглтона
		static ssh_u const singl_idx = SSH_SINGL_ARCHIVE;
	};

	#define archive		Singlton<Archive>::Instance()
}
