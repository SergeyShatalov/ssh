
#pragma once

#include "ssh_singl.h"

namespace ssh
{
	class SSH Sql
	{
	public:
	protected:
		Sql();
		virtual ~Sql();
		// индекс синглота
		static ssh_u const singl_idx = SSH_SINGL_SQL;
	};
}