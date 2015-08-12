
#include "stdafx.h"
#include "ssh_srlz.h"

namespace ssh
{
	static Xml* _xml(nullptr);
	static File* _fl(nullptr);
	static Serialize::SCHEME* _sc(nullptr);
	static ssh_cs* _buf(nullptr);

	void Serialize::open(const Buffer<ssh_cs>& buf, void* srlz, bool is_xml)
	{
		SSH_TRACE;
		ssh_l offs((ssh_b*)srlz - (ssh_b*)this);
		_buf = buf;
		_sc = get_scheme();
		if(is_xml)
		{
			Xml xml(buf);
			_xml = &xml;
			readXml(_xml->root(), offs);
		}
		else
		{
			readBin(offs);
		}
	}

	void Serialize::save(const String& path, void* srlz, bool is_xml, ssh_wcs code)
	{
		SSH_TRACE;
		ssh_l offs((ssh_b*)srlz - (ssh_b*)this);
		_sc = get_scheme();
		if(is_xml)
		{
			Xml xml;
			_xml = &xml;
			writeXml(_xml->root(), offs);
			_xml->save(path, code);
		}
		else
		{
			File f(path, File::create_write);
			_fl = &f;
			writeBin(offs);
		}
	}

	void Serialize::writeXml(HXML hp, ssh_l p_offs)
	{
		SCHEME* sc;
		// создать узел
		HXML h(_xml->add_node(hp, _sc->name, _sc->def));
		ssh_u _ID(_sc++->ID);
		while((sc = _sc++))
		{
			if(!sc->name) break;
			ssh_u flg(sc->flags);
			ssh_l offs(sc->offs + p_offs);
			ssh_u count(sc->count);
			String sval;
			while(count--)
			{
				if(_ID)
				{
					if(!(flg & SC_OBJ) || sc->ID == -1) { if(sc->ID != -1) _sc--; return; }
					if(sc->ID != _ID) { _sc--; writeXml(h, offs); if(count) _sc = sc + 1; }
				}
				else if((flg & SC_OBJ)) { _sc--; writeXml(h, offs); if(count) _sc = sc + 1; }
				else if((flg & SC_NODE)) { Serialize* srlz((Serialize*)((ssh_b*)(this) + offs)); _sc = srlz->get_scheme(); srlz->writeXml(h, 0); _sc = sc + 1; }
				if((flg & SC_VAR))
				{
					if(!sval.is_empty()) sval += L',';
					ssh_b* obj((ssh_b*)(this) + offs);
					ssh_u val(0);
					switch(sc->hash)
					{
						case _hash_wcs:
						case _hash_ccs: break;
						case _hash_string: sval += *(String*)obj; break;
						case _hash_char: sval += String((ssh_ccs)obj, (ssh_u)1); break;
						case _hash_wchar: sval += *(ssh_ws*)obj; break;
						case _hash_float: sval += String(*(ssh_u*)obj, String::_flt); break;
						case _hash_double: sval += String(*(ssh_u*)obj, String::_dbl); break;
						default:
							if(sc->width <= 8)
							{
								memcpy(&val, obj, sc->width);
								switch(flg & 15)
								{
									case SC_ENUM:
									case SC_FLAGS: sval += hlp->cnvString(val, sc->stk, sc->def, ((flg & SC_ENUM) == SC_ENUM)); break;
									case SC_BOOL: sval += (val == 0 ? L"false" : L"true"); break;
									default: sval += String(val, (String::Radix)(flg & 3));
								}
							}
							break;
					}
				}
				offs += sc->width;
			}
			if((flg & SC_VAR)) _xml->set_attr(h, sc->name, ((flg & SC_BASE64) ? ssh_base64(cp_utf, sval) : sval));
		}
	}

	void Serialize::writeBin(ssh_l p_offs)
	{
		SCHEME* sc;
		ssh_u _ID(_sc++->ID);
		String sval;
		while((sc = _sc++))
		{
			if(!sc->name) break;
			ssh_u flg(sc->flags);
			ssh_l offs(sc->offs + p_offs);
			ssh_u count(sc->count);
			while(count--)
			{
				if(_ID)
				{
					// вложенный класс без своей схемы
					if(!(flg & SC_OBJ) || sc->ID == -1) { if(sc->ID != -1) _sc--; return; }
					if(sc->ID != _ID) { _sc--; writeBin(offs); if(count) _sc = sc + 1; }
				}
				else if((flg & SC_OBJ)) { _sc--; writeBin(offs); if(count) _sc = sc + 1; }
				else if((flg & SC_NODE)) { Serialize* srlz((Serialize*)((ssh_b*)(this) + offs)); _sc = srlz->get_scheme(); srlz->writeBin(0); _sc = sc + 1; }
				if((flg & SC_VAR))
				{
					ssh_b* obj((ssh_b*)(this) + offs);
					ssh_cs* _cs;
					ssh_u len(0);
					switch(sc->hash)
					{
						case _hash_wcs:
						case _hash_ccs: break;
						case _hash_string: sval = *(String*)obj; _cs = (ssh_cs*)sval.buffer(); len = sval.length() * 2 + 2; break;
						default: _cs = (ssh_cs*)obj; len = sc->width; break;
					}
					if(len) _fl->write(_cs, len);
				}
				offs += sc->width;
			}
		}
	}
	
