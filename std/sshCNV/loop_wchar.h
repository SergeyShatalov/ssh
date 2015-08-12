
struct wchar_conv_struct
{
	struct conv_struct parent;
	int state;
};

static size_t wchar_id_loop_convert(iconv_t icd, const char* * inbuf, size_t *inbytesleft, char* * outbuf, size_t *outbytesleft)
{
	const wchar_t* inptr = (const wchar_t*)*inbuf;
	size_t inleft = *inbytesleft / sizeof(wchar_t);
	wchar_t* outptr = (wchar_t*)*outbuf;
	size_t outleft = *outbytesleft / sizeof(wchar_t);
	size_t count = (inleft <= outleft ? inleft : outleft);
	if(count > 0)
	{
		*inbytesleft -= count * sizeof(wchar_t);
		*outbytesleft -= count * sizeof(wchar_t);
		do
			*outptr++ = *inptr++;
		while(--count > 0);
		*inbuf = (const char*)inptr;
		*outbuf = (char*)outptr;
	}
	return 0;
}

static size_t wchar_id_loop_reset(iconv_t icd, char* * outbuf, size_t *outbytesleft)
{
	return 0;
}
