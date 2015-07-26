
/*
*	Автор:		Шаталов С. В.
*	Создано:	Владикавказ, 18 июля 2015, 9:07
*	Модификация:--
*	Описание:	Классы компресии и декомпресии в формате ZIP
*/

#pragma once

#define ZSWAP32(q) ((((q) >> 24) & 0xff) + (((q) >> 8) & 0xff00) + (((q) & 0xff00) << 8) + (((q) & 0xff) << 24))

#define ZIP_FIXED			4
#define ZIP_BLOCK			5
#define ZIP_TREES			6

#define ZIP_DEFLATED		8

#define ZIP_BINARY			0
#define ZIP_TEXT			1
#define ZIP_ASCII			ZIP_TEXT
#define ZIP_UNKNOWN			2

#define MIN_MATCH			3
#define BL_CODES			19
#define D_CODES				30
#define MAX_BITS			15
#define LENGTH_CODES		29
#define LITERALS			256
#define L_CODES				(LITERALS + 1 + LENGTH_CODES)
#define HEAP_SIZE			(2 * L_CODES + 1)
#define ZIP_INIT_STATE		42
#define ZIP_BUSY_STATE		113
#define ZIP_FINISH_STATE	333
#define ZIP_FINISH			4

#define MIN_LOOKAHEAD		(MAX_MATCH + MIN_MATCH + 1)
#define MAX_DIST()			((UINT)(w_size - MIN_LOOKAHEAD))

#define MAX_BL_BITS			7
#define END_BLOCK			256
#define REP_3_6				16
#define REPZ_3_10			17
#define REPZ_11_138			18
#define DIST_CODE_LEN		512
#define BASE				65521
#define NMAX				5552
#define MAX_MATCH			258
#define WIN_INIT			MAX_MATCH

#define DO1(buf,i)			{adler += (buf)[i]; sum2 += adler;}
#define DO2(buf,i)			DO1(buf,i); DO1(buf, i + 1);
#define DO4(buf,i)			DO2(buf,i); DO2(buf, i + 2);
#define DO8(buf,i)			DO4(buf,i); DO4(buf, i + 4);
#define DO16(buf)			DO8(buf,0); DO8(buf, 8);
#define MOD(a)				a %= BASE
#define MOD28(a)			a %= BASE
#define MOD63(a)			a %= BASE

#define ENOUGH_LENS 852
#define ENOUGH_DISTS 592
#define ENOUGH (ENOUGH_LENS + ENOUGH_DISTS)

struct zip_ct_data
{
	union {WORD freq, code;} fc;
	union{WORD dad, len;} dl;
};
struct zip_static_tree_desc
{
	const zip_ct_data* static_tree;
	const int* extra_bits;
	int extra_base;
	int elems;
	int max_length;
};
struct zip_tree_desc
{
	zip_ct_data* dyn_tree;
	int max_code;
	zip_static_tree_desc* stat_desc;
};

struct code
{
	BYTE op;
	BYTE bits;
	WORD val;
};

