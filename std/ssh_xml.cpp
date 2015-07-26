
#include "stdafx.h"
#include "ssh_xml.h"

namespace ssh
{
	ssh_ws* Xml::_xml(nullptr);
	static ssh_ws _null_ws;
	static ssh_ws* _exml;

	ssh_wcs Xml::get_param_coder(ssh_w& bom)
	{
		SSH_TRACE;
		ssh_wcs _s;
		bom = 0;
		switch(coder)
		{
			default:
				_s = cp_ansi;
				break;
			case Xml::_utf8:
				_s = L"utf-8";
				break;
			case Xml::_utf16le:
				bom = 0xfeff;
				_s = cp_utf;
				break;
			case Xml::_utf16be:
				bom = 0xfffe;
				_s = L"utf-16be";
				break;
		}
		return _s;
	}

	void Xml::init()
	{
		SSH_TRACE;
		tree.reset();
		ssh_w bom;
		XmlNode* n(new XmlNode(L"?xml", L""));
		n->attrs.add(new XmlNode(L"version", L"1.0"));
		n->attrs.add(new XmlNode(L"encoding", get_param_coder(bom)));
		n->attrs.add(new XmlNode(L"?", L""));
		tree.add(tree.get_root(), n);
	}

	Xml::Xml(const Buffer<ssh_cs>& buf, Cod cod)
	{
		try
		{
			SSH_TRACE;
			coder = cod;
			init();
			String tmp(encode(buf));
			_xml = tmp.buffer();
			make(root(), 0);
		}
		catch(const Exception& e) { e.add(L"ѕарсер XML!"); }
	}

	void Xml::open(const String& path, Cod cod)
	{
		try
		{
			SSH_TRACE;
			coder = cod;
			init();
			// 1. открываем файл
			File f(path, File::open_read);
			// 2. загружаем, декодируем и строим дерево
			String tmp(encode(f.read<ssh_cs>()));
			_xml = tmp.buffer();
			make(root(), 0);
		}
		catch(const Exception& e) { e.add(L"ѕарсер XML <%s>!", path); }
	}

	String Xml::encode(const Buffer<ssh_cs>& buf)
	{
		SSH_TRACE;
		String ret;
		regx rx(LR"((?im)<\?xml\s+version=.+encoding=["]?(utf-\d\d(?:le|be))["]?\s*\?>)");
		// проверить на BOM
		ssh_b _0(buf[0]), _1(buf[1]), _2(buf[2]);
		bool bom16be(_0 == 0xfe && _1 == 0xff);
		bool bom16le(_0 == 0xff && _1 == 0xfe);
		bool bom8(_0 == 0xef && _1 == 0xbb && _2 == 0xbf);
		if(bom8 || (strstr(buf, "encoding=\"utf-8\"") || strstr(buf, "encoding=\"UTF-8\""))) ret = ssh_cnv(L"utf-8", buf, 3 * bom8);
		else if(rx.match(buf.to<ssh_ws>()) > 0)
		{
			bool is(false);
			String charset(rx.substr(1).lower());
			if((charset == L"utf-16be") && bom16be) { is = true; }
			else if((charset == L"utf-16le") && bom16le) { is = true; }
			if(is) ret = ssh_cnv(charset, buf, 2);
		}
		if(ret.is_empty()) ret = ssh_cnv(cp_ansi, buf, 0);
		return ret.substr(ret.find(L'\n', ret.find(L"<?xml")) + 1);
	}

	void Xml::_skip_spc()
	{
		while((*_xml <= L' ' && *_xml != 0)) { _xml++; }
	}

	static ssh_ws xml_chars[] =	L"zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz"
								L"szkzzzzzzzzzzzzzddddddddddzzzssz"
								L"zwwwwwwwwwwwwwwwwwwwwwwwwwwzzzzw"
								L"zwwwwwwwwwwwwwwwwwwwwwwwwwwzzzzz";
	
