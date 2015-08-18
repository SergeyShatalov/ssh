
#pragma once

#include "ssh_array.h"
#include "ssh_map.h"

namespace ssh
{
	class SSH MySql
	{
	public:
		enum class LockTables
		{
			read, write, readwrite
		};
		enum class TypeField
		{
			_number, _real, _string, _binary, _undef, _datetime
		};
		// ������������
		MySql() : is_conn(false) {}
		MySql(ssh_wcs host, ssh_wcs user, ssh_wcs pass, ssh_wcs db) : MySql() { connect(host, user, pass, db); }
		// ����������
		virtual ~MySql() { disconnect(); }
		// ������� � ��������
		void connect(ssh_wcs host, ssh_wcs user, ssh_wcs pass, ssh_wcs db);
		// ���������� � ��������
		void disconnect();
		// ������ ������
		void query(ssh_wcs q) { _query(q); }
		// ������� ���� ������
		void add_db(ssh_wcs name);
		// ������� ���� ������
		void del_db(ssh_wcs name);
		// ������� ���� ������
		void sel_db(ssh_wcs name);
		// �������� �������
		void add_table(ssh_wcs name, ssh_wcs auto_inc, ssh_wcs fields, ssh_wcs types, ssh_wcs defs, ssh_wcs comment = nullptr);
		// ������� �������
		void del_table(ssh_wcs name);
		// ����������� �������
		void lock_table(ssh_wcs name, LockTables lt, bool is);
		// �������
		Array<Map<String, String, SSH_TYPE, SSH_TYPE>, SSH_TYPE> select(ssh_wcs express, ssh_wcs tbls, bool is_duplicate = false, ssh_wcs comp = nullptr, ssh_wcs group = nullptr, ssh_wcs order = nullptr, ssh_u offs = 0, ssh_u rows = 0);
		// �������� ������
		ssh_u update(ssh_wcs tbl, ssh_wcs fields, ssh_wcs values, ssh_wcs comp, ssh_u limit = 0);
		// �������� ������
		ssh_u insert(ssh_wcs tbl, ssh_wcs fields, ssh_wcs values, bool is_replace = false);
		// ������� ������
		ssh_u remove(ssh_wcs tbl, ssh_wcs comp = nullptr, ssh_wcs order = nullptr, ssh_u limit = 0);
		// ������� �������������
		ssh_u get_id() { return mysql_insert_id(&sql); }
		// ������� ��������� �� ������
		String error_string() { return String(mysql_error(&sql)); }
		// ������� ��������� �������
		Array<Map<String, String, SSH_TYPE, SSH_TYPE>, SSH_TYPE> get_result();
	protected:
		// ��� ����������� ������
		void _query(ssh_wcs wcs, ...);
		// �������������� ���� ���� � "���" ������
		TypeField cnv_type_field(int tp) const;
		// ��������� ����������� ������ ����������
		MYSQL sql;
		// ������� ��������
		bool is_conn;
	};
}