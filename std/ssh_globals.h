
#pragma once

namespace ssh
{
	Buffer<ssh_cs> SSH ssh_cnv(ssh_wcs to, const String& str, bool is_null);
	String SSH ssh_cnv(ssh_wcs from, const Buffer<ssh_cs>& in, ssh_u offs);
	String SSH ssh_md5(const String& str);
	String SSH ssh_base64(ssh_wcs charset, const String& str);
	String SSH ssh_base64(const Buffer<ssh_cs>& buf);
	Buffer<ssh_cs> SSH ssh_base64(const String& str, bool is_null);
	long SSH ssh_cpu_caps();
	ssh_u SSH ssh_rand(ssh_u begin, ssh_u end);
	ssh_u SSH ssh_hash(ssh_wcs wcs);
	ssh_u SSH ssh_hash(ssh_ccs ccs);
	ssh_u SSH ssh_hash_type(ssh_ccs nm);
}