static const zip_ct_data static_ltree[L_CODES + 2] =
{
	{{ 12},{  8}}, {{140},{  8}}, {{ 76},{  8}}, {{204},{  8}}, {{ 44},{  8}},
	{{172},{  8}}, {{108},{  8}}, {{236},{  8}}, {{ 28},{  8}}, {{156},{  8}},
	{{ 92},{  8}}, {{220},{  8}}, {{ 60},{  8}}, {{188},{  8}}, {{124},{  8}},
	{{252},{  8}}, {{  2},{  8}}, {{130},{  8}}, {{ 66},{  8}}, {{194},{  8}},
	{{ 34},{  8}}, {{162},{  8}}, {{ 98},{  8}}, {{226},{  8}}, {{ 18},{  8}},
	{{146},{  8}}, {{ 82},{  8}}, {{210},{  8}}, {{ 50},{  8}}, {{178},{  8}},
	{{114},{  8}}, {{242},{  8}}, {{ 10},{  8}}, {{138},{  8}}, {{ 74},{  8}},
	{{202},{  8}}, {{ 42},{  8}}, {{170},{  8}}, {{106},{  8}}, {{234},{  8}},
	{{ 26},{  8}}, {{154},{  8}}, {{ 90},{  8}}, {{218},{  8}}, {{ 58},{  8}},
	{{186},{  8}}, {{122},{  8}}, {{250},{  8}}, {{  6},{  8}}, {{134},{  8}},
	{{ 70},{  8}}, {{198},{  8}}, {{ 38},{  8}}, {{166},{  8}}, {{102},{  8}},
	{{230},{  8}}, {{ 22},{  8}}, {{150},{  8}}, {{ 86},{  8}}, {{214},{  8}},
	{{ 54},{  8}}, {{182},{  8}}, {{118},{  8}}, {{246},{  8}}, {{ 14},{  8}},
	{{142},{  8}}, {{ 78},{  8}}, {{206},{  8}}, {{ 46},{  8}}, {{174},{  8}},
	{{110},{  8}}, {{238},{  8}}, {{ 30},{  8}}, {{158},{  8}}, {{ 94},{  8}},
	{{222},{  8}}, {{ 62},{  8}}, {{190},{  8}}, {{126},{  8}}, {{254},{  8}},
	{{  1},{  8}}, {{129},{  8}}, {{ 65},{  8}}, {{193},{  8}}, {{ 33},{  8}},
	{{161},{  8}}, {{ 97},{  8}}, {{225},{  8}}, {{ 17},{  8}}, {{145},{  8}},
	{{ 81},{  8}}, {{209},{  8}}, {{ 49},{  8}}, {{177},{  8}}, {{113},{  8}},
	{{241},{  8}}, {{  9},{  8}}, {{137},{  8}}, {{ 73},{  8}}, {{201},{  8}},
	{{ 41},{  8}}, {{169},{  8}}, {{105},{  8}}, {{233},{  8}}, {{ 25},{  8}},
	{{153},{  8}}, {{ 89},{  8}}, {{217},{  8}}, {{ 57},{  8}}, {{185},{  8}},
	{{121},{  8}}, {{249},{  8}}, {{  5},{  8}}, {{133},{  8}}, {{ 69},{  8}},
	{{197},{  8}}, {{ 37},{  8}}, {{165},{  8}}, {{101},{  8}}, {{229},{  8}},
	{{ 21},{  8}}, {{149},{  8}}, {{ 85},{  8}}, {{213},{  8}}, {{ 53},{  8}},
	{{181},{  8}}, {{117},{  8}}, {{245},{  8}}, {{ 13},{  8}}, {{141},{  8}},
	{{ 77},{  8}}, {{205},{  8}}, {{ 45},{  8}}, {{173},{  8}}, {{109},{  8}},
	{{237},{  8}}, {{ 29},{  8}}, {{157},{  8}}, {{ 93},{  8}}, {{221},{  8}},
	{{ 61},{  8}}, {{189},{  8}}, {{125},{  8}}, {{253},{  8}}, {{ 19},{  9}},
	{{275},{  9}}, {{147},{  9}}, {{403},{  9}}, {{ 83},{  9}}, {{339},{  9}},
	{{211},{  9}}, {{467},{  9}}, {{ 51},{  9}}, {{307},{  9}}, {{179},{  9}},
	{{435},{  9}}, {{115},{  9}}, {{371},{  9}}, {{243},{  9}}, {{499},{  9}},
	{{ 11},{  9}}, {{267},{  9}}, {{139},{  9}}, {{395},{  9}}, {{ 75},{  9}},
	{{331},{  9}}, {{203},{  9}}, {{459},{  9}}, {{ 43},{  9}}, {{299},{  9}},
	{{171},{  9}}, {{427},{  9}}, {{107},{  9}}, {{363},{  9}}, {{235},{  9}},
	{{491},{  9}}, {{ 27},{  9}}, {{283},{  9}}, {{155},{  9}}, {{411},{  9}},
	{{ 91},{  9}}, {{347},{  9}}, {{219},{  9}}, {{475},{  9}}, {{ 59},{  9}},
	{{315},{  9}}, {{187},{  9}}, {{443},{  9}}, {{123},{  9}}, {{379},{  9}},
	{{251},{  9}}, {{507},{  9}}, {{  7},{  9}}, {{263},{  9}}, {{135},{  9}},
	{{391},{  9}}, {{ 71},{  9}}, {{327},{  9}}, {{199},{  9}}, {{455},{  9}},
	{{ 39},{  9}}, {{295},{  9}}, {{167},{  9}}, {{423},{  9}}, {{103},{  9}},
	{{359},{  9}}, {{231},{  9}}, {{487},{  9}}, {{ 23},{  9}}, {{279},{  9}},
	{{151},{  9}}, {{407},{  9}}, {{ 87},{  9}}, {{343},{  9}}, {{215},{  9}},
	{{471},{  9}}, {{ 55},{  9}}, {{311},{  9}}, {{183},{  9}}, {{439},{  9}},
	{{119},{  9}}, {{375},{  9}}, {{247},{  9}}, {{503},{  9}}, {{ 15},{  9}},
	{{271},{  9}}, {{143},{  9}}, {{399},{  9}}, {{ 79},{  9}}, {{335},{  9}},
	{{207},{  9}}, {{463},{  9}}, {{ 47},{  9}}, {{303},{  9}}, {{175},{  9}},
	{{431},{  9}}, {{111},{  9}}, {{367},{  9}}, {{239},{  9}}, {{495},{  9}},
	{{ 31},{  9}}, {{287},{  9}}, {{159},{  9}}, {{415},{  9}}, {{ 95},{  9}},
	{{351},{  9}}, {{223},{  9}}, {{479},{  9}}, {{ 63},{  9}}, {{319},{  9}},
	{{191},{  9}}, {{447},{  9}}, {{127},{  9}}, {{383},{  9}}, {{255},{  9}},
	{{511},{  9}}, {{  0},{  7}}, {{ 64},{  7}}, {{ 32},{  7}}, {{ 96},{  7}},
	{{ 16},{  7}}, {{ 80},{  7}}, {{ 48},{  7}}, {{112},{  7}}, {{  8},{  7}},
	{{ 72},{  7}}, {{ 40},{  7}}, {{104},{  7}}, {{ 24},{  7}}, {{ 88},{  7}},
	{{ 56},{  7}}, {{120},{  7}}, {{  4},{  7}}, {{ 68},{  7}}, {{ 36},{  7}},
	{{100},{  7}}, {{ 20},{  7}}, {{ 84},{  7}}, {{ 52},{  7}}, {{116},{  7}},
	{{  3},{  8}}, {{131},{  8}}, {{ 67},{  8}}, {{195},{  8}}, {{ 35},{  8}},
	{{163},{  8}}, {{ 99},{  8}}, {{227},{  8}}
};

