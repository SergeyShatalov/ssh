
#include "stdafx.h"
#include "ssh_arch.h"
#include "ssh_zip.h"
#include "ssh_xml.h"

namespace ssh
{
	void Resource::open(const String& path)
	{
		try
		{
			SSH_TRACE;
			// устанавливаем критическую секцию
			Section cs;
			// инициализация ресурса
			init();
			// тип создания
			if(path.is_empty())
			{
				// из архива
				archive->get(this);
			}
			else
			{
				// из файла
				File f(path, File::open_read);
				make(f.read<ssh_b>());
			}
		}
		catch(const Exception& e) { e.add(L""); }
	}

	void Archive::make(const String& path, const String& sign, const String& xml_list)
	{
		try
		{
			SSH_TRACE;
			// 1. создать новый
			file.open(path, File::create_read_write);
			// 2. создать заголовок
			caption.version = SSH_SDK_VERSION;
			caption.hash_signature = sign.hash();
			caption.hash_name = hlp->file_name(path).hash();
			caption.tm_create = Time::current();
			// 3. записать заголовок
			file.write(&caption, sizeof(ARCHIVE));
			// 4. добавить ресурсы из xml
			if(!xml_list.is_empty())
			{
				Xml xml(xml_list, L"utf-8");
				HXML hroot(xml.get_node(xml.root(), L"archive")), helem;
				String dstMain(xml.attr<String>(hroot, L"dst", L""));
				dstMain = hlp->slash_path(dstMain);
				ssh_l index(0);
				while((helem = xml.get_node(hroot, nullptr, index++)))
				{
					try
					{
						String fmtPath, fmtName;
						ssh_u count(xml.attr<ssh_u>(helem, L"frames", 1));
						String dst(xml.attr<String>(helem, L"path", L""));
						String name(xml.attr<String>(helem, L"name", L""));
						dst = dstMain + dst;
						if(count == 1)
						{
							add(dst, name + L".0");
						}
						else
						{
							String filePath(hlp->file_path(dst)), extPath(hlp->file_ext(dst, true));
							for(ssh_u i = 0; i < count; i++)
							{
								fmtPath.fmt(count >= 100 ? L"%s%03i%s" : L"%s%02i%s", filePath, i, extPath);
								fmtName.fmt(L"%s.%i", name, i);
								add(fmtPath, fmtName);
							}
						}
					}
					catch(const Exception& e) { e.add(L""); }
				}
			}
		}
		catch(const Exception& e) { e.add(L""); }
	}

	void Archive::open(const String& path, const String& sign)
	{
		try
		{
			SSH_TRACE;
			close();
			file.open(path, File::open_read_write);
			// 1. читаем заголовок
			file.read(&caption, sizeof(caption));
			// 3. анализ заголовка
			// 3.1. версия
			if(caption.version != SSH_SDK_VERSION) SSH_THROW(L"Не допустимая версия SDK <%i>!", caption.version);
			// 3.2. имя архива
			if(hlp->file_name(path).hash() != caption.hash_name) SSH_THROW(L"Не допустимое имя архива!");
			// 3.3. сигнатура
			if(ssh_hash(sign) != caption.hash_signature) SSH_THROW(L"Не допустимая сигнатура <%s>!", sign);
			// 4. читаем содержимое
			ssh_u position(file.get_pos());
			while(true)
			{
				RESOURCE res;
				try
				{
					// читаем заголовок ресурса
					file.set_pos(position, File::begin);
					file.read(&res, sizeof(RESOURCE));
					res.position = position;
					// добавить в список
					resources.add(res);
					// следующая позиция
					position += res.length_resource;
				}
				catch(const Exception&) { break; }
			}
		}
		catch(const Exception& e) { e.add(L""); }
	}

	Archive::RESOURCE* Archive::find(const String& name) const
	{
		auto node(resources.root());
		while((node = resources.next()) && node->value.hash_name != name.hash()) {}
		return (node ? &node->value : nullptr);
	}

	String Archive::enumerate(bool is_begin)
	{
		SSH_TRACE;
		String result;

		if(is_begin) resources.root();
		auto node(resources.next());
		if(node)
		{
			file.set_pos((node->value.position + node->value.length_caption) - node->value.length_name, File::begin);
			result = file.read(cp_utf, node->value.length_name);
		}
		return result;
	}

