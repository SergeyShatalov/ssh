
#include "stdafx.h"
#include "ssh_xml.h"

namespace ssh
{
	ssh_ws* Xml::_xml(nullptr);

	ssh_w Xml::bom_coder() const
	{
		SSH_TRACE;
		if(code == L"utf-16le") return 0xfeff;
		else if(code == L"utf-16be") return 0xfffe;
		return 0;
	}

	void Xml::init()
	{
		SSH_TRACE;
		tree.reset();
		XmlNode* n(new XmlNode(L"?xml", L""));
		n->attrs.add(new XmlNode(L"version", L"1.0"));
		n->attrs.add(new XmlNode(L"encoding", code));
		n->attrs.add(new XmlNode(L"?", L""));
		tree.add(tree.get_root(), n);
	}

	Xml::Xml(const Buffer<ssh_cs>& buf, ssh_wcs cod)
	{
		try
		{
			SSH_TRACE;
			code = cod;
			_make(buf);
		}
		catch(const Exception& e) { e.add(L"Парсер XML!"); }
	}

	void Xml::open(const String& path, ssh_wcs cod)
	{
		try
		{
			SSH_TRACE;
			code = cod;
			// 1. открываем файл
			File f(path, File::open_read);
			// 2. загружаем, декодируем и строим дерево
			_make(f.read<ssh_cs>());
		}
		catch(const Exception& e) { e.add(L"Парсер XML <%s>!", path); }
	}

	String Xml::encode(const Buffer<ssh_cs>& buf)
	{
		SSH_TRACE;
		String ret, charset;
		regx rx(LR"((?im)<\?xml\s+version=.+encoding=["]?(.*?)["]?\s*\?>)");
		// проверить на BOM
		ssh_b _0(buf[0]), _1(buf[1]), _2(buf[2]);
		ssh_l pos, pos1;
		bool bom16be(_0 == 0xfe && _1 == 0xff);
		bool bom16le(_0 == 0xff && _1 == 0xfe);
		if((bom16le || bom16be))
		{
			if(rx.match(buf.to<ssh_ws>()) > 0)
			{
				// utf-16le или utf16-be
				charset = rx.substr(1);
				charset.lower();
				if(((charset == L"utf-16be") && bom16be) || ((charset == L"utf-16le") && bom16le)) pos1 = rx.vec(0, 1); else charset.empty();
			}
		}
		else
		{
			// utf-8 или какая то однобайтовая
			// <?xml .... ?>
			// определить границы заголовка xml
			if((pos = strstr(buf, "<?xml") - buf) < 0) SSH_THROW(L"");
			if((pos1 = (strstr(buf + pos, "?>") - (buf + pos))) < 0) SSH_THROW(L"");
			pos1 += 2;
			ssh_cs _cs(buf[pos1]);
			buf[pos1] = 0;
			// вытащить заголовок и преобразовать его в utf-16le
			String caption(ssh_cnv(L"utf-8", buf, pos));
			buf[pos1] = _cs;
			// вытащить реальную кодировку
			if(rx.match(caption) > 0)
			{
				bool bom8(_0 == 0xef && _1 == 0xbb && _2 == 0xbf);
				charset = rx.substr(1);
				charset.lower();
				if(bom8 && charset != L"utf-8") charset.empty();
			}
		}
		if(charset.is_empty()) SSH_THROW(L"Неизвестная кодирока.");
		return ssh_cnv(charset, buf, pos1);
	}

	void Xml::_make(const Buffer<ssh_cs>& buf)
	{
		SSH_TRACE;
		code.lower();
		init();
		String tmp(encode(buf));
		_xml = tmp.buffer();
		regx rx;
		// тег
		rx.set_pattern(0, LR"serg((?mUs)<(?:(?:([/]{0,1})([\w_]+[\w\d_-]*)>)|(!--.*-->)|(?:(\w+[\w\d_-]*)\s+(\w+.*)([/]{0,1})>)))serg");
		// атрибуты
		rx.set_pattern(1, LR"serg((?sm)([\w_]+[\w\d_-]*)\s*=\s*(?:"(.*?)")\s*)serg");
		// значение тега
		rx.set_pattern(2, LR"serg((?ms)()?<=>"(.*?)")serg");
		// формирование
		make(&rx, root(), 0);
	}

