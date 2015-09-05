
#include "stdafx.h"
#include "ssh_sql.h"

namespace ssh
{
	static ssh_wcs _lck_types[] = { L"READ", L"WRITE", L"READWRITE" };
	static ssh_wcs _tbl_types[] = { L"BDB", L"HEAP", L"ISAM", L"InnoDB", L"MERGE", L"MRG_MYISAM", L"MYISAM" };
	static ssh_wcs _fld_types[] = { L"DOUBLE", L"FLOAT", L"DECIMAL",
									L"CHAR", L"VARCHAR",
									L"TINYINT", L"SMALLINT", L"MEDIUMINT", L"BIGINT", L"INT", L"BIT",
									L"ENUM", L"SET",
									L"TINYBLOB", L"MEDIUMBLOB", L"LONGBLOB", L"BLOB",
									L"TINYTEXT", L"MEDIUMTEXT", L"LONGTEXT", L"TEXT",
									L"DATE", L"TIME", L"DATETIME", L"YEAR", L"TIMESTAMP", L""};
	static ssh_wcs _row_types[] = { L"DEFAULT", L"DYNAMIC", L"FIXED", L"COMPRESSED"};

	void MySql::connect(ssh_wcs _host, ssh_wcs user, ssh_wcs pass, ssh_wcs db, ssh_wcs _charset)
	{
		SSH_TRACE;
		
		disconnect();

		if(!mysql_init(&sql)) SSH_THROW(L"Не удалось инициализировать mysql! Ошибка - %s!", error_string());
		if(mysql_options(&sql, MYSQL_SET_CHARSET_NAME, ssh_cnv(cp_ansi, _charset, true))) SSH_THROW(L"Не удалось установить кодировку mysql! Ошибка - %s!", error_string());

		String host(_host);
		ssh_l port(0), pos;
		if((pos = host.find(L':')) >= 0)
		{
			port = host.toNum<ssh_l>(pos + 1);
			host = host.left(pos);
		}
		if(!mysql_real_connect(&sql, ssh_cnv(cp_ansi, host, true), ssh_cnv(cp_ansi, user, true), ssh_cnv(cp_ansi, pass, true), ssh_cnv(cp_ansi, db, true), (unsigned int)port, nullptr, 0))
			SSH_THROW(L"Не удалось соединиться с сервером mysql! Ошибка - %s!", error_string());
		charset = _charset;
	}
	
	void MySql::disconnect()
	{
		SSH_TRACE;
		if(is_conn) mysql_close(&sql);
		is_conn = false;
	}
	
