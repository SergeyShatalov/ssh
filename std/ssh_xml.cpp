
#include "stdafx.h"
#include "ssh_xml.h"

namespace ssh
{
	ssh_ws* Xml::_xml(nullptr);

	Xml::Xml(const Buffer<ssh_cs>& buf)
	{
		try
		{
			_make(buf);
		}
		catch(const Exception& e) { e.add(L"ѕарсер XML!"); }
	}

	void Xml::open(const String& path)
	{
		try
		{
			SSH_TRACE;
			close();
			// 1. открываем файл
			File f(path, File::open_read);
			// 2. загружаем, декодируем и строим дерево
			_make(f.read<ssh_cs>());
		}
		catch(const Exception& e) { e.add(L"ѕарсер XML <%s>!", path); }
	}

	String Xml::encode(const Buffer<ssh_cs>& buf)
	{
		SSH_TRACE;
		String ret, charset, caption;
		regx* rx(get_regx());
		// проверить на BOM
		ssh_b _0(buf[0]), _1(buf[1]), _2(buf[2]);
		ssh_l pos;
		bool bom16be(_0 == 0xfe && _1 == 0xff);
		bool bom16le(_0 == 0xff && _1 == 0xfe);
		bool bom8(_0 == 0xef && _1 == 0xbb && _2 == 0xbf);
		int width((bom16le || bom16be) + 1);
		// определить границы заголовка xml
		if((pos = (width == 1 ? (strstr(buf, "?>") - buf) : (wcsstr(buf.to<ssh_ws>(), L"?>") - buf.to<ssh_ws>()))) < 0) SSH_THROW(L"Ќе удалось найти заголовок XML!");
		pos += 2;
		ssh_cs _cs(buf[pos * width]);
		buf[pos * width] = 0;
		caption = (width == 1 ? ssh_cnv(L"utf-8", buf, bom8 * 3) : buf.to<ssh_ws>() + 1);
		buf[pos * width] = _cs;
		if(rx->match(caption, (ssh_u)3) > 0)
		{
			charset = rx->substr(1);
			charset.lower();
			if((bom16le && charset != L"utf-16le") || (bom16be && charset != L"utf-16be") || (bom8 && charset != L"utf-8")) charset.empty();
		}
		if(charset.is_empty()) SSH_THROW(L"Ќеизвестна€ кодирока.");
		return ssh_cnv(charset, buf, pos * width);
	}

	void Xml::_make(const Buffer<ssh_cs>& buf)
	{
		SSH_TRACE;
		tree.reset();
		String tmp(encode(buf));
		_xml = tmp.buffer();
		// формирование
		make(root(), 0);
	}

	void Xml::make(HXML hp, ssh_u lev)
	{
		while(*_xml)
		{
			regx* rx(get_regx());
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
						if(rx->vec(0, 0)) SSH_THROW(L"Ќедопустимо заданы атрибуты тега<%s>.", h->value->nm);
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
					if(ws == 0) SSH_THROW(L"Ќеожиданный конец XML!");
					if(is)
					{
						*(_xml - 1) = 0;
						set_val(h, _x);
					}
					make(h, lev + 1);
				}
			}
			else
			{
				while(*_xml <= L' ' && *_xml != 0) _xml++;
				if(*_xml != 0) SSH_THROW(L"Ќайдена неизвестна€ лексема!");
			}
		}
		if(lev) SSH_THROW(L"Ќеожиданный конец XML!");
	}

	void Xml::save(const String& path, ssh_wcs code)
	{
		SSH_TRACE;
		String txt;
		txt.fmt(L"<?xml version=\"1.0\" encoding=\"%s\" ?>\r\n", code);
		txt += _save(tree.get_root(), 0);
		File f(path, File::create_write);
		ssh_u bom(0);
		if(code == L"utf-16le") bom = 0xfeff;
		else if(code == L"utf-16be") bom = 0xfffe;
		if(bom) f.write(&bom, 2);
		f.write(txt, code);
	}

	String Xml::_save(HXML h, ssh_l level)
	{
		auto n(h->value);
		String s(L'\t', level);
		String str(s + L"<" + n->nm);
		// атрибуты узла
		auto a(n->attrs);
		while(a) { str += L" " + a->nm + L"=\"" + a->val + L"\""; a = a->next; }
		// значение узла
		auto ch(h->fchild);
		bool is_child(ch != 0);
		bool is_val(!n->val.is_empty());
		if(is_val) str += L">\"" + n->val + L"\"";
		else if(!is_child) str += L"/>\r\n";
		// начать обработку дочерних узлов
		if(is_child)
		{
			if(!is_val) str += L'>';
			str += "\r\n";
			do { str += _save(ch, level + 1); } while(ch = ch->next);
		}
		if((is_val || is_child))
		{
			if(is_child) str += s;
			str += L"</" + n->nm + L">\r\n";
		}
		return str;
	}
}
