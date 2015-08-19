
#pragma once

#include "ssh_array.h"
#include "ssh_map.h"

namespace ssh
{
	class SSH MySql
	{
	public:
		enum ClassType : int
		{
			NUMBER, STRING, REAL, DATETIME, BIN
		};
		enum class TypesTable : int
		{
			BDB, HEAP, ISAM, InnoDB, MERGE, MRG_MYISAM, MYISAM
		};
		enum class TypesField : int
		{
			// длина, длина дробной части, опции
			DOUBLE, FLOAT, DECIMAL,
			// обязательна длина [binary]
			CHAR, VARCHAR,
			// длина, опции
			TINYINT, SMALLINT, MEDIUMINT, BIGINT, INT, BIT,
			// перечисляемые значения
			ENUM, SET,
			// без длины, без дефолта
			TINYBLOB, MEDIUMBLOB, LONGBLOB, BLOB,
			TINYTEXT, MEDIUMTEXT, LONGTEXT, TEXT,
			DATE, TIME, DATETIME, YEAR, TIMESTAMP,
			NONE
		};
		enum OptionsField : int
		{
			_NULL			= 0x00,
			NOT_NULL		= 0x01,
			UNSIGNED		= 0x02,
			ZEROFILL		= 0x04,
			BINARY			= 0x08,
			UNIQUE			= 0x10,
			KEY				= 0x20,
			FULLTEXT		= 0x40,
			AUTO_INCREMENT	= 0x80,
			PRIMARY			= 0x100
		};
		enum class TypesRow
		{
			DEFAULT, DYNAMIC, FIXED, COMPRESSED
		};
		struct FIELD
		{
			ssh_wcs name;
			TypesField type;
			int opt;
			ssh_d length;
			ssh_wcs def;
			ssh_wcs comment;
			ssh_d decimals;
			ssh_d index_length;
			ssh_wcs enums;
		};
		// конструкторы
		MySql() : is_conn(false) {}
		MySql(ssh_wcs host, ssh_wcs user, ssh_wcs pass, ssh_wcs db, ssh_wcs charset = L"cp1251") : MySql() { connect(host, user, pass, db); }
		// деструктор
		virtual ~MySql() { disconnect(); }
		// коннект с сервером
		void connect(ssh_wcs host, ssh_wcs user, ssh_wcs pass, ssh_wcs db, ssh_wcs charset = L"cp1251");
		// дизконнект с сервером
		void disconnect();
		// прямой запрос
		void query(ssh_wcs q) { _query(q); }
		// добвить базу данных
		void add_db(ssh_wcs name) { _query(L"CREATE DATABASE IF NOT EXISTS `%s`", name); }
		// удалить базу данных
		void del_db(ssh_wcs name) { _query(L"DROP DATABASE IF_EXISTS `%s`", name); }
		// выбрать базу данных
		void sel_db(ssh_wcs name) { _query(L"USE `%s`", name); }
		// добавить таблицу
		void add_table(const FIELD flds[], ssh_wcs name, ssh_wcs comment = nullptr, TypesTable type = TypesTable::MYISAM, TypesRow row = TypesRow::DEFAULT);
		// удалить таблицу
		void del_table(ssh_wcs name) { _query(L"DROP TABLE IF EXISTS `%s`", name); }
		// блокировать таблицу
		void lock_table(ssh_wcs name, bool is_lock) { if(is_lock) _query(L"LOCK TABLES `%s` WRITE", name); else _query(L"UNLOCK TABLES"); }
		// очистить таблицу
		ssh_u truncate_table(ssh_wcs name, bool is_locked);
		// переименовать таблицу
		void rename_table(ssh_wcs name, ssh_wcs new_name) { _query(L"RENAME TABLE `%s` TO `%s`", name, new_name); }
		// выбрать
		Array<Map<String, String, SSH_TYPE, SSH_TYPE>, SSH_TYPE> select(ssh_wcs express, ssh_wcs tbls, bool is_duplicate = false, ssh_wcs comp = nullptr, ssh_wcs group = nullptr, ssh_wcs order = nullptr, ssh_u offs = 0, ssh_u rows = 0);
		// обновить запись
		ssh_u update(ssh_wcs tbl, ssh_wcs fields, ssh_wcs values, ssh_wcs comp, bool is_locked = true, ssh_u limit = 0);
		// вставить запись
		ssh_u insert(ssh_wcs tbl, ssh_wcs fields, ssh_wcs values, bool is_locked = true, bool is_replace = false);
		// удалить запись
		ssh_u remove(ssh_wcs tbl, bool is_locked = true, ssh_wcs comp = nullptr, ssh_wcs order = nullptr, ssh_u limit = 0);
		// вернуть идентификатор
		ssh_u get_id() { return mysql_insert_id(&sql); }
		// вернуть сообщение об ошибке
		String error_string() { return String(mysql_error(&sql)); }
		// вернуть результат запроса
		Array<Map<String, String, SSH_TYPE, SSH_TYPE>, SSH_TYPE> get_result();
	protected:
		// для внутреннего вызова
		void _query(ssh_wcs wcs, ...);
		// преобразование типа поля в "мой" формат
		TypesField cnv_type_field(int tp) const;
		// вернуть класс типа
		ClassType get_class_type(TypesField tp) const;
		// формирование опций поля
		String make_opts(int opt, ClassType cls) const;
		// формирование поля
		String make_field(const FIELD* f) const;
		// структура описывающая статус соединения
		MYSQL sql;
		// признак коннекта
		bool is_conn;
		// кодировка
		String charset;
	};
}