static const zip_ct_data static_dtree[D_CODES] =
{
	{{ 0},{ 5}}, {{16},{ 5}}, {{ 8},{ 5}}, {{24},{ 5}}, {{ 4},{ 5}},
	{{20},{ 5}}, {{12},{ 5}}, {{28},{ 5}}, {{ 2},{ 5}}, {{18},{ 5}},
	{{10},{ 5}}, {{26},{ 5}}, {{ 6},{ 5}}, {{22},{ 5}}, {{14},{ 5}},
	{{30},{ 5}}, {{ 1},{ 5}}, {{17},{ 5}}, {{ 9},{ 5}}, {{25},{ 5}},
	{{ 5},{ 5}}, {{21},{ 5}}, {{13},{ 5}}, {{29},{ 5}}, {{ 3},{ 5}},
	{{19},{ 5}}, {{11},{ 5}}, {{27},{ 5}}, {{ 7},{ 5}}, {{23},{ 5}}
};

const BYTE _dist_code[DIST_CODE_LEN] =
{
	0,  1,  2,  3,  4,  4,  5,  5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  8,
	8,  8,  8,  8,  9,  9,  9,  9,  9,  9,  9,  9, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
	11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13,
	13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
	13, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,  0,  0, 16, 17,
	18, 18, 19, 19, 20, 20, 20, 20, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 22,
	23, 23, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
	26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
	26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 27, 27, 27, 27, 27, 27, 27, 27,
	27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
	27, 27, 27, 27, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
	28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
	28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
	28, 28, 28, 28, 28, 28, 28, 28, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
	29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
	29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
	29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29
};

