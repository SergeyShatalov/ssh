
/*
*	�����:		������� �. �.
*	�������:	�����������, 19 ���� 2015, 8:00
*	�����������:--
*	��������:	����� ���������� �������
*/

#pragma once

#include "ssh_singl.h"
#include "ssh_list.h"

namespace ssh
{
	class Archive;
	class SSH Resource : public Base
	{
		friend class Archive;
	public:
		// ����������� �� ���������
		//Resource() {}
		// ���������
		virtual void save(const String& path, bool is_xml) = 0;
	protected:
		// �������
		virtual void open(const String& path);
		// ��������� �������������
		virtual void init() = 0;
		// �����
		virtual void reset() = 0;
		// ������������ �� ������
		virtual void make(const Buffer<ssh_b>& buf) = 0;
	};

	class SSH Archive final
	{
		friend class Resource;
		friend class Singlton<Archive>;
	public:
		struct ARCHIVE
		{
			// ������
			ssh_u version;
			// ��� ���������
			ssh_u hash_signature;
			// ��� �����
			ssh_u hash_name;
			// ���� ��������
			Time tm_create;
		};
		struct RESOURCE
		{
			// ������� � ������
			ssh_u position;
			// ����� ����� �������
			ssh_u length_name;
			// ����� ���� �������
			ssh_u length_body;
			// ����� ���������
			ssh_u length_caption;
			// ����� ����� �������
			ssh_u length_resource;
			// ��� �����
			ssh_u hash_name;
			// ����� ����������
			Time tm_create;
		};
		// ����������� ������� � ������
		Archive(const String& path, const String& sign) { open(path, sign); }
		// ����������� �������� ������ ������
		Archive(const String& path, const String& sign, const String& xml_list) { make(path, sign, xml_list); }
		// ������� �����
		void open(const String& path, const String& sign);
		// ������� �����
		void make(const String& path, const String& sign, const String& xml_list);
		// ��������
		void close() { resources.free(); file.close(); }
		// ������� �� ������ ������
		void remove(const String& path);
		// �������� ����� ������
		void add(const String& path, const String& name);
		// �������������� �������
		void rename(const String& _old, const String& _new);
		// ������� ���� � ������
		String path() const { return file.get_path(); }
		// ����������� ��� ��������� �������
		String enumerate(bool is_begin);
		// ������� ������������������� ������
		Buffer<ssh_b> get(const String& name);
		// ������ ��� ���������
	protected:
		// ����������� �� ���������
		Archive() { resources.setID(400); }
		// ����������
		virtual ~Archive() { close(); }
		// ���������� ������ �� ������(�� ��� �����)
		void get(Resource* res);
		// ����� �������
		RESOURCE* find(const String& name) const;
		// ���� �������
		File file;
		// ������ ��������
		List<RESOURCE, SSH_TYPE> resources;
		// ��������� ������
		ARCHIVE caption;
		// ������ ���������
		static ssh_u const singl_idx = SSH_SINGL_ARCHIVE;
	};

	#define archive		Singlton<Archive>::Instance()
}
