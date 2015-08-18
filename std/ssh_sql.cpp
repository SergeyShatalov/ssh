
#include "stdafx.h"
#include "ssh_sql.h"

namespace ssh
{
	void MySql::connect(ssh_wcs _host, ssh_wcs user, ssh_wcs pass, ssh_wcs db)
	{
		SSH_TRACE;
		disconnect();

		if(!mysql_init(&sql))
			SSH_THROW(L"Не удалось инициализировать mysql! Ошибка - %s!", error_string());
		String host(_host);
		ssh_l port(0), pos;
		if((pos = host.find(L':')) >= 0)
		{
			port = host.toNum<ssh_l>(pos + 1);
			host = host.left(pos);
		}
		if(!mysql_real_connect(&sql, ssh_cnv(cp_ansi, host, true), ssh_cnv(cp_ansi, user, true), ssh_cnv(cp_ansi, pass, true), ssh_cnv(cp_ansi, db, true), (unsigned int)port, nullptr, 0))
			SSH_THROW(L"Не удалось соединиться с сервером mysql! Ошибка - %s!", error_string());
	}
	
	void MySql::disconnect()
	{
		SSH_TRACE;
		if(is_conn) mysql_close(&sql);
		is_conn = false;
	}
	
	void MySql::add_db(ssh_wcs name)
	{
		SSH_TRACE;
		_query(L"CREATE DATABASE IF NOT EXISTS %s", name);
	}
	
	void MySql::del_db(ssh_wcs name)
	{
		SSH_TRACE;
		_query(L"DROP DATABASE IF_EXISTS %s", name);
	}
	
	void MySql::sel_db(ssh_wcs name)
	{
		SSH_TRACE;
		_query(L"USE %s", name);
	}
	
	void MySql::add_table(ssh_wcs name, ssh_wcs auto_inc, ssh_wcs fields, ssh_wcs types, ssh_wcs defs, ssh_wcs comment)
	{
		SSH_TRACE;
		int pos_f[128], pos_t[128], pos_d[128];
		int cf(ssh_split(L',', fields, pos_f, 64));
		int ct(ssh_split(L',', types, pos_t, 64));
		int cd(ssh_split(L',', defs, pos_d, 64));
		if(cf != ct) SSH_THROW(L"");
		String q, c;
		if(auto_inc) q.fmt(L"%s int(11) NOT NULL PRIMARY KEY AUTO_INCREMENT", auto_inc);
		for(int i = 0; i < cf; i++)
		{
			if(!q.is_empty()) q += L',';
			q += String(fields + pos_f[i * 2], pos_f[i * 2 + 1]).trim() + L' ';
			q += String(types + pos_t[i * 2], pos_t[i * 2 + 1]).trim();
			if(cd >= i && pos_d[i * 2 + 1] > 0)
			{
				String def(defs + pos_d[i * 2], pos_d[i * 2 + 1]);
				if(def.trim().length()) q += L" DEFAULT \'" + def + L'\'';
			}
		}
		if(comment) c.fmt(L" COMMENT=\"%s\"", comment);
		_query(L"CREATE TABLE IF NOT EXISTS %s (%s)%s", name, q, c);
	}
	
	void MySql::del_table(ssh_wcs name)
	{
		SSH_TRACE;
		_query(L"DROP TABLE IF EXISTS %s", name);
	}
	
	void MySql::lock_table(ssh_wcs name, LockTables lt, bool is)
	{
		SSH_TRACE;
		String rights;
		switch(lt)
		{
			case LockTables::read: rights = L"READ"; break;
			case LockTables::write: rights = L"WRITE"; break;
			case LockTables::readwrite: rights = L"READ WRITE"; break;
		}
		if(is) _query(L"LOCK TABLES %s %s", name, rights); else _query(L"UNLOCK TABLES");
	}
	
	Array<Map<String, String, SSH_TYPE, SSH_TYPE>, SSH_TYPE> MySql::select(ssh_wcs express, ssh_wcs tbls, bool is_duplicate, ssh_wcs comp, ssh_wcs group, ssh_wcs order, ssh_u offs, ssh_u rows)
	{
		SSH_TRACE;
		String q_where, q_group, q_order, q_limit, q_se;
		if(comp) q_where.fmt(L" WHERE %s", comp);
		if(group) q_group.fmt(L" GROUP BY %s", group);
		if(order) q_order.fmt(L" ORDER BY %s", order);
		if(offs || rows) q_limit.fmt(L" LIMIT %i, %i", offs, rows);
		_query(L"SELECT %s%s FROM %s%s%s%s%s", (is_duplicate ? L"DISTINCT " : L""), express, tbls, q_where, q_group, q_order, q_limit);
		return get_result();
	}

	ssh_u MySql::update(ssh_wcs tbl, ssh_wcs fields, ssh_wcs values, ssh_wcs comp, ssh_u limit)
	{
		SSH_TRACE;
		int pos_f[128], pos_v[128];
		int cf(ssh_split(L',', fields, pos_f, 64));
		int cv(ssh_split(L',', values, pos_v, 64));
		if(cf != cv) SSH_THROW(L"Недопустимо задан оператор UPDATE!");
		String q_lim, q;
		for(int i = 0; i < cf; i++)
		{
			if(!q.is_empty()) q += L',';
			q += String(fields + pos_f[i * 2], pos_f[i * 2 + 1]).trim() + L'=';
			q += L'\'' + String(values + pos_v[i * 2], pos_v[i * 2 + 1]).trim(L"'") + L'\'';
		}
		if(limit) q_lim.fmt(L" LIMIT %i", limit);
		_query(L"UPDATE %s SET %s %s%s", tbl, q, comp, q_lim);
		return mysql_affected_rows(&sql);
	}
	
