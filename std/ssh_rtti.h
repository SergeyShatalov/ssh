
#pragma once

#include "ssh_str.h"

namespace ssh
{
	class SSH RTTI
	{
	public:
		RTTI() { next = root; root = this; }
		// создать объект
		virtual RTTI* create() = 0;
		// вернуть имя класса
		const String& className() const { return name; }
		// создать
		static RTTI* createClass(const String& nm)
		{
			RTTI* rtti(root);
			while(rtti && rtti->className() != nm) rtti = rtti->next;
			return (rtti ? rtti->create() : nullptr);
		}
	protected:
		// имя класса
		String name;
		// следующий в списке
		RTTI* next;
		// корень списка
		static RTTI* root;
	};
}
