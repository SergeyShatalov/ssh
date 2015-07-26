
#include "stdafx.h"
#include "ssh_file.h"

namespace ssh
{
	struct FILE_OPEN
	{
		int flag;
		int mode;
	};

	static FILE_OPEN opens[] = 
	{
		{_O_APPEND | _O_RDONLY, 0},
		{_O_APPEND | _O_RDWR, 0},
		{_O_CREAT | _O_TRUNC | _O_WRONLY, _S_IWRITE},
		{_O_CREAT | _O_TRUNC | _O_RDWR, _S_IWRITE | _S_IREAD},
		{_O_RDONLY, 0},
		{_O_WRONLY, 0},
		{_O_RDWR, 0}
	};

	void File::open(ssh_wcs name, int flags)
	{
		if((h = _wopen(name, opens[flags & 7].flag | (flags & (~7) | _O_BINARY), opens[flags & 7].mode)) == -1)
			SSH_THROW(L"Не удалось открыть файл (%s)", name);
		memcpy(path, name, wcslen(name) * 2 + 2);
	}

	void File::get_time(Time* create, Time* access, Time* write) const
	{
		struct _stat64 stat;
		if(_wstat64(path, &stat)) SSH_THROW(L"Не удалось получить время файла %s!", path);
		if(access) *access = stat.st_atime;
		if(create) *create = stat.st_ctime;
		if(write) *write = stat.st_mtime;
	}

	void File::read(void* buf, ssh_u size) const
	{
		if(_read(h, buf, (unsigned int)size) != size)
			SSH_THROW(L"Ошибка чтения файла %s!", path);
	}
	
	void File::write(void* buf, ssh_u size) const
	{
		if(_write(h, buf, (unsigned int)size) != size)
			SSH_THROW(L"Ошибка записи файла %s!", path);
	}
}
