
#include "stdafx.h"
#include "ssh_sql.h"

namespace ssh
{
	void MySql::connect(ssh_wcs _host, ssh_wcs user, ssh_wcs pass, ssh_wcs db)
	{
		disconnect();

		if(!mysql_init(&sql)) SSH_THROW(L"Не удалось инициализировать mysql! Ошибка - %s!", error_string());
		if(!mysql_options(&sql, MYSQL_SET_CHARSET_NAME, "cp1251")) SSH_THROW(L"Не удалось установить кодировку в mysql! Ошибка - %s!", error_string());
		String host(_host);
		ssh_l port(0), pos;
		if((pos = host.find(L':')) >= 0)
		{
			port = host.toNum<ssh_l>(pos + 1);
			host = host.left(pos);
		}
		if(!mysql_real_connect(&sql, ssh_cnv(cp_ansi, host, true), ssh_cnv(cp_ansi, user, true), ssh_cnv(cp_ansi, pass, true), ssh_cnv(cp_ansi, db, true), (unsigned int)port, nullptr, 0))
			SSH_THROW(L"Не удалось законнектиться с сервером mysql! Ошибка - %s!", error_string());
	}
	
	void MySql::disconnect()
	{
		mysql_close(&sql);
	}
	
	void MySql::add_db(ssh_wcs name)
	{
		/* CREATE DATABASE [IF NOT EXISTS] db_name */
		_query(L"CREATE DATABASE IF NOT EXISTS %s", name);
	}
	
	void MySql::del_db(ssh_wcs name)
	{
		/* DROP DATABASE [IF EXISTS] db_name */
		_query(L"DROP DATABASE IF_EXISTS %s", name);
	}
	
	void MySql::sel_db(ssh_wcs name)
	{
		/* USE db_name */
		_query(L"USE %s", name);
	}
	
	void MySql::add_table(ssh_wcs name, ssh_wcs fields, ssh_wcs types, ssh_wcs opts, ssh_wcs comment)
	{
		/* CREATE TABLE [IF NOT EXISTS] tbl_name [col_name type [NOT NULL | NULL] [DEFAULT default_value] [AUTO_INCREMENT] [PRIMARY KEY], ...)] [COMMENT = "string"] */

	}
	
	void MySql::del_table(ssh_wcs name)
	{
		/* DROP TABLE [IF EXISTS] tbl_name */
		_query(L"DROP TABLE IF EXISTS %s", name);
	}
	
	void MySql::lock_table(ssh_wcs name, LockTables lt, bool is)
	{
		/*
			LOCK TABLES tbl_name {READ | [READ LOCAL] | [LOW_PRIORITY] WRITE} [, tbl_name {READ | [LOW_PRIORITY] WRITE} ...] ...
			UNLOCK TABLES
		*/
		String rights((lt == LockTables::read ? L"READ" : (lt == LockTables::write ? L"WRITE" : L"READ WRITE")));
		if(is) _query(L"LOCK TABLES %s %s", name, rights); else _query(L"UNLOCK TABLES");
	}
	
	void MySql::select(ssh_wcs express, ssh_wcs from, ssh_wcs where, SelectExpress se, ssh_wcs group, ssh_wcs order, ssh_u offs, ssh_u rows)
	{
		/* SELECT [DISTINCT | DISTINCTROW | ALL] select_expression,... [FROM table_references [WHERE where_definition] [GROUP BY col_name [ASC | DESC], ...] [ORDER BY col_name [ASC | DESC], ...] [LIMIT [offset,] rows] */
		String q_where, q_group, q_order, q_limit;
		if(where) q_where.fmt(L" WHERE %s", where);
		if(group) q_group.fmt(L" GROUP BY %s", group);
		if(order) q_order.fmt(L" ORDER BY %s", order);
		if(offs || rows) q_limit.fmt(L" LIMIT %i, %i", offs, rows);
		_query(L"SELECT %s %s FROM %s%s%s%s%s", q_se, express, from, q_where, q_group, q_order, q_limit);
	}

