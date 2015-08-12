
/*
*	�����:		������� �. �.
*	�������:	�����������, 19 ���� 2015, 8:00
*	�����������:--
*	��������:	����� ���������� �������
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
		// �������
		virtual void open(ssh_wcs path);
		// ���������
		virtual void save(ssh_wcs path, bool is_xml) = 0;
	protected:
		// ������������ �� ������
		virtual void make(const Buffer<ssh_cs>& buf) = 0;
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
		void close() { resources.reset(); file.close(); }
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
		Buffer<ssh_cs> get(const String& name);
		// ������ ��� ���������
	protected:
		// ����������� �� ���������
		Archive() {}
		// ����������
		virtual ~Archive() { close(); }
		// ���������� ������ �� ������(�� ��� �����)
		void get(Resource* res);
		// ����� �������
		List<RESOURCE, SSH_TYPE>::Node* find(const String& name) const;
		// ���� �������
		File file;
		// ������ ��������
		List<RESOURCE, SSH_TYPE> resources{ID_RESOURCES_ARCHIVE};
		// ��������� ������
		ARCHIVE caption;
		// ������ ���������
		static ssh_u const singl_idx = SSH_SINGL_ARCHIVE;
	};

	#define archive		Singlton<Archive>::Instance()
}