	void MySql::add_table(const FIELD flds[], ssh_wcs name, ssh_wcs comment, TypesTable type, TypesRow row)
	{
		SSH_TRACE;
		String q;
		const FIELD* f(&flds[0]);
		// поля
		while(f->name)
		{
			if(!q.is_empty()) q += L',';
			q += make_field(f);
			f++;
		}
		_query(L"CREATE TABLE IF NOT EXISTS `%s` (%s) ENGINE=%s COMMENT='%s'", name, q, _tbl_types[(int)type], comment);
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

	ssh_u MySql::update(ssh_wcs tbl, ssh_wcs fields, ssh_wcs values, ssh_wcs comp, bool is_locked, ssh_u limit)
	{
		SSH_TRACE;
		int pos_f[128], pos_v[128];
		int cf(ssh_split(L',', fields, pos_f, 64));
		int cv(ssh_split(L',', values, pos_v, 64));
		if(cf != cv || !cf) SSH_THROW(L"Недопустимо задан оператор UPDATE!");
		String q_lim, q;
		for(int i = 0; i < cf; i++)
		{
			if(!q.is_empty()) q += L',';
			q += L'`' + String(fields + pos_f[i * 2], pos_f[i * 2 + 1]).trim() + L"`=";
			q += L'\'' + String(values + pos_v[i * 2], pos_v[i * 2 + 1]).trim(L"'") + L'\'';
		}
		if(limit) q_lim.fmt(L" LIMIT %i", limit);
		
		if(is_locked) lock_table(tbl, true);
		_query(L"UPDATE `%s` SET %s %s%s", tbl, q, comp, q_lim);
		if(is_locked) lock_table(tbl, false);
		
		return mysql_affected_rows(&sql);
	}
	
	ssh_u MySql::insert(ssh_wcs tbl, ssh_wcs fields, ssh_wcs values, bool is_locked, bool is_replace)
	{
		SSH_TRACE;
		int pos_f[128], pos_v[128];
		int cf(ssh_split(L',', fields, pos_f, 64));
		int cv(ssh_split(L',', values, pos_v, 64));
		if(cf != cv || !cf) SSH_THROW(L"Недопустимо задан оператор INSERT!");
		String qf, qv;
		for(int i = 0; i < cf; i++)
		{
			if(!qf.is_empty()) qf += L',';
			if(!qv.is_empty()) qv += L',';
			qf += L'`' + String(fields + pos_f[i * 2], pos_f[i * 2 + 1]).trim() + L'`';
			qv += L'\'' + String(values + pos_v[i * 2], pos_v[i * 2 + 1]).trim(L"'") + L'\'';
		}
		
		if(is_locked) lock_table(tbl, true);
		_query(L"%s INTO `%s` (%s) VALUES (%s)", (is_replace ? L"REPLACE" : L"INSERT"), tbl, qf, qv);
		if(is_locked) lock_table(tbl, false);
		
		return mysql_affected_rows(&sql);
	}
	
	ssh_u MySql::remove(ssh_wcs tbl, bool is_locked, ssh_wcs comp, ssh_wcs order, ssh_u limit)
	{
		SSH_TRACE;
	
		if(!comp) return truncate_table(tbl, is_locked);
		
		String q, tmp;
		if(order) q.fmt(L" ORDER BY %s", order);
		if(limit) q += tmp.fmt(L" LIMIT %i", limit);

		if(is_locked) lock_table(tbl, true);
		_query(L"DELETE FROM `%s` WHERE %s%s", tbl, tmp);
		if(is_locked) lock_table(tbl, false);

		return mysql_affected_rows(&sql);
	}
	
	ssh_u MySql::truncate_table(ssh_wcs name, bool is_locked)
	{
		SSH_TRACE;
		if(is_locked) lock_table(name, true);
		_query(L"TRUNCATE TABLE `%s`", name);
		if(is_locked) lock_table(name, false);
		return 0;
	}

	void MySql::_query(ssh_wcs wcs, ...)
	{
		SSH_TRACE;

		String q;

		va_list args;
		va_start(args, wcs);
		q.fmt(wcs, args);
		va_end(args);

		if(mysql_query(&sql, ssh_cnv(charset, q, true)))
			SSH_THROW(L"Не удалось выполнить запрос к БД! Ошибка - <%s>!", error_string());
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
			ssh_u rows(mysql_num_rows(result));
			MYSQL_FIELD* f(mysql_fetch_fields(result));
			arrs.set_size(rows);
			for(ssh_u j = 0; j < rows; j++)
			{
				MYSQL_ROW row(mysql_fetch_row(result));
				Map<String, String, SSH_TYPE, SSH_TYPE>* map(&arrs[j]);
				ssh_d* lengths(mysql_fetch_lengths(result));
				for(ssh_u i = 0; i < flds; i++)
				{
					String val;
					ssh_d l(lengths[i]);
					ssh_ws* dt(nullptr);
					if(l)
					{
						TypesField tp(cnv_type_field(f[i].type));
						switch(tp)
						{
							case TypesField::BIT: break;
							case TypesField::TINYINT:
							case TypesField::SMALLINT:
							case TypesField::MEDIUMINT:
							case TypesField::BIGINT:
							case TypesField::INT:
							case TypesField::FLOAT:
							case TypesField::DOUBLE:
							case TypesField::DECIMAL:
							case TypesField::SET:
							case TypesField::ENUM: val = row[i]; break;
							case TypesField::DATETIME:
							case TypesField::TIMESTAMP: dt = L"YYYY-MM-DD HH:XX:SS"; break;
							case TypesField::YEAR: dt = L"YYYY"; break;
							case TypesField::DATE: dt = L"YYYY-MM-DD"; break;
							case TypesField::TIME: dt = L"HH:XX:SS"; break;
							case TypesField::CHAR:
							case TypesField::VARCHAR:
							case TypesField::TINYBLOB:
							case TypesField::MEDIUMBLOB:
							case TypesField::LONGBLOB:
							case TypesField::BLOB: val = ssh_cnv(charset, Buffer<ssh_cs>(row[i], l, false), 0); break;
						}
					}
					if(dt)
					{
						val = row[i];
						int tm[6] = { 1970, 1, 2, 3, 0, 0};
						ssh_wcs pattern[] = { L"YY", L"MM", L"DD", L"HH", L"XX", L"SS" };
						ssh_l pos;
						for(int i = 0; i < 6; i++)
						{
							if((pos = (wcsstr(dt, pattern[i]) - dt)) >= 0) tm[i] = val.toNum<int>(pos);
						}
						val = String(Time(tm[0], tm[1], tm[2], tm[3], tm[4], tm[5]), String::_dec);
					}
					(*map)[f[i].name] = val;
				}
			}
			mysql_free_result(result);
		}
		return arrs;
	}
	