	void Serialize::readXml(HXML hp, ssh_l p_offs, ssh_l idx)
	{
		SCHEME* sc;
		HXML h;
		String sval;
		// получить узел
		if(!(h = _xml->node(hp, (idx ? nullptr : _sc->name), idx)))
		{
			SSH_LOG(L"Ќе найден узел <%s, индекс: %i> xml!", _sc->name, idx);
			return;
		}
		ssh_u _ID(_sc++->ID);
		ssh_l i;
		while((sc = _sc++))
		{
			if(!sc->name) break;
			ssh_u flg(sc->flags);
			ssh_l offs(sc->offs + p_offs);
			ssh_l count(sc->count);
			ssh_u val, pos(0);
			ssh_ws* _ws;
			if((flg & SC_VAR))
			{
				sval = _xml->attr<String>(h, sc->name, sc->def);
				if((flg & SC_BASE64)) sval = ssh_base64(sval, true). to<ssh_ws>();
				_ws = sval.buffer();
			}
			for(i = 0; i < count; i++)
			{
				if(_ID)
				{
					if(!(flg & SC_OBJ) || sc->ID == -1) { if(sc->ID != -1) _sc--; return; }
					if(sc->ID != _ID) { _sc--; readXml(h, offs, i); if((i + 1) < count) _sc = sc + 1; }
				}
				else if((flg & SC_OBJ)) { _sc--; readXml(h, offs, i); if((i + 1) < count) _sc = sc + 1; }
				else if((flg & SC_NODE)) { Serialize* srlz((Serialize*)((ssh_b*)(this) + offs)); _sc = srlz->get_scheme(); srlz->readXml(h, 0, i); _sc = sc + 1; }
				if((flg & SC_VAR))
				{
					ssh_ws* _nws(nullptr);
					ssh_b* obj((ssh_b*)(this) + offs);
					if(*_ws && count > 1)
					{
						pos = (_ws - sval.buffer());
						if((_nws = wcschr(_ws, L','))) *_nws = 0;
					}
					switch(sc->hash)
					{
						case _hash_wcs:
						case _hash_ccs: break;
						case _hash_string: *(String*)obj = _ws; break;
						case _hash_char: *(ssh_cs*)obj = *ssh_cnv(cp_ansi, _ws, false).to<ssh_cs>(); break;
						case _hash_wchar: *(ssh_ws*)obj = *_ws; break;
						case _hash_float: *(float*)obj = sval.toNum<float>(pos, String::_flt); break;
						case _hash_double: *(double*)obj = sval.toNum<double>(pos, String::_dbl); break;
						default:
							if(sc->width <= 8)
							{
								switch(flg & 15)
								{
									case SC_ENUM:
									case SC_FLAGS: val = hlp->cnvValue(_ws, sc->stk, String(sc->def)); break;
									case SC_BOOL: val = (wcscmp(_ws, L"true") == 0); break;
									default: val = sval.toNum<ssh_u>(pos, (String::Radix)(flg & 3)); break;
								}
								memcpy(obj, &val, sc->width);
							}
							break;
					}
					if(_nws) _ws = _nws + 1;
					else if(*_ws) pos = sval.length(), _ws = (sval.buffer() + pos);
				}
				offs += sc->width;
			}
		}
	}

	void Serialize::readBin(ssh_l p_offs)
	{
		SCHEME* sc;
		ssh_u _ID(_sc++->ID);
		while((sc = _sc++))
		{
			if(!sc->name) break;
			ssh_u flg(sc->flags);
			ssh_l offs(sc->offs + p_offs);
			ssh_l count(sc->count);
			while(count--)
			{
				if(_ID)
				{
					if(!(flg & SC_OBJ) || sc->ID == -1) { if(sc->ID != -1) _sc--; return; }
					if(sc->ID != _ID) { _sc--; readBin(offs); if(count) _sc = sc + 1; }
				}
				else if((flg & SC_OBJ)) { _sc--; readBin(offs); if(count) _sc = sc + 1; }
				else if((flg & SC_NODE)) { Serialize* srlz((Serialize*)((ssh_b*)(this) + offs)); _sc = srlz->get_scheme(); srlz->readBin(0); _sc = sc + 1; }
				if((flg & SC_VAR))
				{
					ssh_b* obj((ssh_b*)(this) + offs);
					switch(sc->hash)
					{
						case _hash_wcs:
						case _hash_ccs: break;
						case _hash_string: *(String*)obj = (ssh_ws*)_buf; _buf += (wcslen((ssh_ws*)_buf) * 2 + 2); break;
						default: memcpy(obj, _buf, sc->width); _buf += sc->width; break;
					}
				}
				offs += sc->width;
			}
		}
	}
}