	ssh_ws* Xml::_word(int sub)
	{
		ssh_ws _w;
		bool is_s;
		_xml += sub;
		if((is_s = (*_xml == L'\"'))) _xml++;
		ssh_ws* _ws(_xml);
		_null_ws = 0;
		while((_w = *_xml++))
		{
			ssh_ws _ww(_w < 128 ? xml_chars[_w] : L'w');
			if((is_s && _ww != L'k') || (!is_s && (_ww == L'w' || (_ww == L'd' && (_ws != (_xml - 1)))))) continue;
			if(_ww == L's' || (is_s && _ww == L'k'))
			{
				_null_ws = _w;
				*(_xml - 1) = 0;
				_skip_spc();
				return _ws;
			}
			break;
		}
		*_xml = 0;
		SSH_THROW(L"—лово <%s> задано недопустимо.", _ws);
	}

	void Xml::make(HXML hp, ssh_u _lev)
	{
		ssh_ws _ws;
		ssh_ws* _nm;
		HXML h;
		_skip_spc();
		while((_ws = *_xml++))
		{
			if(_ws == L'\"')
			{
				// возможно значение тега?
				set_val(hp, _word(-1));
			}
			else if(_ws == L'<')
			{
				// может конец тега?
				if(*_xml == L'/')
				{
					if(get_name(hp) != (_nm = _word(1))) SSH_THROW(L"Ќачальный <%s> и завершающий <%s> теги не совпадают.", get_name(hp), _nm);
					return;
				}
				else if(*_xml == L'!')
				{
					// может комментарий?
					if(*(ssh_d*)++_xml != 0x002d002d) SSH_THROW(L"Ќедопустимо задан тег <!--.");
					_xml += 2;
					// пропустить все до -->
					bool is(false);
					while((_ws = *_xml))
					{
						if(*(ssh_d*)_xml == 0x002d002d) { is = true; _xml += 2; }
						else if(is && _ws == L'>') { _xml++; break; }
						else { is = false; _xml++; }
					}
					_skip_spc();
				}
				else
				{
					// новый тег
					h = add_node(hp, _word(), nullptr);
					while((_ws = *_xml))
					{
						// конец объ€влени€ тега?
						if(_ws == L'>' || _null_ws == L'>')
						{
							if(_null_ws != L'>') _xml++;
							make(h, _lev + 1);
							break;
						}
						// завершение объ€влени€ тега?
						else if(*(ssh_d*)_xml == 0x003e002f)
						{
							_xml += 2;
							_skip_spc();
							break;
						}
						else
						{
							// атрибут
							_nm = _word();
							if(*_xml == L'=') _xml++;
							else if(_null_ws != L'=') SSH_THROW(L"Ќедопустимо задан атрибут <%s> тега <%s>, ожидаетс€ \'=\'.", _nm, get_name(h));
							_skip_spc();
							set_attr(h, _nm, _word());
						}
					}
					if(!_ws) SSH_THROW(L"“ег <%s> не был завершен.", get_name(h));
				}
			}
			else SSH_THROW(L"Ќеизвестна€ лексема \'%c\'.", _ws);
		}
		if(_lev > 0) SSH_THROW(L"Ќеожиданный конец XML!");
	}

	void Xml::save(const String& path)
	{
		SSH_TRACE;
		ssh_w bom(0xfeff);
		String txt(_save(tree.get_root(), 0));
		File f(path, File::create_write);
		ssh_wcs _ws(get_param_coder(bom));
		if(bom) f.write(&bom, 2);
		f.write(txt, _ws);
	}