const BYTE _length_code[MAX_MATCH-MIN_MATCH+1]=
{
	0,  1,  2,  3,  4,  5,  6,  7,  8,  8,  9,  9, 10, 10, 11, 11, 12, 12, 12, 12,
	13, 13, 13, 13, 14, 14, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16,
	17, 17, 17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 19, 19, 19, 19,
	19, 19, 19, 19, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 23,
	23, 23, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
	25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 26, 26, 26, 26, 26, 26, 26, 26,
	26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
	26, 26, 26, 26, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
	27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 28
};

static const int base_length[LENGTH_CODES] =
{
	0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 14, 16, 20, 24, 28, 32, 40, 48, 56,
	64, 80, 96, 112, 128, 160, 192, 224, 0
};

static const int base_dist[D_CODES] =
{
	0,     1,     2,     3,     4,     6,     8,    12,    16,    24,
	32,    48,    64,    96,   128,   192,   256,   384,   512,   768,
	1024,  1536,  2048,  3072,  4096,  6144,  8192, 12288, 16384, 24576
};

static const int extra_lbits[LENGTH_CODES] = {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0};
static const int extra_dbits[D_CODES] = {0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};
static const int extra_blbits[BL_CODES] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,3,7};
static const BYTE bl_order[BL_CODES] = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};

static zip_static_tree_desc static_l_desc = {static_ltree, extra_lbits, LITERALS + 1, L_CODES, MAX_BITS};
static zip_static_tree_desc static_d_desc = {static_dtree, extra_dbits, 0, D_CODES, MAX_BITS};
static zip_static_tree_desc static_bl_desc = {(const zip_ct_data*)0, extra_blbits, 0, BL_CODES, MAX_BL_BITS};

namespace ssh
{
	class Zip;
	class ZipDeflate;
	class ZipInflate;

	class SSH ZipDeflate final
	{
		friend class Zip;
	public:
		enum zip_block_state {need_more, block_done, finish_started, finish_done};
		ZipDeflate() : strm(nullptr), pending_buf(nullptr), head(nullptr), prev(nullptr), window(nullptr) {}
		~ZipDeflate() {SSH_DEL(pending_buf); SSH_DEL(head); SSH_DEL(prev); SSH_DEL(window);}
		// запаковщик
		DWORD make(Zip* zip, int flush);
	protected:
		void pqdownheap(zip_ct_data* tree, int k);
		void gen_bitlen(zip_tree_desc* desc);
		void compress_block(zip_ct_data* ltree, zip_ct_data* dtree);
		void build_tree(zip_tree_desc* desc);
		void bi_windup();
		void copy_block(char* buf, UINT len, int header);
		void init_block();
		void zip_putShortMSB(UINT b);
		void zip_bi_flush();
		void zip_fill_window();
		void _zip_tr_stored_block(char* buf, DWORD stored_len, int last);
		void send_tree(zip_ct_data* tree, int max_code);
		void scan_tree(zip_ct_data* tree, int max_code);
		zip_block_state zip_deflate_slow(int flush);
		UINT longest_match(DWORD cur_match);
		void _zip_tr_flush_block(char* buf, DWORD stored_len, int last);
		void send_all_trees(int lcodes, int dcodes, int blcodes);
		int build_bl_tree();
		void gen_codes(zip_ct_data* tree, int max_code, WORD* bl_count);
		Zip* strm;
		int status;
		BYTE* pending_buf;
		DWORD pending_buf_size;
		BYTE* pending_out;
		UINT pending;
		int wrap;
		UINT gzindex;
		BYTE method;
		UINT w_size, w_bits, w_mask;
		BYTE* window;
		DWORD window_size;
		WORD* prev, *head;
		UINT ins_h, hash_size, hash_bits, hash_mask, hash_shift;
		long block_start;
		UINT match_length, prev_match;
		int match_available;
		UINT strstart, match_start, lookahead, prev_length, max_chain_length, max_lazy_match;
		int level;
		int strategy;
		UINT good_match;
		int nice_match;
		zip_ct_data dyn_ltree[HEAP_SIZE], dyn_dtree[2 * D_CODES + 1], bl_tree[2 * BL_CODES + 1];
		zip_tree_desc l_desc, d_desc, bl_desc;
		WORD bl_count[MAX_BITS + 1];
		int heap[2 * L_CODES + 1], heap_len, heap_max;
		WORD depth[2 * L_CODES + 1];
		BYTE* l_buf;
		UINT lit_bufsize, last_lit;
		WORD* d_buf;
		DWORD opt_len, static_len;
		UINT matches, insert;
		WORD bi_buf;
		int bi_valid;
		DWORD high_water;
	};
	
