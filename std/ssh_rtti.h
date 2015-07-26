
#pragma once

#include "ssh_str.h"

namespace ssh
{
	class SSH RTTI
	{
	public:
		RTTI() { next = root; root = this; }
		// ������� ������
		virtual RTTI* create() = 0;
		// ������� ��� ������
		const String& className() const { return name; }
		// �������
		static RTTI* createClass(const String& nm)
		{
			RTTI* rtti(root);
			while(rtti && rtti->className() != nm) rtti = rtti->next;
			return (rtti ? rtti->create() : nullptr);
		}
	protected:
		// ��� ������
		String name;
		// ��������� � ������
		RTTI* next;
		// ������ ������
		static RTTI* root;
	};
}
