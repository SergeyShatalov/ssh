
#include "stdafx.h"
#include "ssh_srlz.h"

namespace ssh
{
	Serialize::SCHEME* Serialize::_sc(nullptr);
	Xml* Serialize::_xml(nullptr);
	File* Serialize::_fl(nullptr);
	ssh_cs* _buf(nullptr);

	void Serialize::openXml(const Buffer<ssh_cs>& buf, void* srlz)
	{
		SSH_TRACE;
		Xml xml(buf);
		_xml = &xml;
		_sc = get_scheme();
		readXml(xml.root(), (ssh_b*)srlz - (ssh_b*)this);
	}
	void Serialize::openBin(const Buffer<ssh_cs>& buf, void* srlz)
	{
		SSH_TRACE;
		SCHEME* sc(get_scheme());
		_sc = get_scheme();
		_buf = buf;
		readBin((ssh_b*)srlz - (ssh_b*)this);
	}
	void Serialize::saveXml(const String& path, ssh_wcs code, void* srlz)
	{
		SSH_TRACE;
		Xml xml;
		_xml = &xml;
		_sc = get_scheme();
		writeXml(xml.root(), (ssh_b*)srlz - (ssh_b*)this);
		xml.save(path, code);
	}
	void Serialize::saveBin(const String& path, void* srlz)
	{
		SSH_TRACE;
		File f(path, File::create_write);
		_sc = get_scheme();
		_fl = &f;
		writeBin((ssh_b*)srlz - (ssh_b*)this);
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
			for(ssh_u i = 0; i < count; i++)
			{
				if(_ID)
				{
					if(!(flg & SC_OBJ) || sc->ID == -1) { if(sc->ID != -1) _sc--; return; }
					if(sc->ID != _ID) { _sc--; writeXml(h, offs); if(i + 1 < count) _sc = sc + 1; }
				}
				else if((flg & SC_OBJ)) { _sc--; writeXml(h, offs); if(i + 1 < count) _sc = sc + 1; }
				else if((flg & SC_NODE)) { Serialize* srlz((Serialize*)((ssh_b*)(this) + offs)); _sc = srlz->get_scheme(); srlz->writeXml(h, 0); _sc = sc + 1; }
				if((flg & SC_VAR))
				{
					// простая переменная
					if(!sval.is_empty() && count) sval += L',';
					ssh_u val(0);
					ssh_b* obj((ssh_b*)(this) + offs);
					switch(sc->hash)
					{
						case _hash_string: sval += *(String*)obj; break;
						case _hash_char: sval += *(ssh_cs*)obj; break;
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
			if((flg & SC_VAR)) _xml->set_attr(h, sc->name, sval);
		}
	}

	void Serialize::writeBin(ssh_l p_offs)
	{
		SCHEME* sc;
		ssh_u _ID(_sc++->ID);
		while((sc = _sc++))
		{
			if(!sc->name) break;
			ssh_u flg(sc->flags);
			ssh_l offs(sc->offs + p_offs);
			ssh_u count(sc->count);
			String sval;
			for(ssh_u i = 0; i < count; i++)
			{
				if(_ID)
				{
					// вложенный класс без своей схемы
					if(!(flg & SC_OBJ) || sc->ID == -1) { if(sc->ID != -1) _sc--; return; }
					if(sc->ID != _ID) { _sc--; writeBin(offs); if(i + 1 < count) _sc = sc + 1; }
				}
				else if((flg & SC_OBJ)) { _sc--; writeBin(offs); if(i + 1 < count) _sc = sc + 1; }
				else if((flg & SC_NODE)) { Serialize* srlz((Serialize*)((ssh_b*)(this) + offs)); _sc = srlz->get_scheme(); srlz->writeBin(0); _sc = sc + 1; }
				if((flg & SC_VAR))
				{
					ssh_b* obj((ssh_b*)(this) + offs);
					ssh_cs* _cs;
					ssh_u len;
					String sval;
					if(sc->hash == _hash_string) { sval = *(String*)obj; _cs = (ssh_cs*)sval.buffer(); len = sval.length() * 2 + 2; } else { _cs = (ssh_cs*)obj; len = sc->width; }
					_fl->write(_cs, len);
				}
				offs += sc->width;
			}
		}
	}
	
	void Serialize::readXml(HXML hp, ssh_l p_offs, ssh_l idx)
	{
		SCHEME* sc;
		// создать узел
		HXML h(_xml->node(hp, (idx ? nullptr : _sc->name), idx));
		ssh_u _ID(_sc++->ID);
		while((sc = _sc++))
		{
			if(!sc->name) break;
			ssh_u flg(sc->flags);
			ssh_l offs(sc->offs + p_offs);
			ssh_l count(sc->count);
			String sval;
			for(ssh_l i = 0; i < count; i++)
			{
				if(_ID)
				{
					if(!(flg & SC_OBJ) || sc->ID == -1) { if(sc->ID != -1) _sc--; return; }
					if(sc->ID != _ID) { _sc--; readXml(h, offs, i); if(i + 1 < count) _sc = sc + 1; }
				}
				else if((flg & SC_OBJ)) { _sc--; readXml(h, offs, i); if(i + 1 < count) _sc = sc + 1; }
				else if((flg & SC_NODE)) { Serialize* srlz((Serialize*)((ssh_b*)(this) + offs)); _sc = srlz->get_scheme(); srlz->readXml(h, 0, i); _sc = sc + 1; }
				if((flg & SC_VAR))
				{
					String sval;
					ssh_u val;
					ssh_b* obj((ssh_b*)(this) + offs);
					switch(sc->hash)
					{
						case _hash_string: *(String*)obj = _xml->attr<String>(h, sc->name, sc->def); break;
						case _hash_char: *(ssh_cs*)obj = *(ssh_cs*)_xml->attr<String>(h, sc->name, String(sc->def)).buffer(); break;
						case _hash_wchar: *(ssh_ws*)obj = *_xml->attr<String>(h, sc->name, String(sc->def)).buffer(); break;
						case _hash_float: *(float*)obj = _xml->attr<float>(h, sc->name, String(sc->def)); break;
						case _hash_double: *(double*)obj = _xml->attr<double>(h, sc->name, String(sc->def)); break;
						default:sval = _xml->attr<String>(h, sc->name, sc->def);
								switch(flg & 15)
								{
									case SC_ENUM:
									case SC_FLAGS: val = hlp->cnvValue(sval, sc->stk, String(sc->def)); break;
									case SC_BOOL: val = (sval.lower() == L"true"); break;
									default: val = sval.toNum<ssh_u>(0, (String::Radix)(flg & 3)); memcpy(obj, &val, sc->width); break;
								}
								break;
					}
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
			String sval;
			for(ssh_l i = 0; i < count; i++)
			{
				if(_ID)
				{
					if(!(flg & SC_OBJ) || sc->ID == -1) { if(sc->ID != -1) _sc--; return; }
					if(sc->ID != _ID) { _sc--; readBin(offs); if(i + 1 < count) _sc = sc + 1; }
				}
				else if((flg & SC_OBJ)) { _sc--; readBin(offs); if(i + 1 < count) _sc = sc + 1; }
				else if((flg & SC_NODE)) { Serialize* srlz((Serialize*)((ssh_b*)(this) + offs)); _sc = srlz->get_scheme(); srlz->readBin(0); _sc = sc + 1; }
				if((flg & SC_VAR))
				{
					ssh_b* obj((ssh_b*)(this) + offs);
					if(sc->hash == _hash_string) { *(String*)obj = (ssh_ws*)_buf; _buf += (wcslen((ssh_ws*)_buf) * 2 + 2); } else { memcpy(obj, _buf, sc->width); _buf += sc->width; }
				}
				offs += sc->width;
			}
		}
	}
}