	ssh_u MySql::update(ssh_wcs tbl, ssh_wcs fields, ssh_wcs values, ssh_wcs where, ssh_u limit)
	{
		/* UPDATE tbl_name SET col_name1=expr1 [, col_name2=expr2, ...] [WHERE where_definition] [LIMIT #] */
		ssh_u pos_f[128], pos_v[128];
		ssh_u cf(ssh_split(L",", 0, fields, pos_f, 128));
		ssh_u cv(ssh_split(L",", 0, values, pos_v, 128));
		if(cf != cv) SSH_THROW(L"");
		String q_lim, q, f(fields), v(values);
		for(ssh_u i = 0; i < cf; i++)
		{
			if(!q.is_empty()) q += L',';
			q += f.substr(fields[pos_f[i * 2]], fields[pos_f[i * 2 + 1]]) + L'=';
			q += v.substr(values[pos_v[i * 2]], values[pos_v[i * 2 + 1]]);
		}
		if(limit) q_lim.fmt(L" LIMIT %i", limit);
		_query(L"UPDATE %s SET %s %s%s", tbl, q, where, q_lim);
		return mysql_affected_rows(&sql);
	}
	
	ssh_u MySql::insert(ssh_wcs tbl, ssh_wcs fields, ssh_wcs values, bool is_replace)
	{
		/*
			INSERT [INTO] tbl_name [(col_name,...)] VALUES (expression,...)
			REPLACE [INTO] tbl_name [(col_name,...)] VALUES (expression,...)
		*/
		ssh_u pos_f[128], pos_v[128];
		ssh_u cf(ssh_split(L",", 0, fields, pos_f, 128));
		ssh_u cv(ssh_split(L",", 0, values, pos_v, 128));
		if(cf != cv) SSH_THROW(L"");
		String qf, qv, f(fields), v(values);
		for(ssh_u i = 0; i < cf; i++)
		{
			if(!qf.is_empty()) qf += L',';
			if(!qv.is_empty()) qv += L',';
			qf += f.substr(fields[pos_f[i * 2]], fields[pos_f[i * 2 + 1]]);
			qv += v.substr(values[pos_v[i * 2]], values[pos_v[i * 2 + 1]]);
		}
		_query(L"%s INTO %s (qf) VALUES (%s)", (is_replace ? L"REPLACE" : L"INSERT"), tbl, qf, qv);
		return mysql_affected_rows(&sql);
	}
	
	ssh_u MySql::remove(ssh_wcs tbl, ssh_wcs where, ssh_wcs order, ssh_u limit)
	{
		/*
			DELETE FROM table_name [WHERE where_definition] [ORDER BY ...] [LIMIT rows]
			TRUNCATE TABLE table_name 
		*/
		ssh_u ret(0);
		if(!where)
		{
			_query(L"TRUNCATE TABLE %s", tbl);
		}
		else
		{
			String q(order ? tmp.fmt(L" ORDER BY %s", order) : L"");
			if(limit) q += tmp.fmt(L" LIMIT %i", limit);
			_query(L"DELETE FROM %s WHERE %s%s", tbl, tmp);
			ret = mysql_affected_rows(&sql);
		}
		return ret;
	}
	
	Array<Map<String, String>, SSH_TYPE> MySql::get_result()
	{
		return Array<Map<String, String>, SSH_TYPE>();
	}
	
	void MySql::_query(ssh_wcs wcs, ...)
	{
		String q;
		va_list args;
		va_start(args, wcs);
		q.fmt(wcs, args);
		va_end(args);

		if(!mysql_query(&sql, ssh_cnv(cp_ansi, q, true)))
			SSH_THROW(L"Не удалось выполнить запрос <%s> к БД! Ошибка - %s!", q, error_string());
	}
}