	void Xml::make(regx* rx, HXML hp, ssh_u lev)
	{
		while(*_xml)
		{
			if(rx->match(_xml, (ssh_u)0) > 0)
			{
				bool is_child(false);
				HXML h;
				ssh_ws* _x(_xml);
				_xml += rx->vec(0, 1);
				// это комментырий?
				if(rx->len(3)) continue;
				// это завершающий тег?
				if(rx->len(1))
				{
					// </tag>
					_x[rx->vec(2, 1)] = 0;
					if(get_name(hp) != (_x + rx->vec(2, 0))) SSH_THROW(L"");
					return;
				}
				else if(rx->len(2))
				{
					// <tag>
					is_child = true;
					_x[rx->vec(2, 1)] = 0;
					h = add_node(hp, _x + rx->vec(2, 0), L"");
					_x[rx->vec(2, 1)] = L'>';
				}
				else
				{
					// это стартовый тег
					is_child = (rx->len(6) == 0);
					_x[rx->vec(4, 1)] = 0;
					h = add_node(hp, _x + rx->vec(4, 0), L"");
					// атрибуты
					_x[rx->vec(5, 1)] = 0;
					ssh_ws* _ws(_x + rx->vec(5, 0));
					while(rx->match(_ws, (ssh_u)1) == 3)
					{
						// проверка на пространство между атрибутами
						if(rx->vec(0, 0)) SSH_THROW(L"Недопустимо заданы атрибуты тега<%s>.", h->value->nm);
						// добавить атрибут
						_ws[rx->vec(2, 1)] = 0;
						_ws[rx->vec(1, 1)] = 0;
						set_attr(h, _ws + rx->vec(1, 0), _ws + rx->vec(2, 0));
						_ws += rx->len(0);
					}
					// проверить после найденных атрибутов больше ничего нет?
					if(*_ws != L'\0') SSH_THROW(L"");
				}
				if(is_child)
				{
					ssh_ws ws;
					bool is(false);
					// проверим на значение тега
					while(ws = *_xml++)
					{
						if(ws == L'\"')
						{
							if(is) break;
							_x = _xml;
							is = true;
						}
						else if(ws == L'\\') _xml++;
						else if(ws == L'<' && !is) { _xml--; break; }
					}
					if(ws == 0) SSH_THROW(L"Неожиданный конец XML!");
					if(is)
					{
						*(_xml - 1) = 0;
						set_val(h, _x);
					}
					make(rx, h, lev + 1);
				}
			}
			else
			{
				while(*_xml <= L' ' && *_xml != 0) _xml++;
				if(*_xml != 0) SSH_THROW(L"Найдена неизвестная лексема!");
			}
		}
		if(lev) SSH_THROW(L"Неожиданный конец XML!");
	}

	void Xml::save(const String& path)
	{
		SSH_TRACE;
		String txt(_save(tree.get_root(), 0));
		File f(path, File::create_write);
		ssh_w bom(bom_coder());
		if(bom) f.write(&bom, 2);
		f.write(txt, code);
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
SSH_THROW(L"Слово <%s> задано недопустимо.", _ws);
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
if(get_name(hp) != (_nm = _word(1))) SSH_THROW(L"Начальный <%s> и завершающий <%s> теги не совпадают.", get_name(hp), _nm);
return;
}
else if(*_xml == L'!')
{
// может комментарий?
if(*(ssh_d*)++_xml != 0x002d002d) SSH_THROW(L"Недопустимо задан тег <!--.");
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
// конец объявления тега?
if(_ws == L'>' || _null_ws == L'>')
{
if(_null_ws != L'>') _xml++;
make(h, _lev + 1);
break;
}
// завершение объявления тега?
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
else if(_null_ws != L'=') SSH_THROW(L"Недопустимо задан атрибут <%s> тега <%s>, ожидается \'=\'.", _nm, get_name(h));
_skip_spc();
set_attr(h, _nm, _word());
}
}
if(!_ws) SSH_THROW(L"Тег <%s> не был завершен.", get_name(h));
}
}
else SSH_THROW(L"Неизвестная лексема \'%c\'.", _ws);
}
if(_lev > 0) SSH_THROW(L"Неожиданный конец XML!");
}

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
// 2. проходя по все дескам, выполнять операции
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
default: OSTD_LOG("Обнаружен узел/атрибут <%s> с поддерживаемым типом данных <%I64X> для сериализации!", desc->name, desc->hash); break;
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
// 2. проходя по все дескам, выполнять операции
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
default: OSTD_LOG("Обнаружен узел/атрибут <%s> с поддерживаемым типом данных <%I64X> для сериализации!", desc->name, desc->hash); break;
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
default: OSTD_LOG("Обнаружен узел/атрибут <%s> с поддерживаемым типом данных <%I64X> для сериализации!", desc->name, desc->hash); break;
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
default: OSTD_LOG("Обнаружен узел/атрибут <%s> с поддерживаемым типом данных <%I64X> для сериализации!", desc->name, desc->hash); break;
}
f->write(obj, count * len_data);
if(!--nComplex) offsComplex = 0;
}
}
}
}
}
*/