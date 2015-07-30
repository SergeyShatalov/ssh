
#include "stdafx.h"
#include "ssh_srlz.h"

namespace ssh
{
	Serialize::SCHEME* Serialize::_sc(nullptr);
	Xml* Serialize::_xml(nullptr);
	File* Serialize::_fl(nullptr);

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
		ssh_cs* _buf(buf);
		//readBin(&_buf, (ssh_b*)srlz - (ssh_b*)this);
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

	String Serialize::getVal(ssh_u flg, ssh_u offs, SCHEME* sc)
	{
		String sval;
		ssh_u val(0);
		ssh_b* obj((ssh_b*)(this) + offs);
		switch(sc->hash)
		{
			case _hash_string:
				sval = *(String*)obj;
				break;
			case _hash_char:
				sval = *(ssh_cs*)obj;
				break;
			case _hash_wchar:
				sval = *(ssh_ws*)obj;
				break;
			case _hash_wcs:
				sval = *(ssh_wcs*)obj;
				break;
			case _hash_ccs:
				sval = *(ssh_ccs*)obj;
				break;
			case _hash_float:
				sval = String(*(ssh_u*)obj, String::_flt);
				break;
			case _hash_double:
				sval = String(*(ssh_u*)obj, String::_dbl);
				break;
			default:
				memcpy(&val, obj, sc->width);
				if((flg & (SC_ENUM | SC_FLAGS))) sval = hlp->cnvString(val, sc->stk, sc->def, ((flg & SC_ENUM) == SC_ENUM));
				else if((flg & SC_BOOL)) sval = (val == 0 ? L"false" : L"true");
				else if((flg & SC_BIN)) sval = String(val, String::_bin);
				else sval = String(val, ((flg & SC_HEX) ? String::_hex : String::_dec));
				break;
		}
		return sval;
	}

	void Serialize::setVal(HXML h, ssh_u flg, ssh_u offs, SCHEME* sc)
	{
		String sval;
		ssh_u val;
		ssh_b* obj((ssh_b*)(this) + offs);
		switch(sc->hash)
		{
			case _hash_string:
				*(String*)obj = _xml->attr<String>(h, sc->name, sc->def);
				break;
			case _hash_char:
				//*(ssh_cs*)obj = _xml->attr<ssh_cs>(h, sc->name, String(sc->def));
				break;
			case _hash_wchar:
				// *(ssh_ws*)obj = _xml->attr<ssh_ws>(h, sc->name, String(sc->def));
				break;
			case _hash_float:
				*(float*)obj = _xml->attr<float>(h, sc->name, String(sc->def));
				break;
			case _hash_double:
				*(double*)obj = _xml->attr<double>(h, sc->name, String(sc->def));
				break;
			default:
				sval = _xml->attr<String>(h, sc->name, sc->def);
				if((flg & (SC_ENUM | SC_FLAGS))) val = hlp->cnvValue(sval, sc->stk, String(sc->def));
				else if((flg & SC_BOOL)) val = (sval == L"true");
				else if((flg & SC_BIN)) val = sval.toNum<ssh_u>(0, String::_bin);
				else val = sval.toNum<ssh_u>(0, ((flg & SC_HEX) ? String::_hex : String::_dec));
				memcpy(obj, &val, sc->width);
				break;
		}
	}

