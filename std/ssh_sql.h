
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
		enum class UserRights : ssh_u
		{
			alter = 0x80000000,
			remove = 0x40000000,
			index = 0x20000000,
			insert = 0x10000000,
			select = 0x08000000,
			update = 0x04000000,
			create = 0x02000000,
			drop = 0x01000000,
			grant = 0x00800000,
			references = 0x00400000,
			create_tmp = 0x00200000,
			execute = 0x00100000,
			file = 0x00080000,
			lock = 0x00040000,
			process = 0x00020000,
			reload = 0x00010000,
			shutdown = 0x00008000,
			super = 0x00004000,
			show_db = 0x00002000,
			repl_client = 0x00001000,
			repl_slave = 0x00000800
		};
		enum class SelectExpress
		{
			none, distinct, distinctrow, all
		};
		// конструкторы
		MySql() {}
		MySql(ssh_wcs host, ssh_wcs user, ssh_wcs pass, ssh_wcs db) { connect(host, user, pass, db); }
		// деструктор
		virtual ~MySql() { disconnect(); }
		// коннект с сервером
		void connect(ssh_wcs host, ssh_wcs user, ssh_wcs pass, ssh_wcs db);
		// дизконнект с сервером
		void disconnect();
		// прямой запрос
		void query(ssh_wcs q) { _query(q); }
		// добвить базу данных
		void add_db(ssh_wcs name);
		// удалить базу данных
		void del_db(ssh_wcs name);
		// выбрать базу данных
		void sel_db(ssh_wcs name);
		// добавить таблицу
		void add_table(ssh_wcs name, ssh_wcs fields, ssh_wcs types, ssh_wcs opts, ssh_wcs comment = nullptr);
		// удалить таблицу
		void del_table(ssh_wcs name);
		// блокировать таблицу
		void lock_table(ssh_wcs name, LockTables lt, bool is);
		// добавить пользователя
		void add_user(ssh_wcs name, UserRights ur);
		// удалить пользователя
		void del_user(ssh_wcs name);
		// установить права пользователю
		void rights_user(ssh_wcs name, UserRights ur);
		// выбрать
		void select(ssh_wcs express, ssh_wcs from, ssh_wcs where, SelectExpress = SelectExpress::none, ssh_wcs group = nullptr, ssh_wcs order = nullptr, ssh_u offs = 0, ssh_u rows = 0);
		// обновить запись
		ssh_u update(ssh_wcs tbl, ssh_wcs fields, ssh_wcs values, ssh_wcs where, ssh_u limit = 0);
		// вставить запись
		ssh_u insert(ssh_wcs tbl, ssh_wcs fields, ssh_wcs values, bool is_replace = false);
		// удалить запись
		ssh_u remove(ssh_wcs tbl, ssh_wcs where = nullptr, ssh_wcs order = nullptr, ssh_u limit = 0);
		// вернуть идентификатор
		ssh_u get_id() { return mysql_insert_id(&sql); }
		// вернуть сообщение об ошибке
		String error_string() { return String(mysql_error(&sql)); }
		// вернуть результат запроса
		Array<Map<String, String>, SSH_TYPE> get_result();
	protected:
		// для внутреннего вызова
		void _query(ssh_wcs wcs, ...);
		// структура описывающая статус соденинеия
		MYSQL sql;
		String tmp;
	};
}