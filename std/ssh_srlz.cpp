
#include "stdafx.h"
#include "ssh_srlz.h"

namespace ssh
{
	String Serialize::getVal(ssh_u flg, ssh_u offs, SCHEME* sc)
	{
		String sval;
		if(sc->type == 0) sval = *(String*)(this + offs);
		else if(sc->width <= 8)
		{
			ssh_u val(0);
			String sval;
			memcpy(&val, (this + offs), sc->width);
			if((flg & (SC_ENUM | SC_FLAGS)))
			{
				sval = hlp->cnvString(val, sc->stk, sc->def, ((flg & SC_ENUM) == SC_ENUM));
			}
			else
			{
				if((flg & SC_T_BOOL))
				{
					sval = (val == 0 ? L"false" : L"true");
				}
				else
				{
					sval = String(val, ((flg & SC_T_HEX) ? String::_hex : String::_dec));
				}
			}
		}
		else SSH_THROW(L"Ќеизветный тип данных!");
		return sval;
	}

	Serialize::SCHEME* Serialize::readXml(Xml* xml, HXML hp, SCHEME* arr, ssh_u p_offs)
	{
		/*
		SCHEME* sc;
		// создать узел
		HXML h(xml->add_node(hp, arr->name, L""));
		ssh_u _ID(arr++->ID);
		while((sc = arr++))
		{
			if(!sc->name) break;
			ssh_u flg(sc->flags);
			ssh_u offs(sc->offs + p_offs);
			String sval;
			if((flg & SC_OBJ))
			{
				// вложенный класс без своей схемы
				if(sc->ID != _ID)
				{
					arr = writeXml(xml, h, sc, sc->offs);
					continue;
				}
			}
			else if((flg & SC_NODE))
			{
				// вложенный со своей схемой
				Serialize* srlz((Serialize*)(this + sc->offs + p_offs));
				srlz->writeXml(xml, h, srlz->get_scheme());
				continue;
			}
			else if((flg & SC_ARRAY))
			{
				// массив фиксированной длины
				for(ssh_u i = 0; i < sc->count - 1; i++)
				{
					sval += getVal(flg, offs, sc) + L',';
					offs += sc->width;
				}
			}
			// проста€ переменна€
			sval += getVal(flg, offs, sc);
			xml->set_attr(h, sc->name, sval);
		}
		*/
		return nullptr;
	}

	Serialize::SCHEME* Serialize::writeXml(Xml* xml, HXML hp, SCHEME* arr, ssh_u p_offs)
	{
		SCHEME* sc;
		// создать узел
		HXML h(xml->add_node(hp, arr->name, L""));
		ssh_u _ID(arr++->ID);
		while((sc = arr++))
		{
			if(!sc->name) break;
			ssh_u flg(sc->flags);
			ssh_u offs(sc->offs + p_offs);
			String sval;
			if((flg & SC_OBJ))
			{
				// вложенный класс без своей схемы
				if(sc->ID != _ID)
				{
					arr = writeXml(xml, h, sc, sc->offs);
					continue;
				}
			}
			else if((flg & SC_NODE))
			{
				// вложенный со своей схемой
				Serialize* srlz((Serialize*)(this + sc->offs + p_offs));
				srlz->writeXml(xml, h, srlz->get_scheme(), p_offs);
				continue;
			}
			else if((flg & SC_ARRAY))
			{
				// массив фиксированной длины
				for(ssh_u i = 0; i < sc->count - 1; i++)
				{
					sval += getVal(flg, offs, sc) + L',';
					offs += sc->width;
				}
			}
			// проста€ переменна€
			sval += getVal(flg, offs, sc);
			xml->set_attr(h, sc->name, sval);
		}
		return arr;
	}
}