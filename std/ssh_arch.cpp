
#include "stdafx.h"
#include "ssh_arch.h"
#include "ssh_zip.h"
#include "ssh_xml.h"

namespace ssh
{
	void Resource::open(ssh_wcs path)
	{
		try
		{
			SSH_TRACE;
			// ������������� ����������� ������
			Section cs;
			// ��� ��������
			if(!path)
			{
				// �� ������
				archive->get(this);
			}
			else
			{
				// �� �����
				File f(path, File::open_read);
				make(f.read<ssh_cs>());
			}
		}
		catch(const Exception& e) { e.add(L""); }
	}

	void Archive::make(const String& path, const String& sign, const String& xml_list)
	{
		try
		{
			SSH_TRACE;
			// 1. ������� �����
			file.open(path, File::create_read_write);
			// 2. ������� ���������
			caption.version = SSH_SDK_VERSION;
			caption.hash_signature = sign.hash();
			caption.hash_name = hlp->file_name(path).hash();
			caption.tm_create = Time::current();
			// 3. �������� ���������
			file.write(&caption, sizeof(ARCHIVE));
			// 4. �������� ������� �� xml
			if(!xml_list.is_empty())
			{
				Xml xml(xml_list);
				HXML hroot(xml.node(xml.root(), L"archive")), helem;
				String dstMain(hlp->slash_path(xml.attr<String>(hroot, L"dst", L"")));
				ssh_l index(0);
				while((helem = xml.node(hroot, nullptr, index++)))
				{
					String fmtPath, fmtName;
					try
					{
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
					catch(const Exception& e) { e.add(L"������ ��� ���������� ������� <%s, ����: %s> � �����.", fmtName, fmtPath); }
				}
			}
		}
		catch(const Exception& e) { e.add(L"�������� ������ <%s>.", path); }
	}

	void Archive::open(const String& path, const String& sign)
	{
		try
		{
			SSH_TRACE;
			close();
			file.open(path, File::open_read_write);
			// 1. ������ ���������
			file.read(&caption, sizeof(caption));
			// 3. ������ ���������
			// 3.1. ������
			if(caption.version != SSH_SDK_VERSION) SSH_THROW(L"�� ���������� ������ SDK <%i>!", caption.version);
			// 3.2. ��� ������
			if(hlp->file_name(path).hash() != caption.hash_name) SSH_THROW(L"�� ���������� ��� ������!");
			// 3.3. ���������
			if(ssh_hash(sign) != caption.hash_signature) SSH_THROW(L"�� ���������� ��������� <%s>!", sign);
			// 4. ������ ����������
			ssh_u position(file.get_pos());
			while(true)
			{
				RESOURCE res;
				try
				{
					// ������ ��������� �������
					file.set_pos(position, File::begin);
					file.read(&res, sizeof(RESOURCE));
					res.position = position;
					// �������� � ������
					resources.add(res);
					// ��������� �������
					position += res.length_resource;
				}
				catch(const Exception&) { break; }
			}
		}
		catch(const Exception& e) { e.add(L""); }
	}

	List<Archive::RESOURCE, SSH_TYPE>::Node* Archive::find(const String& name) const
	{
		auto n(resources.root());
		while(n && n->value.hash_name != name.hash()) n = n->next;
		return n;
	}

	String Archive::enumerate(bool is_begin)
	{
		SSH_TRACE;
		static List<RESOURCE, SSH_TYPE>::Node* n(nullptr);
		String result;
		if(is_begin) n = resources.root();
		if(n)
		{
			file.set_pos((n->value.position + n->value.length_caption) - n->value.length_name, File::begin);
			result = file.read(cp_utf, n->value.length_name);
			n = n->next;
		}
		return result;
	}

	Buffer<ssh_cs> Archive::get(const String& name)
	{
		SSH_TRACE;
		auto n(find(name));
		if(!n) SSH_THROW(L"�� ��������� ������ <%s> � ������!", name);
		// ��������� � �������������
		auto r(&n->value);
		Zip zip;
		return zip.decompress((file.read<ssh_cs>(r->length_body, r->position + r->length_caption, File::begin)));
	}

	void Archive::get(Resource* res)
	{
		SSH_TRACE;
		auto n(find(res->name()));
		if(!n) SSH_THROW(L"�� ��������� ������ <%s> � ������!", res->name());
		// ���������, ������������� � ���������
		auto r(&n->value);
		Zip zip;
		return res->make(zip.decompress((file.read<ssh_cs>(r->length_body, r->position + r->length_caption, File::begin))));
	}

	void Archive::rename(const String& old_name, const String& new_name)
	{
		SSH_TRACE;

		auto n(find(old_name));
		if(!n)
		{
			SSH_LOG(L"�� ������� ����� ������ <%s>.", old_name);
			return;
		}
		if(!find(new_name))
		{
			try
			{
				auto res(&n->value);
				// 1. ��������� ������
				// ��������� ���� �������
				Buffer<ssh_b> tmp(file.read<ssh_b>(res->length_body, res->position + res->length_caption, File::begin));
				// 2. ������� ������
				remove(old_name);
				// 3. ������� ��� �� �����
				RESOURCE r{file.length(), new_name.length() * 2, tmp.size(), r.length_name + sizeof(RESOURCE), r.length_body + r.length_caption, new_name.hash(), Time::current()};
				// �������� � ������
				resources.add(r);
				// �������� � �����
				file.set_pos(r.position, File::begin);
				file.write(&r, sizeof(RESOURCE));
				file.write(new_name, 0);
				file.write(tmp, tmp.size());
			}
			catch(const Exception& e) { e.add(L"�������������� ������� �� <%s> � <%s>.", old_name, new_name); }
		}
	}

	void Archive::add(const String& path, const String& name)
	{
		try
		{
			SSH_TRACE;
			Time tm;
			RESOURCE* r;
			// ��������� ����
			File f(path, File::open_read);
			// ���������� ���� �����
			try { f.get_time(nullptr, nullptr, &tm); } catch(const Exception&) { tm = Time::current(); }
			// ������ ��� � ������������
			Zip zip;
			Buffer<ssh_cs> buf(zip.compress(f.read<ssh_cs>()));
			// ���� � ������ ��������
			auto n(find(name));
			if(n)
			{
				r = &n->value;
				// ���� ���� � ������ ��������� ������� � ������ ���������, �� ������ �����
				if(tm == r->tm_create && r->length_body == buf.count()) return;
				// ���� ������ ������ � ������� �������� - ������� ������
				if(r->length_body != buf.count()) { remove(name); r = nullptr; }
				r->tm_create = tm;
			}
			// ������� ���������
			if(!n)
			{
				RESOURCE res{file.length(), name.length() * 2, buf.size(), res.length_name + sizeof(RESOURCE), res.length_body + res.length_caption, name.hash(), tm};
				r = &resources.add(res)->value;
			}
			// �������� � �����
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
			const ssh_u block(1024 * 64);
			// 1. ���� ������ � ������
			auto n(find(name));
			if(!n)
			{
				SSH_LOG(L"�� ������� ������� ������ <%s>.", name);
				return;
			}
			// ������������� ��������� ����������
			auto r(&n->value);
			ssh_u positionW(r->position), lenRes(r->length_resource), positionR(positionW + lenRes), lenW(file.length() - positionR);
			// ��������� �����
			Buffer<ssh_b> ptmp(block);
			// 2. ������������ �� ������
			for(ssh_u i = 0; i < lenW / block; i++)
			{
				file.set_pos(positionR, File::begin); file.read(ptmp, block);
				file.set_pos(positionW, File::begin); file.write(ptmp, block);
				positionR += block;
				positionW += block;
			}
			// �������
			file.set_pos(positionR, File::begin); file.read(ptmp, lenW % block);
			file.set_pos(positionW, File::begin); file.write(ptmp, lenW % block);
			// 3. ������������ ������� �������� �������� �� �������� �������� �������
			n = resources.remove(n);
			do { n->value.position -= lenRes; } while((n = n->next));
			// 4. ������������� ����� ������ ������
			file.set_length(file.length() - lenRes);
		}
		catch(const Exception& e) { e.add(L"�������� ������� <%s> �� ������ <%s>.", name, path()); }
	}
}