	Buffer<ssh_b> Archive::get(const String& name)
	{
		SSH_TRACE;
		RESOURCE* r;
		if(!(r = find(name))) SSH_THROW(L"Не обнаружен ресурс <%s> в архиве!", name);
		// загружаем и распаковываем
		Zip zip;
		return zip.decompress((file.read<ssh_b>(r->length_body, r->position + r->length_caption, SEEK_SET)));
	}

	void Archive::get(Resource* res)
	{
		SSH_TRACE;
		RESOURCE* r;
		if(!(r = find(res->name()))) SSH_THROW(L"Не обнаружен ресурс <%s> в архиве!", res->name());
		// загружаем, распаковываем и формируем
		Zip zip;
		return res->make(zip.decompress((file.read<ssh_b>(r->length_body, r->position + r->length_caption, SEEK_SET))));
	}

	void Archive::rename(const String& old_name, const String& new_name)
	{
		SSH_TRACE;
		RESOURCE* res;

		if(!(res = find(old_name))) return;
		if(find(new_name)) return;

		try
		{
			// 1. запомнить старый
			// загрузить тело старого
			Buffer<ssh_b> tmp(file.read<ssh_b>(res->length_body, res->position + res->length_caption, SEEK_SET));
			// 2. удалить старый
			remove(old_name);
			// 3. создать его по новой
			RESOURCE r{file.length(), new_name.length() * 2, tmp.size(), r.length_name + sizeof(RESOURCE), r.length_body + r.length_caption, new_name.hash(), Time::current()};
			// добавить в список
			resources.add(r);
			// записать в архив
			file.set_pos(r.position, File::begin);
			file.write(&r, sizeof(RESOURCE));
			file.write(new_name, 0);
			file.write(tmp, tmp.size());
		}
		catch(const Exception& e) { e.add(L""); }
	}

	void Archive::add(const String& path, const String& name)
	{
		try
		{
			SSH_TRACE;
			Time tm;
			Zip zip;
			// открываем файл
			File f(path, File::open_read);
			// определяем дату файла
			try { f.get_time(nullptr, nullptr, &tm); } catch(const Exception&) { tm = Time::current(); }
			// читаем его и запаковываем
			Buffer<ssh_b> buf(zip.compress(f.read<ssh_b>()));
			// ищем в списке ресурсов
			auto r(find(name));
			if(r)
			{
				// если дата и размер реального ресурса и нового одинакова, то просто выйти
				if(tm == r->tm_create && r->length_body == buf.count()) return;
				// если размер нового и старого различны - стереть старый
				if(r->length_body != buf.count()) { remove(name); r = nullptr; }
				r->tm_create = tm;
			}
			// создаем структуру
			if(!r)
			{
				RESOURCE res{file.length(), name.length() * 2, buf.size(), res.length_name + sizeof(RESOURCE), res.length_body + res.length_caption, name.hash(), tm};
				r = &resources.add(res)->value;
			}
			// записать в архив
			file.set_pos(r->position, File::begin);
			file.write(r, sizeof(RESOURCE));
			file.write(name, 0);
			file.write(buf, buf.size());
		}
		catch(const Exception& e) { e.add(L""); }
	}

	void Archive::remove(const String& name)
	{
		try
		{
			SSH_TRACE;
			RESOURCE* r;
			const ssh_u block(1024 * 64);
			// 1. ищем ресурс в списке
			if(!(r = find(name))) return;
			// устанавливаем временные переменные
			ssh_u positionW(r->position), lenRes(r->length_resource), positionR(positionW + lenRes), lenW(file.length() - positionR);
			// временный буфер
			Buffer<ssh_b> ptmp(block);
			// 2. переписываем по частям
			for(ssh_u i = 0; i < lenW / block; i++)
			{
				file.set_pos(positionR, File::begin); file.read(ptmp, block);
				file.set_pos(positionW, File::begin); file.write(ptmp, block);
				positionR += block;
				positionW += block;
			}
			// остаток
			file.set_pos(positionR, File::begin); file.read(ptmp, lenW % block);
			file.set_pos(positionW, File::begin); file.write(ptmp, lenW % block);
			// 3. корректируем позиции структур ресурсов на величину стертого ресурса
			auto node(resources.remove());
			do { node->value.position -= lenRes; } while(node = resources.next());
			// 4. устанавливаем новый размер архива
			file.set_length(file.length() - lenRes);
		}
		catch(const Exception& e) { e.add(L""); }
	}
}
