
#pragma once

#include "ssh_math.h"

namespace ssh
{
	Buffer<ssh_cs> SSH ssh_cnv(ssh_wcs to, const String& str, bool is_null);
	String SSH ssh_cnv(ssh_wcs from, const Buffer<ssh_cs>& in, ssh_u offs);
	long SSH ssh_cpu_caps();
	ssh_u SSH ssh_rand(ssh_u begin, ssh_u end);
	ssh_u SSH ssh_hash(ssh_wcs wcs);
	ssh_u SSH ssh_hash(ssh_ccs ccs);
	ssh_u SSH ssh_hash_type(ssh_ccs nm);
	String SSH ssh_md5(const String& str);
	Buffer<ssh_cs> SSH ssh_to_base64(ssh_wcs charset, const String& str, bool to_str);
	Buffer<ssh_cs> SSH ssh_to_base64(const Buffer<ssh_cs>& buf, bool to_str);
	Buffer<ssh_cs> SSH ssh_from_base64(const String& str);
	Buffer<ssh_cs> SSH ssh_from_base64(const Buffer<ssh_cs>& buf, bool from_str);
	vec3 SSH ssh_vec3_mtx(const vec3& v, const mtx& m);
	vec4 SSH ssh_vec4_mtx(const vec4& v, const mtx& m);
	vec3 SSH ssh_mtx_vec3(const mtx& m, const vec3& v);
	vec4 SSH ssh_mtx_vec4(const mtx& m, const vec4& v);
	mtx SSH ssh_mtx_mtx(const mtx& m1, const mtx& m2);
}