	class SSH ZipInflate final
	{
		friend class Zip;
	public:
		enum inflate_mode
		{
			HEAD, FLAGS, TIME, OS, EXLEN, EXTRA, NAME, COMMENT, HCRC, DICTID, DICT, TYPE, TYPEDO, STORED, COPY_, COPY, TABLE, LENLENS,
			CODELENS, LEN_, LEN, LENEXT, DIST, DISTEXT, MATCH, LIT, CHECK, LENGTH, DONE, BAD
		};
		enum codetype
		{
			CODES, LENS, DISTS
		};
		ZipInflate() : strm(nullptr) {}
		~ZipInflate() {}
		// распаковщик
		DWORD make(Zip* zip, int flush);
	protected:
		void fixedtables();
		int inflateTable(codetype type, WORD* lens, UINT codes, code** table, UINT* bits, WORD* work);
		void updateWindow(UINT out);
		void inflateFast(UINT start);
		DWORD inflate(int flush);
		Zip* strm;
		// текущий режим
		inflate_mode mode;
		// признак обработки последнего блока
		int last, wrap;
		// признак использования словаря
		int havedict;
		// метод и флаги
		int flags;
		// максимальная дистанция
		UINT dmax;
		// для проверки
		DWORD check, total;
		UINT wbits, wsize, whave, wnext;
		BYTE* window;
		DWORD hold;
		UINT bits, length, offset, extra;
		code const* lencode;
		code const* distcode;
		UINT lenbits, distbits, ncode, nlen, ndist, have;
		code *next;
		WORD lens[320], work[288];
		code codes[ENOUGH];
		int sane, back;
		UINT was;
	};
	
	class SSH Zip
	{
		friend class ZipDeflate;
		friend class ZipInflate;
	public:
		Zip() : next_in(nullptr), avail_in(0), total_in(0), next_out(nullptr), avail_out(0), total_out(0), adler(0) {}
		~Zip() {}
		Buffer<ssh_b> compress(const Buffer<ssh_b>& buf);
		Buffer<ssh_b> decompress(const Buffer<ssh_b>& buf);
	protected:
		static DWORD ostrov32(DWORD adler, const BYTE* buf, UINT len);
		static DWORD crc32_big(DWORD crc, const BYTE* buf, UINT len);
		static DWORD crc32_little(DWORD crc, const BYTE* buf, UINT len);
		static DWORD crc32(DWORD crc, BYTE* buf, UINT len);
		void zip_flush_pending();
		int zip_read_buf(BYTE* buf, UINT size);
		// следующий входящий байт
		BYTE* next_in;
		// количество доступных входящих байт
		UINT avail_in;
		// всего входящих байт
		DWORD total_in;
		// следующий выходной байт
		BYTE* next_out;
		// количество оставшихся байт
		UINT avail_out;
		// всего оставшихся байт
		DWORD total_out;
		// значение для входных данных
		DWORD adler;
		ZipDeflate* state;
	};
}