	void Serialize::writeVal(ssh_u flg, ssh_u offs, SCHEME* sc)
	{
		ssh_b* obj((ssh_b*)(this) + offs);
		if(sc->hash == _hash_string)
		{
			String sval(*(String*)obj);
			_fl ->write(sval.buffer(), sval.length() * 2 + 2);
		}
		else
			_fl->write(obj, sc->width);
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
			ssh_u offs(sc->offs + p_offs);
			String sval;
			for(ssh_u i = 0; i < sc->count; i++)
			{
				if(_ID)
				{
					// вложенный класс без своей схемы
					if(!(flg & SC_OBJ) || sc->ID == -1)
					{
						// выход из вложенного
						if(sc->ID != -1) _sc--;
						return;
					}
					if(sc->ID != _ID)
					{
						_sc--;
						// другой ID - вложенный в этот вложенный
						writeXml(h, offs);
					}
				}
				if((flg & SC_OBJ))
				{
					_sc--;
					writeXml(h, offs);
				}
				else if((flg & SC_NODE))
				{
					// вложенный со своей схемой
					Serialize* srlz((Serialize*)((ssh_b*)(this) + offs));
					SCHEME* __sc(_sc);
					_sc = srlz->get_scheme();
					srlz->writeXml(h, 0);
					_sc = __sc;
				}
				else
				{
					// простая переменная
					if(!sval.is_empty() && sc->count) sval += L',';
					sval += getVal(flg, offs, sc);
				}
				offs += sc->width;
			}
			_xml->set_attr(h, sc->name, sval);
		}
	}

	void Serialize::writeBin(ssh_u p_offs)
	{
		SCHEME* sc;
		// создать узел
		ssh_u _ID(_sc++->ID);
		while((sc = _sc++))
		{
			if(!sc->name) break;
			ssh_u flg(sc->flags);
			ssh_u offs(sc->offs + p_offs);
			String sval;
			if(_ID)
			{
				// вложенный класс без своей схемы
				if(!(flg & SC_OBJ) || sc->ID == -1)
				{
					// выход из вложенного
					if(sc->ID != -1) _sc--;
					return;
				}
				else if(sc->ID != _ID)
				{
					_sc--;
					// другой ID - вложенный в этот вложенный
					writeBin(offs);
					continue;
				}
			}
			else if((flg & SC_OBJ))
			{
				_sc--;
				writeBin(offs);
				continue;
			}
			else if((flg & SC_NODE))
			{
				// вложенный со своей схемой
				Serialize* srlz((Serialize*)((ssh_b*)(this) + offs));
				SCHEME* __sc(_sc);
				_sc = srlz->get_scheme();
				srlz->writeBin(0);
				_sc = __sc;
				continue;
			}
			else
			{
				// простая переменная
				for(ssh_u i = 0; i < sc->count; i++) writeVal(flg, offs, sc);
			}
		}
	}
	
	void Serialize::readXml(HXML hp, ssh_u p_offs)
	{
		SCHEME* sc;
		// создать узел
		HXML h(_xml->node(hp, _sc->name));
		ssh_u _ID(_sc++->ID);
		while((sc = _sc++))
		{
			if(!sc->name) break;
			ssh_u flg(sc->flags);
			ssh_u offs(sc->offs + p_offs);
			String sval;
			if(_ID)
			{
				// вложенный класс без своей схемы
				if(!(flg & SC_OBJ) || sc->ID == -1)
				{
					// выход из вложенного
					if(sc->ID != -1) _sc--;
					return;
				}
				else if(sc->ID != _ID)
				{
					_sc--;
					// другой ID - вложенный в этот вложенный
					readXml(h, offs);
					continue;
				}
			}
			else if((flg & SC_OBJ))
			{
				_sc--;
				readXml(h, offs);
				continue;
			}
			else if((flg & SC_NODE))
			{
				// вложенный со своей схемой
				Serialize* srlz((Serialize*)((ssh_b*)(this) + offs));
				SCHEME* __sc(_sc);
				_sc = srlz->get_scheme();
				srlz->readXml(h, 0);
				_sc = __sc;
				continue;
			}
			else
			{
				// массив фиксированной длины
				for(ssh_u i = 0; i < sc->count; i++)
				{
					setVal(h, flg, offs, sc);
					offs += sc->width;
				}
			}
		}
	}

	void Serialize::readBin(ssh_u p_offs)
	{
		/*
		SCHEME* sc;
		// создать узел
		ssh_u _ID(_sc++->ID);
		while((sc = _sc++))
		{
			if(!sc->name) break;
			ssh_u flg(sc->flags);
			ssh_u offs(sc->offs + p_offs);
			String sval;
			if(_ID)
			{
				// вложенный класс без своей схемы
				if(!(flg & SC_OBJ) || sc->ID == -1)
				{
					// выход из вложенного
					if(sc->ID != -1) _sc--;
					return;
				}
				else if(sc->ID != _ID)
				{
					_sc--;
					// другой ID - вложенный в этот вложенный
					//readBin(xml, h, offs);
					continue;
				}
			}
			else if((flg & SC_OBJ))
			{
				_sc--;
				//readBin(xml, h, offs);
				continue;
			}
			else if((flg & SC_NODE))
			{
				// вложенный со своей схемой
				Serialize* srlz((Serialize*)((ssh_b*)(this) + offs));
				SCHEME* __sc(_sc);
				_sc = srlz->get_scheme();
				//srlz->readBin(h, 0);
				_sc = __sc;
				continue;
			}
			else if((flg & SC_ARRAY))
			{
				// массив фиксированной длины
				for(ssh_u i = 0; i < sc->count - 1; i++)
				{
					//setVal(buf, flg, offs, sc);
					offs += sc->width;
				}
			}
			// простая переменная
			//setVal(buf, flg, offs, sc);
		}
		*/
	}
}

/*
ssh_u hash1;
int i(0);
unsigned int ui(0);
long l(0);
unsigned long ul(0);
char c(0);
unsigned char uc(0);
wchar_t w(0);
short s(0);
unsigned short us(0);
ssh_l ll(0);
ssh_u ull(0);
String str(L"");
ssh_wcs wcs;
ssh_ccs ccs;
float f(100.0);
double d(0);
Half h;
Time t;

hash1 = (typeid(t).hash_code());
hash1 = (typeid(i).hash_code());
hash1 = (typeid(ui).hash_code());
hash1 = (typeid(l).hash_code());
hash1 = (typeid(ul).hash_code());
hash1 = (typeid(c).hash_code());
hash1 = (typeid(uc).hash_code());
hash1 = (typeid(w).hash_code());
hash1 = (typeid(s).hash_code());
hash1 = (typeid(us).hash_code());
hash1 = (typeid(ll).hash_code());
hash1 = (typeid(ull).hash_code());
hash1 = (typeid(str).hash_code());
hash1 = (typeid(wcs).hash_code());
hash1 = (typeid(ccs).hash_code());
hash1 = (typeid(f).hash_code());
hash1 = (typeid(d).hash_code());
hash1 = (typeid(h).hash_code());
*/