	ssh_u MySql::insert(ssh_wcs tbl, ssh_wcs fields, ssh_wcs values, bool is_replace)
	{
		SSH_TRACE;
		int pos_f[128], pos_v[128];
		int cf(ssh_split(L',', fields, pos_f, 64));
		int cv(ssh_split(L',', values, pos_v, 64));
		if(cf != cv) SSH_THROW(L"Недопустимо задан оператор INSERT!");
		String qf, qv;
		for(int i = 0; i < cf; i++)
		{
			if(!qf.is_empty()) qf += L',';
			if(!qv.is_empty()) qv += L',';
			qf += String(fields + pos_f[i * 2], pos_f[i * 2 + 1]).trim();
			qv += L'\'' + String(values + pos_v[i * 2], pos_v[i * 2 + 1]).trim(L"'") + L'\'';
		}
		_query(L"%s INTO %s (%s) VALUES (%s)", (is_replace ? L"REPLACE" : L"INSERT"), tbl, qf, qv);
		return mysql_affected_rows(&sql);
	}
	
	ssh_u MySql::remove(ssh_wcs tbl, ssh_wcs comp, ssh_wcs order, ssh_u limit)
	{
		SSH_TRACE;
		ssh_u ret(0);
		if(!comp)
		{
			_query(L"TRUNCATE TABLE %s", tbl);
		}
		else
		{
			String q, tmp;
			if(order) q.fmt(L" ORDER BY %s", order);
			if(limit) q += tmp.fmt(L" LIMIT %i", limit);
			_query(L"DELETE FROM %s WHERE %s%s", tbl, tmp);
			ret = mysql_affected_rows(&sql);
		}
		return ret;
	}
	
	Array<Map<String, String, SSH_TYPE, SSH_TYPE>, SSH_TYPE> MySql::get_result()
	{
		SSH_TRACE;
		Array<Map<String, String, SSH_TYPE, SSH_TYPE>, SSH_TYPE> arrs;
		if(mysql_field_count(&sql))
		{
			MYSQL_RES* result;
			if(!(result = mysql_store_result(&sql))) SSH_THROW(L"Внутренняя ошибка сервера - <%s>!", error_string());
			ssh_u flds(mysql_num_fields(result));
			MYSQL_FIELD* f(mysql_fetch_fields(result));
			MYSQL_ROW row;
			while((row = mysql_fetch_row(result)))
			{
				Map<String, String, SSH_TYPE, SSH_TYPE> map;
				ssh_d* lengths(mysql_fetch_lengths(result));
				for(ssh_u i = 0; i < flds; i++)
				{
					String val;
					if(lengths[i])
					{
						switch(cnv_type_field(f[i].type))
						{
							case TypeField::_string:
								val = ssh_cnv(L"utf-8", Buffer<ssh_cs>(row[i], lengths[i], false), 0);
								break;
							case TypeField::_number:
							case TypeField::_real:
								val = row[i];
								break;
							case TypeField::_binary:
								val = ssh_base64(Buffer<ssh_cs>(row[i], lengths[i], false));
								break;
							case TypeField::_time:
							case TypeField::_date:
								val = row[i];
						}
					}
					map[f[i].name] = val;
				}
				arrs += map;
			}
			mysql_free_result(result);
		}
		return arrs;
	}
	
	MySql::TypeField MySql::cnv_type_field(int tp) const
	{
		switch(tp)
		{
			case MYSQL_TYPE_TINY:
			case MYSQL_TYPE_SHORT:
			case MYSQL_TYPE_LONG:
			case MYSQL_TYPE_NULL:
			case MYSQL_TYPE_LONGLONG:
			case MYSQL_TYPE_INT24:
			case MYSQL_TYPE_NEWDECIMAL:
				return TypeField::_number;
			case MYSQL_TYPE_DECIMAL:
			case MYSQL_TYPE_FLOAT:
			case MYSQL_TYPE_DOUBLE:
				return TypeField::_real;
			case MYSQL_TYPE_VARCHAR:
			case MYSQL_TYPE_VAR_STRING:
			case MYSQL_TYPE_STRING:
			case MYSQL_TYPE_BLOB:
				return TypeField::_string;
			case MYSQL_TYPE_TIMESTAMP:
			case MYSQL_TYPE_TIME:
			case MYSQL_TYPE_TIME2:
			case MYSQL_TYPE_DATETIME:
			case MYSQL_TYPE_TIMESTAMP2:
			case MYSQL_TYPE_DATETIME2:
				return TypeField::_time;
			case MYSQL_TYPE_DATE:
			case MYSQL_TYPE_NEWDATE:
			case MYSQL_TYPE_YEAR:
				return TypeField::_date;
			case MYSQL_TYPE_TINY_BLOB:
			case MYSQL_TYPE_MEDIUM_BLOB:
			case MYSQL_TYPE_LONG_BLOB:
			case MYSQL_TYPE_GEOMETRY:
				return TypeField::_binary;
			case MYSQL_TYPE_BIT:
			case MYSQL_TYPE_ENUM:
			case MYSQL_TYPE_SET:
				break;
		};
		return TypeField::_undef;
	}
	void MySql::_query(ssh_wcs wcs, ...)
	{
		SSH_TRACE;
		String q;
		va_list args;
		va_start(args, wcs);
		q.fmt(wcs, args);
		va_end(args);

		if(mysql_query(&sql, ssh_cnv(L"utf-8", q, true)))
			SSH_THROW(L"Не удалось выполнить запрос к БД! Ошибка - <%s>!", error_string());
	}
}