	MySql::TypesField MySql::cnv_type_field(int tp) const
	{
		static TypesField _t_flds[] = { TypesField::DECIMAL, TypesField::TINYINT, TypesField::SMALLINT, TypesField::INT, TypesField::FLOAT, TypesField::DOUBLE, TypesField::BIT, TypesField::TIMESTAMP,
										TypesField::BIGINT, TypesField::MEDIUMINT, TypesField::DATE, TypesField::TIME, TypesField::DATETIME, TypesField::YEAR, TypesField::DATE, TypesField::VARCHAR,
										TypesField::BIT, TypesField::TIMESTAMP, TypesField::DATETIME, TypesField::TIME, TypesField::DECIMAL, TypesField::ENUM, TypesField::SET, TypesField::TINYBLOB,
										TypesField::MEDIUMBLOB, TypesField::LONGBLOB, TypesField::BLOB, TypesField::VARCHAR, TypesField::VARCHAR, TypesField::NONE};
		if(tp >= MYSQL_TYPE_NEWDECIMAL)
		{
			tp -= MYSQL_TYPE_NEWDECIMAL;
			tp += MYSQL_TYPE_TIME2 + 1;
		}
		return _t_flds[tp];
	}
	
	MySql::ClassType MySql::get_class_type(MySql::TypesField tp) const
	{
		static ClassType _c_type[] = {	ClassType::REAL, ClassType::REAL, ClassType::REAL, ClassType::STRING, ClassType::STRING, ClassType::NUMBER, ClassType::NUMBER, ClassType::NUMBER,
										ClassType::NUMBER, ClassType::NUMBER, ClassType::NUMBER, ClassType::STRING, ClassType::STRING, ClassType::STRING,
										ClassType::STRING, ClassType::STRING, ClassType::STRING, ClassType::STRING, ClassType::STRING, ClassType::STRING, ClassType::STRING,
										ClassType::DATETIME, ClassType::DATETIME, ClassType::DATETIME, ClassType::DATETIME, ClassType::DATETIME, ClassType::BIN};
		return _c_type[(int)tp];
	}
	
	String MySql::make_opts(int opt, MySql::ClassType cls) const
	{
		SSH_TRACE;
		String ret;
		switch(cls)
		{
			case ClassType::NUMBER:
				if((opt & OptionsField::AUTO_INCREMENT)) ret += L" AUTO_INCREMENT";
			case ClassType::REAL:
				if((opt & OptionsField::UNSIGNED)) ret += L" UNSIGNED";
				if((opt & OptionsField::ZEROFILL)) ret += L" ZEROFILL";
				break;
			case ClassType::STRING:
				if((opt & OptionsField::BINARY)) ret += L" BINARY";
				break;
			case ClassType::DATETIME:
				break;
			case ClassType::BIN:
				break;
		}
		if((opt & OptionsField::NOT_NULL)) ret += L" NOT NULL"; else ret += L" NULL";
		if((opt & OptionsField::UNIQUE)) ret += L" UNIQUE";

		return ret;
	}

	String MySql::make_field(const MySql::FIELD* f) const
	{
		SSH_TRACE;
		String len, def, com, idx, tmp;
		ClassType cls(get_class_type(f->type));
		if(f->type < TypesField::TINYBLOB)
		{
			if(f->def && !(f->opt & OptionsField::AUTO_INCREMENT)) def.fmt(L" DEFAULT '%s'", f->def);
			if(f->type < TypesField::CHAR && f->length) len.fmt(L"(%i,%i)", f->length, (f->decimals > 0 ? f->decimals : 2));
			else if(f->type < TypesField::ENUM)
			{
				ssh_d length(f->length);
				if(!length)
				{
					if(f->type == TypesField::CHAR) length = 4;
					else if(f->type == TypesField::VARCHAR) length = 255;
				}
				if(length) len.fmt(L"(%i)", length);
			}
			else if(f->type == TypesField::ENUM || f->type == TypesField::SET)
			{
				int vec[128];
				int c(ssh_split(L',', f->enums, vec, 64));
				if(!c) SSH_THROW(L"Недопустимо заданы значения перечисления для поля <%s>!", f->name);
				len = L'(';
				for(int i = 0; i < c; i++)
				{
					if(len.length() > 1) len += L',';
					len += L'\'' + String(f->enums + vec[i * 2], vec[i * 2 + 1]).trim(L"'") + L'\'';
				}
				len += L')';
			}
		}
		if(f->opt & OptionsField::KEY)
		{
			if(f->index_length && cls == ClassType::STRING) tmp.fmt(L"(%i)", f->index_length);
			idx.fmt(L", %sKEY `%s`(`%s`%s)", ((f->opt & OptionsField::FULLTEXT && cls == ClassType::STRING) ? L"FULLTEXT " : (f->opt & OptionsField::PRIMARY ? L"PRIMARY " : L"")), ssh_gen_name(f->name, false), f->name, tmp);
		}
		if(f->comment) com.fmt(L" COMMENT '%s'", f->comment);
		return tmp.fmt(L"`%s` %s%s%s%s%s%s", f->name, _fld_types[(int)f->type], len, make_opts(f->opt, cls), def, com, idx);
	}
}
