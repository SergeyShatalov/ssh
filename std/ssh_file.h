
#pragma once

#include "ssh_buf.h"
#include "ssh_str.h"
#include "ssh_time.h"
#include "ssh_log.h"

namespace ssh
{
	Buffer<ssh_cs> SSH ssh_cnv(ssh_wcs to, ssh_wcs str, bool is_null);
	String SSH ssh_cnv(ssh_wcs from, const Buffer<ssh_cs>& in, ssh_u offs);

	class SSH File
	{
	public:
		enum Seek : int
		{
			begin	= SEEK_SET,
			current	= SEEK_CUR,
			end		= SEEK_END
		};
		enum OpenFlags : int
		{
			append_read			= 0,
			append_read_write	= 1,
			create_write		= 2,
			create_read_write	= 3,
			open_read			= 4,
			open_write			= 5,
			open_read_write		= 6,
			access_temp_remove	= _O_TEMPORARY,
			access_temp			= _O_SHORT_LIVED,
			access_random		= _O_RANDOM,
			access_sequential	= _O_SEQUENTIAL
		};
		File() : h(0) {}
		File(ssh_wcs name, int flags) : h(0) { open(name, flags); }
		virtual ~File() { close(); }
		// открытие
		void open(ssh_wcs name, int flags);
		// закрытие
		void close() { if(h) _close(h); h = 0; }
		// чтение
		template <typename T> Buffer<T> read(ssh_u size = 0) const
		{
			if(!size) size = (length() - get_pos());
			Buffer<T> buf(size);
			read((void*)buf, buf.size());
			return buf;
		}
		template <typename T> Buffer<T> read(ssh_u size, ssh_u pos, int flags) const
		{
			set_pos(pos, flags);
			return read<T>(size); 
		}
		String read(ssh_wcs cnv, ssh_u size) const
		{
			return ssh_cnv(cnv, read<ssh_cs>(size), 0);
		}
		void File::write(const String& str, ssh_wcs cnv) const
		{
			write<ssh_cs>(ssh_cnv(cnv, str, false));
		}
		// запись
		template <typename T> void write(const Buffer<T>& buf) const
		{
			write((void*)buf, buf.size());
		}
		template <typename T> bool write(const Buffer<T>& buf, ssh_u pos, int flags) const
		{
			set_pos(pos, flags);
			return write(buf);
		}
		// сервис
		ssh_u get_pos() const { return _telli64(h); }
		ssh_u length() const { return _filelengthi64(h); }
		bool set_length(ssh_u size) { return (_chsize(h, (long)size) == 0); }
		ssh_u set_pos(ssh_u pos, int flags) const { return _lseeki64(h, pos, flags); }
		static bool rename(ssh_wcs _old, ssh_wcs _new) { return (_wrename(_old, _new) == 0); }
		static bool unlink(ssh_wcs name) { return (_wunlink(name) == 0); }
		static bool find(ssh_wcs path, _wfinddata64_t* data, intptr_t* handle, bool is_begin)
		{
			if(is_begin) return ((*handle = _wfindfirst64(path, data)) != 0);
			return (_wfindnext64(*handle, data) == 0);
		}
		void get_time(Time* create, Time* access, Time* write) const;
		String get_path() const { return path; }
		static bool is_exist(ssh_wcs path)
		{
			int h(_wopen(path, _O_RDONLY));
			if(h != -1) _close(h);
			return (h != -1);
		}
		void read(void* buf, ssh_u size) const;
		void write(void* buf, ssh_u size) const;
		bool is_close() const { return (h == 0); }
	protected:
		int h;
		String path;
	};
}