	String Xml::_save(HXML h, ssh_l level)
	{
		auto n(h->value);
		String s(L'\t', level < 2 ? 0 : level - 1);
		String str(s + L"<" + n->nm);
		// атрибуты узла
		auto a(n->attrs.root());
		while(a = n->attrs.next()) { auto v(a->value); str += L" " + ((h == tree.get_root() && v->nm == L"?") ? v->nm : v->nm + L"=\"" + v->val + L"\""); }
		// начать обработку дочерних узлов
		auto ch(h->fchild);
		bool is_child(ch != 0);
		if(is_child)
		{
			str += L">\r\n";
			do { str += _save(ch, level + 1); } while(ch = ch->next);
		}
		// завершить обработку дочерних узлов
		if(!is_child && n->val.is_empty()) str += L"/>\r\n";
		else
		{
			if(!n->val.is_empty()) str += L">\"" + n->val + L"\"";
			if(is_child) str += s;
			if(h != tree.get_root()) str = str + L"</" + n->name() + L">\r\n";
		}
		return str;
	}
}
/*

namespace ostd
{
base_scheme::SCHEME_DESC Serialize::desc = {"", 0, 1, nullptr, "", 0, 0};

void Serialize::writeXml(Xml* xml, HXML h, base_scheme::SCHEME_DESC* parent)
{
base_scheme* scheme(getScheme());
for(uint_t i = 0; i < parent->count; i++)
{
// 1. добавить узел
HXML hn(xml->node(h, parent->name, "")), hc;
uint_t nComplex(0), offsComplex(0);
for(uint_t j = 0; j < scheme->descs.size(); j++)
{
// 2. проход€ по все дескам, выполн€ть операции
base_scheme::SCHEME_DESC* desc(&scheme->descs[j]);
uint_t flags(desc->flags);
if(flags & base_scheme::F_SPEC)
{
specWriteXml(xml, hn, j, desc->hash);
}
else if(flags & base_scheme::F_COMPLEX)
{
nComplex = desc->count;
offsComplex = desc->offs;
hc = xml->node(hn, desc->name, "");
}
else if(flags & base_scheme::F_NODE)
{
// если составной, то...
Serialize* obj((Serialize*)((BYTE*)(this) + desc->offs));
if(obj) obj->writeXml(xml, hn, desc);
}
else
{
bool is_hex((flags & base_scheme::F_HEX) != 0);
bool is_flag((flags & base_scheme::F_FLAG) == 0);
bool is_bool((flags & base_scheme::F_BOOL) != 0);
uint_t count(desc->count);
uint_t* arr_content(nullptr);
StringEx result;
void* obj((BYTE*)(this) + desc->offs + offsComplex);
switch(desc->hash)
{
case TYPEID_LONG:
case TYPEID_ULONG:
case TYPEID_INT:
case TYPEID_UINT:
case TYPEID_DWORD: arr_content = copyToUINT_T<long>((long*)obj, count); break;
case TYPEID_BYTE:
case TYPEID_CHAR: arr_content = copyToUINT_T((BYTE*)obj, count); break;
case TYPEID_WORD: arr_content = copyToUINT_T((WORD*)obj, count); break;
case TYPEID_UINT_T:
case TYPEID_INT_T: arr_content = copyToUINT_T((uint_t*)obj, count); break;
case TYPEID_STRING: result = copyToString<String>((String*)obj, count); break;
case TYPEID_WCHAR_T:
case TYPEID_BSTR: result = copyToString<BSTR>((BSTR*)obj, count); break;
case TYPEID_DOUBLE: result = copyToString<double>((double*)obj, count); break;
case TYPEID_FLOAT: result = copyToString<float>((float*)obj, count); break;
default: OSTD_LOG("ќбнаружен узел/атрибут <%s> с поддерживаемым типом данных <%I64X> дл€ сериализации!", desc->name, desc->hash); break;
}
if(arr_content)
{
result = (count > 1 ? hlp->implodeEx(",", arr_content, count, desc->stk, desc->def_val, is_bool, is_hex, is_flag) :
(result.is_empty() ? (desc->stk ? hlp->cnvString(*arr_content, desc->stk, desc->def_val, is_flag) :
(is_hex ? (StringEx)Hex<uint_t>(*arr_content) : *arr_content)) : result));
}
(flags & base_scheme::F_VAL) ? xml->val(offsComplex ? hc : hn, result, true) : xml->attr(offsComplex ? hc : hn, desc->name, result, true);
if(!--nComplex) offsComplex = 0;
}
}
}
}

void Serialize::readXml(Xml* xml, HXML h, base_scheme::SCHEME_DESC* parent)
{
base_scheme* scheme(getScheme());
for(uint_t i = 0; i < parent->count; i++)
{
// 1. получить узел
HXML hn(xml->node(h, parent->name, i)), hc;
uint_t nComplex(0), offsComplex(0);
for(uint_t j = 0; j < scheme->descs.size(); j++)
{
// 2. проход€ по все дескам, выполн€ть операции
base_scheme::SCHEME_DESC* desc(&scheme->descs[j]);
uint_t flags(desc->flags);
if(flags & base_scheme::F_SPEC)
{
specReadXml(xml, hn, j, desc->hash);
}
else if(flags & base_scheme::F_COMPLEX)
{
nComplex = desc->count;
offsComplex = desc->offs;
hc = xml->node(hn, desc->name);
}
else if(flags & base_scheme::F_NODE)
{
// если составной, то...
Serialize* obj((Serialize*)((BYTE*)(this) + desc->offs));
if(obj) obj->readXml(xml, hn, desc);
}
else
{
bool is_hex((flags & base_scheme::F_HEX) != 0);
bool is_flag((flags & base_scheme::F_FLAG) == 0);
bool is_bool((flags & base_scheme::F_BOOL) != 0);
uint_t count(desc->count);
void* obj((BYTE*)(this) + desc->offs + offsComplex);
// получаем значение узла или атрибута
StringEx result((flags & base_scheme::F_VAL) ? xml->val(offsComplex ? hc : hn, desc->def_val) : xml->attr(offsComplex ? hc : hn, desc->name, desc->def_val));
switch(desc->hash)
{
case TYPEID_LONG:
case TYPEID_ULONG:
case TYPEID_INT:
case TYPEID_UINT:
case TYPEID_DWORD: hlp->explode<long>(",", result, (long*)obj, count, StringEx(desc->def_val), desc->stk, is_hex); break;
case TYPEID_BYTE:
case TYPEID_CHAR: hlp->explode<BYTE>(",", result, (BYTE*)obj, count, StringEx(desc->def_val), desc->stk, is_hex); break;
case TYPEID_WORD: hlp->explode<WORD>(",", result, (WORD*)obj, count, StringEx(desc->def_val), desc->stk, is_hex); break;
case TYPEID_UINT_T:
case TYPEID_INT_T: hlp->explode<uint_t>(",", result, (uint_t*)obj, count, StringEx(desc->def_val), desc->stk, is_hex); break;
case TYPEID_STRING: hlp->split(",", result, (String*)obj, count, StringEx(desc->def_val)); break;
case TYPEID_WCHAR_T:
case TYPEID_BSTR: break;
case TYPEID_DOUBLE: hlp->explode<double>(",", result, (double*)obj, count, StringEx(desc->def_val), nullptr); break;
case TYPEID_FLOAT: hlp->explode<float>(",", result, (float*)obj, count, StringEx(desc->def_val), nullptr); break;
default: OSTD_LOG("ќбнаружен узел/атрибут <%s> с поддерживаемым типом данных <%I64X> дл€ сериализации!", desc->name, desc->hash); break;
}
if(!--nComplex) offsComplex = 0;
}
}
}
}

void Serialize::readBin(BYTE* ptr, base_scheme::SCHEME_DESC* parent)
{
base_scheme* scheme(getScheme());
for(uint_t i = 0; i < parent->count; i++)
{
uint_t nComplex(0), offsComplex(0);
for(uint_t j = 0; j < scheme->descs.size(); j++)
{
base_scheme::SCHEME_DESC* desc(&scheme->descs[j]);
uint_t flags(desc->flags);
if(flags & base_scheme::F_SPEC)
{
specReadBin(ptr, j, desc->hash);
}
else if(flags & base_scheme::F_COMPLEX)
{
nComplex = desc->count;
offsComplex = desc->offs;
}
else if(flags & base_scheme::F_NODE)
{
// если составной, то...
Serialize* obj((Serialize*)((BYTE*)(this) + desc->offs));
if(obj) obj->readBin(ptr, desc);
}
else
{
bool is_hex((flags & base_scheme::F_HEX) != 0);
bool is_flag((flags & base_scheme::F_FLAG) == 0);
bool is_bool((flags & base_scheme::F_BOOL) != 0);
uint_t count(desc->count), len_data(0);
void* obj((BYTE*)(this) + desc->offs + offsComplex);
// получаем значение узла или атрибута
switch(desc->hash)
{
case TYPEID_LONG:
case TYPEID_ULONG:
case TYPEID_INT:
case TYPEID_UINT:
case TYPEID_DWORD: len_data = sizeof(long); break;
case TYPEID_BYTE:
case TYPEID_CHAR: len_data = sizeof(BYTE); break;
case TYPEID_WORD: len_data = sizeof(WORD); break;
case TYPEID_UINT_T:
case TYPEID_INT_T: len_data = sizeof(uint_t); break;
case TYPEID_STRING:
{
String* _obj((String*)obj);
for(uint_t i = 0; i < count; i++) {_obj[i] = (PCC)ptr; ptr += (_obj[i].length() + 1);}
}
break;
case TYPEID_WCHAR_T:
case TYPEID_BSTR: break;
case TYPEID_DOUBLE: len_data = sizeof(double); break;
case TYPEID_FLOAT: len_data = sizeof(float); break;
default: OSTD_LOG("ќбнаружен узел/атрибут <%s> с поддерживаемым типом данных <%I64X> дл€ сериализации!", desc->name, desc->hash); break;
}
memcpy(obj, ptr, count * len_data);
ptr += (count * len_data);
if(!--nComplex) offsComplex = 0;
}
}
}
}
void Serialize::writeBin(File* f, base_scheme::SCHEME_DESC* parent)
{
base_scheme* scheme(getScheme());
for(uint_t i = 0; i < parent->count; i++)
{
uint_t nComplex(0), offsComplex(0);
for(uint_t j = 0; j < scheme->descs.size(); j++)
{
base_scheme::SCHEME_DESC* desc(&scheme->descs[j]);
uint_t flags(desc->flags);
if(flags & base_scheme::F_SPEC)
{
specWriteBin(f, j, desc->hash);
}
else if(flags & base_scheme::F_COMPLEX)
{
nComplex = desc->count;
offsComplex = desc->offs;
}
else if(flags & base_scheme::F_NODE)
{
// если составной, то...
Serialize* obj((Serialize*)((BYTE*)(this) + desc->offs));
if(obj) obj->writeBin(f, desc);
}
else
{
bool is_hex((flags & base_scheme::F_HEX) != 0);
bool is_flag((flags & base_scheme::F_FLAG) == 0);
bool is_bool((flags & base_scheme::F_BOOL) != 0);
uint_t count(desc->count), len_data(0);
void* obj((BYTE*)(this) + desc->offs + offsComplex);
// получаем значение узла или атрибута
switch(desc->hash)
{
case TYPEID_LONG:
case TYPEID_ULONG:
case TYPEID_INT:
case TYPEID_UINT:
case TYPEID_DWORD: len_data = sizeof(long); break;
case TYPEID_BYTE:
case TYPEID_CHAR: len_data = sizeof(BYTE); break;
case TYPEID_WORD: len_data = sizeof(WORD); break;
case TYPEID_UINT_T:
case TYPEID_INT_T: len_data = sizeof(uint_t); break;
case TYPEID_STRING:
{
String* _obj((String*)obj);
for(uint_t i = 0; i < count; i++) f->write((void*)(PCC)_obj[i], _obj[i].length() + 1);
}
break;
case TYPEID_WCHAR_T:
case TYPEID_BSTR: break;
case TYPEID_DOUBLE: len_data = sizeof(double); break;
case TYPEID_FLOAT: len_data = sizeof(float); break;
default: OSTD_LOG("ќбнаружен узел/атрибут <%s> с поддерживаемым типом данных <%I64X> дл€ сериализации!", desc->name, desc->hash); break;
}
f->write(obj, count * len_data);
if(!--nComplex) offsComplex = 0;
}
}
}
}
}
*/