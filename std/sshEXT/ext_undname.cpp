
#include "stdafx.h"
#include "ext_undname.h"

#pragma	inline_depth(3)
#pragma	check_stack(off)

const size_t memBlockSize = 508;

class DName;
class DNameNode;
class Replicator;
class HeapManager;
class UnDecorator;
class charNode;
class pcharNode;
class pDNameNode;
class DNameStatusNode;

class HeapManager
{
private:
	struct Block
	{
		Block* next;
		char memBlock[memBlockSize];
		Block() { next = 0; }
	};
	Block* head;
	Block* tail;
	size_t blockLeft;
public:
	void Constructor() { blockLeft = 0; head = 0; tail = 0; }
	void* getMemory(size_t sz, int noBuffer);
	void Destructor() { while(tail = head) { head = tail->next; ::free(tail); } }
};

static HeapManager heap;

inline void* operator new(size_t sz, HeapManager& heap, int noBuffer){return heap.getMemory(sz, noBuffer); }

class DNameNode
{
private:
	DNameNode* next;
protected:
	DNameNode() : next(nullptr) {}
	DNameNode(const DNameNode & rd)	{ next = ((rd.next) ? rd.next->clone() : 0); }
public:
	virtual	int length() const PURE;
	virtual	ssh_cs*	getString(ssh_cs*, int) const PURE;
	DNameNode* clone();
	DNameNode* nextNode() const { return next; }
	DNameNode&	operator += (DNameNode* pNode)
	{
		if(pNode)
		{
			if(next)
			{
				DNameNode* pScan;
				for(pScan = next; pScan->next; pScan = pScan->next);
				pScan->next = pNode;
			}
			else next = pNode;
		}
		return	*this;
	}
};

class charNode : public DNameNode
{
	char me;
public:
	charNode(char ch) : me(ch) {}
	virtual	int length() const { return 1; }
	virtual	ssh_cs*	getString(ssh_cs* buf, int len) const { if(buf && len) *buf = me; else buf = 0; return buf; }
};

class pcharNode : public DNameNode
{
	ssh_cs*	me;
	int	myLen;
public:
	pcharNode(ssh_ccs str, int len = 0)
	{
		if(!len && str) len = (int)strlen(str);
		if(len && str) { me = gnew char[len]; myLen = len; if(me) strncpy(me, str, len); }
		else { me = 0; myLen = 0; }
	}
	virtual	int length() const { return myLen; }
	virtual	ssh_cs* getString(ssh_cs* buf, int len) const { if(len > pcharNode::length()) len = pcharNode::length(); return ((me && buf && len) ? strncpy(buf, me, len) : 0); }
};

class DName
{
public:
	DName() : node(0), stat(DN_valid), isIndir(0), isAUDC(0), isAUDTThunk(0) {}
	DName(char c) : DName() { if(c) doPchar(&c, 1); }
	DName(const DName& rd) : stat(rd.stat), isIndir(rd.isIndir), isAUDC(rd.isAUDC), isAUDTThunk(rd.isAUDTThunk), node(rd.node) {}
	DName(DNameNode* rd) : DName() { node = rd; }
	DName(ssh_ccs s) : DName() { if(s) doPchar(s, (int)strlen(s)); }
	DName(ssh_ccs& name, char terminator);
	DName(DNameStatus st);
	DName(DName* pd);
	DName(ssh_d num);
	int	isValid() const { return ((status() == DN_valid) || (status() == DN_truncated)); }
	int	isEmpty() const { return ((node == 0) || !isValid()); }
	DNameStatus	status() const { return (DNameStatus)stat; }
	DName& setPtrRef() { isIndir = 1; return *this; }
	int	isPtrRef() const { return isIndir; }
	int	isUDC() const { return (!isEmpty() && isAUDC); }
	void setIsUDC() { if(!isEmpty()) isAUDC = TRUE; }
	int	isUDTThunk() const { return (!isEmpty() && isAUDTThunk); }
	void setIsUDTThunk() { if(!isEmpty()) isAUDTThunk = TRUE; }
	int	length() const
	{
		int	len = 0;
		if(!isEmpty()) for(DNameNode * pNode = node; pNode; pNode = pNode->nextNode()) len += pNode->length();
		return	len;
	}
	ssh_cs*	getString(ssh_cs* buf, int max) const;
	DName operator + (ssh_ccs str) const
	{
		DName local(*this);
		if(local.isEmpty()) local = str; else local += str;
		return	local;
	}
	DName operator + (const DName& rd) const
	{
		DName local(*this);
		if(local.isEmpty()) local = rd;
		else if(rd.isEmpty()) local += rd.status(); else local += rd;
		return	local;
	}
	DName operator + (char ch) const
	{
		DName local(*this);
		if(local.isEmpty()) local = ch; else local += ch;
		return	local;
	}
	DName operator + (DName* pd) const
	{
		DName local(*this);
		if(local.isEmpty()) local = pd; else local += pd;
		return	local;
	}
	DName operator + (DNameStatus st) const
	{
		DName local(*this);
		if(local.isEmpty()) local = st; else local += st;
		return	local;
	}
	DName& operator += (char ch)
	{
		if(ch)
		{
			if(isEmpty()) *this = ch;
			else { node = node->clone(); if(node) *node += gnew charNode(ch); else stat = DN_error; }
		}
		return	*this;
	}
	DName& operator += (ssh_ccs str)
	{
		if(str && *str)
		{
			if(isEmpty()) *this = str;
			else { node = node->clone(); if(node) *node += gnew pcharNode(str); else stat = DN_error; }
		}
		return *this;
	}
	DName& operator += (DName* pd);
	DName& operator += (DNameStatus st);
	DName& operator += (const DName& rd)
	{
		if(rd.isEmpty()) *this += rd.status();
		else
		{
			if(isEmpty()) *this = rd;
			else { node = node->clone(); if(node) *node += rd.node; else stat = DN_error; }
		}
		return *this;
	}
	DName& operator |= (const DName& rd)
	{
		if((status() != DN_error) && !rd.isValid()) stat = rd.status();
		return	*this;
	}
	DName& operator = (ssh_ccs str) { isIndir = 0; isAUDC = 0; isAUDTThunk = 0; doPchar(str, (int)strlen(str)); return *this; }
	DName& operator = (const DName& rd)
	{
		if((status() == DN_valid) || (status() == DN_truncated)) { stat = rd.stat; isIndir = rd.isIndir; isAUDC = rd.isAUDC; isAUDTThunk = rd.isAUDTThunk; node = rd.node; }
		return	*this;
	}
	DName& operator = (char ch) { isIndir = 0; isAUDC = 0; isAUDTThunk = 0; doPchar(&ch, 1); return	*this; }
	DName& operator = (DName* pd);
	DName& operator = (DNameStatus st);
	friend DName operator + (char c, const DName& rd) { return	DName(c) + rd; }
	friend DName operator + (ssh_ccs s, const DName& rd) { return	DName(s) + rd; }
	friend DName operator + (DNameStatus st, const DName& rd) { return	DName(st) + rd; }
private:
	DNameNode* node;
	DNameStatus stat : 4;
	unsigned int isIndir : 1;
	unsigned int isAUDC : 1;
	unsigned int isAUDTThunk : 1;
	void doPchar(ssh_ccs str, int len);
};

class pDNameNode : public DNameNode
{
	DName* me;
public:
	pDNameNode(DName* pName) { me = ((pName && ((pName->status() == DN_invalid) || (pName->status() == DN_error))) ? 0 : pName); }
	virtual	int length() const { return (me ? me->length() : 0); }
	virtual	ssh_cs*	getString(ssh_cs* buf, int len) const { return ((me && buf && len) ? me->getString(buf, len) : 0); }
};

class DNameStatusNode : public DNameNode
{
	DNameStatus	me;
	int	myLen;
public:
	DNameStatusNode(DNameStatus stat) { me = stat;	myLen = ((me == DN_truncated) ? TruncationMessageLength : 0); }
	virtual	int length() const { return	myLen; }
	virtual	ssh_cs* getString(ssh_cs* buf, int len) const
	{
		if(len > DNameStatusNode::length()) len = DNameStatusNode::length();
		return (((me == DN_truncated) && buf && len) ? strncpy(buf, TruncationMessage, len) : 0);
	}
};

class Replicator
{
private:
	void operator = (const Replicator&);
	int index;
	DName* dNameBuffer[10];
	const DName ErrorDName;
	const DName	InvalidDName;
public:
	Replicator() : ErrorDName(DN_error), InvalidDName(DN_invalid) { index = -1; }
	int isFull() const { return	(index == 9); }
	Replicator& operator += (const DName & rd)
	{
		if(!isFull() && !rd.isEmpty()) { DName*	pNew = gnew DName(rd); if(pNew) dNameBuffer[++index] = pNew; }
		return	*this;
	}
	const DName& operator [] (int x) const
	{
		if((x < 0) || (x > 9)) return ErrorDName;
		else if((index == -1) || (x > index)) return InvalidDName;
		else return	*dNameBuffer[x];
	}
};

class UnDecorator
{
private:
	void operator = (const UnDecorator&);
	Replicator ArgList;
	static	Replicator*	pArgList;
	Replicator ZNameList;
	static Replicator* pZNameList;
	static Replicator* pTemplateArgList;
	static ssh_ccs gName;
	static ssh_ccs name;
	static ssh_cs* outputString;
	static int	maxStringLength;
	static ssh_d disableFlags;
	static DName getDecoratedName();
	static DName getSymbolName() { if(*gName == '?') { gName++; return getOperatorName(); } else return getZName(); }
	static DName getZName();
	static DName getOperatorName();
	static DName getScope();
	static DName getScopedName();
	static DName getSignedDimension() { if(!*gName) return	DN_truncated; else if(*gName == '?') { gName++; return '-' + getDimension(); } else return getDimension(); }
	static DName getDimension();
	static int getNumberOfDimensions();
	static DName getTemplateName();
	static DName getTemplateArgumentList();
	static DName getTemplateConstant();
	static DName composeDeclaration(const DName &);
	static int getTypeEncoding();
	static DName getBasedType();
	static DName getECSUName() { return getScopedName(); }
	static DName getEnumName();
	static DName getCallingConvention();
	static DName getReturnType(DName* pDeclarator = 0) { if(*gName == '@') { gName++; return DName(pDeclarator); } else return getDataType(pDeclarator); }
	static DName getDataType(DName*);
	static DName getPrimaryDataType(const DName &);
	static DName getDataIndirectType(const DName &, char, const DName &, int = FALSE);
	static DName getDataIndirectType() { return getDataIndirectType(DName(), 0, DName()); }
	static DName getBasicDataType(const DName &);
	static DName getECSUDataType(int = 0);
	static int getECSUDataIndirectType()
	{
		if(*gName)
		{
			unsigned int ecsuCode = *gName++ - 'A';
			if((ecsuCode >= ECSU_near) && (ecsuCode <= (ECSU_const | ECSU_volatile | ECSU_modelmask))) return (ecsuCode | ECSU_valid); else return ECSU_invalid;
		}
		else return ECSU_truncated;
	}
	static DName getPtrRefType(const DName&, const DName&, int);
	static DName getPtrRefDataType(const DName &, int);
	static DName getArrayType(const DName&);
	static DName getFunctionIndirectType(const DName& superType);
	static DName getArgumentTypes();
	static DName getArgumentList();
	static DName getThrowTypes() { if(*gName) if(*gName == AT_ellipsis) return (gName++, DName()); else return (" throw(" + getArgumentTypes() + ')'); else return (DName(" throw(") + DN_truncated + ')'); }
	static DName getLexicalFrame() { return '`' + getDimension() + '\''; }
	static DName getStorageConvention() { return getDataIndirectType(); }
	static DName getThisType() { return getDataIndirectType(DName(), 0, DName(), TRUE); }
	static DName getPointerType(const DName& cv, const DName& name) { return getPtrRefType(cv, name, TRUE); }
	static DName getReferenceType(const DName& cv, const DName& name) { return getPtrRefType(cv, name, FALSE); }
	static DName getExternalDataType(const DName& superType)
	{
		DName*	pDeclarator = gnew DName();
		DName declaration = getDataType(pDeclarator);
		*pDeclarator = getStorageConvention() + ' ' + superType;
		return	declaration;
	}
	static DName getSegmentName() { return getZName(); }
	static DName getDisplacement() { return getDimension(); }
	static DName getCallIndex() { return getDimension(); }
	static DName getGuardNumber() { return getDimension(); }
	static DName getVfTableType(const DName &);
	static DName getVbTableType(const DName& superType) { return getVfTableType(superType); }
	static DName getVCallThunkType()
	{
		switch(*gName)
		{
			case VMT_nTnCnV: ++gName; return DName("{flat}");
			case 0: return DN_truncated;
			default: return DN_invalid;
		}
	}
public:
	UnDecorator(ssh_cs*, ssh_ccs, int, ssh_d);
	static int doUnderScore() { return !(disableFlags & UNDNAME_NO_LEADING_UNDERSCORES); }
	static int doArray() { return !(disableFlags & UNDNAME_NO_RETURN_ARRAY); }
	static int doConst() { return !(disableFlags & UNDNAME_NO_RETURN_CONSTABLES); }
	static int doVolatile() { return !(disableFlags & UNDNAME_NO_RETURN_VOLATILES); }
	static int doMutable() { return !(disableFlags & UNDNAME_NO_RETURN_MUTABLES); }
	static int doMSKeywords() { return !(disableFlags & UNDNAME_NO_MS_KEYWORDS); }
	static int doFunctionReturns() { return !(disableFlags & UNDNAME_NO_FUNCTION_RETURNS); }
	static int doAllocationModel() { return !(disableFlags & UNDNAME_NO_ALLOCATION_MODEL); }
	static int doAllocationLanguage() { return !(disableFlags & UNDNAME_NO_ALLOCATION_LANGUAGE); }
	static int doThisTypes() { return ((disableFlags & UNDNAME_NO_THISTYPE) != UNDNAME_NO_THISTYPE); }
	static int doAccessSpecifiers() { return !(disableFlags & UNDNAME_NO_ACCESS_SPECIFIERS); }
	static int doThrowTypes() { return !(disableFlags & UNDNAME_NO_THROW_SIGNATURES); }
	static int doMemberTypes() { return !(disableFlags & UNDNAME_NO_MEMBER_TYPE); }
	static int doReturnUDTModel() { return !(disableFlags & UNDNAME_NO_RETURN_UDT_MODEL); }
	static int do32BitNear() { return !(disableFlags & UNDNAME_32_BIT_DECODE); }
	static int doNameOnly() { return (disableFlags & UNDNAME_NAME_ONLY); }
	static int doTypeOnly() { return (disableFlags & UNDNAME_TYPE_ONLY); }
	static ssh_ccs UScore(Tokens tok) { if(doUnderScore()) return tokenTable[tok]; else return tokenTable[tok] + 2; }
	operator ssh_cs*();
};
Replicator*	UnDecorator::pArgList;
Replicator*	UnDecorator::pZNameList = 0;
Replicator*	UnDecorator::pTemplateArgList = 0;
ssh_ccs UnDecorator::gName = 0;
ssh_ccs UnDecorator::name = 0;
ssh_cs* UnDecorator::outputString = 0;
int	UnDecorator::maxStringLength = 0;
ssh_d UnDecorator::disableFlags = 0;

EXT ssh_cs*	ext_undname(ssh_cs* out, ssh_ccs name, int len_out, ssh_d disableFlags)
{
	heap.Constructor();
	UnDecorator	unDecorate(out, name, len_out, disableFlags);
	ssh_cs*	unDecoratedName = unDecorate;
	heap.Destructor();
	return	unDecoratedName;
}

UnDecorator::UnDecorator(ssh_cs* out, ssh_ccs dName, int len_out, ssh_d disable)
{
	name = dName;
	gName = name;
	maxStringLength = len_out;
	outputString = out;
	pZNameList = &ZNameList;
	pArgList = &ArgList;
	disableFlags = disable;
}

UnDecorator::operator ssh_cs*()
{
	DName result;
	DName unDName;
	if(name)
	{
		if((*name == '?') && (name[1] == '@')) { gName += 2; result = "CV: " + getDecoratedName(); }
		else if((*name == '?') && (name[1] == '$')) result = getTemplateName(); else result = getDecoratedName();
	}
	if(result.status() == DN_error) return 0;
	else if((*gName && !doNameOnly()) || (result.status() == DN_invalid)) unDName = name;
	else unDName = result;
	if(!outputString)
	{
		maxStringLength = unDName.length() + 1;
		outputString = rnew char[maxStringLength];
	}
	if(outputString) unDName.getString(outputString, maxStringLength);
	return	outputString;
}

DName UnDecorator::getDecoratedName(void)
{
	if(doTypeOnly())
	{
		disableFlags &= ~UNDNAME_TYPE_ONLY;
		DName result = getDataType(NULL);
		disableFlags |= UNDNAME_TYPE_ONLY;
		return result;
	}
	else if(*gName == '?')
	{
		gName++;
		DName symbolName = getSymbolName();
		int	 udcSeen = symbolName.isUDC();
		if(!symbolName.isValid()) return symbolName;
		if(*gName && (*gName != '@'))
		{
			DName scope = getScope();
			if(!scope.isEmpty()) symbolName = scope + "::" + symbolName;
		}
		if(udcSeen) symbolName.setIsUDC();
		if(symbolName.isEmpty() || (doNameOnly() && !udcSeen)) return	symbolName;
		else if(!*gName || (*gName == '@'))
		{
			if(*gName) gName++;
			return composeDeclaration(symbolName);
		}
		else return	DN_invalid;
	}
	else if(*gName) return DN_invalid;
	else return	DN_truncated;
}

DName UnDecorator::getZName(void)
{
	int	zNameIndex = *gName - '0';
	if((zNameIndex >= 0) && (zNameIndex <= 9))
	{
		gName++;
		return	(*pZNameList)[zNameIndex];
	}
	else
	{
		DName zName;
		if(*gName == '?')
		{
			zName = getTemplateName();
			if(*gName++ != '@') zName = *--gName ? DN_invalid : DN_truncated;
		}
		else zName = DName(gName, '@');
		if(!pZNameList->isFull()) *pZNameList += zName;
		return	zName;
	}
}

DName UnDecorator::getOperatorName(void)
{
	DName operatorName;
	DName tmpName;
	int	udcSeen = FALSE;
	switch(*gName++)
	{
		case 0: gName--; return	DN_truncated;
		case OC_ctor:
		case OC_dtor:
		{
			ssh_ccs pName = gName;
			operatorName = getZName();
			gName = pName;
			if(!operatorName.isEmpty() && (gName[-1] == OC_dtor)) operatorName = '~' + operatorName;
			return	operatorName;
		}
		break;
		case OC_new:
		case OC_delete:
		case OC_assign:
		case OC_rshift:
		case OC_lshift:
		case OC_not:
		case OC_equal:
		case OC_unequal: operatorName = nameTable[gName[-1] - OC_new]; break;
		case OC_udc: udcSeen = TRUE;
		case OC_index:
		case OC_pointer:
		case OC_star:
		case OC_incr:
		case OC_decr:
		case OC_minus:
		case OC_plus:
		case OC_amper:
		case OC_ptrmem:
		case OC_divide:
		case OC_modulo:
		case OC_less:
		case OC_leq:
		case OC_greater:
		case OC_geq:
		case OC_comma:
		case OC_call:
		case OC_compl:
		case OC_xor:
		case OC_or:
		case OC_land:
		case OC_lor:
		case OC_asmul:
		case OC_asadd:
		case OC_assub: operatorName = nameTable[gName[-1] - OC_index + (OC_unequal - OC_new + 1)]; break;
		case '_':
			switch(*gName++)
			{
				case 0: gName--; return	DN_truncated;
				case OC_asdiv:
				case OC_asmod:
				case OC_asrshift:
				case OC_aslshift:
				case OC_asand:
				case OC_asor:
				case OC_asxor: operatorName = nameTable[gName[-1] - OC_asdiv + (OC_assub - OC_index + 1) + (OC_unequal - OC_new + 1)]; break;
				case OC_vftable:
				case OC_vbtable:
				case OC_vcall: return nameTable[gName[-1] - OC_asdiv + (OC_assub - OC_index + 1) + (OC_unequal - OC_new + 1)];
				case OC_metatype:
				case OC_guard:
				case OC_uctor:
				case OC_udtor:
				case OC_vdeldtor:
				case OC_defctor:
				case OC_sdeldtor:
				case OC_vctor:
				case OC_vdtor:
				case OC_vallctor:
				case OC_ehvctor:
				case OC_ehvdtor:
				case OC_ehvctorvb:
				case OC_copyctorclosure:
				case OC_locvfctorclosure:
				case OC_locvftable:
				case OC_placementDeleteClosure:
				case OC_placementArrayDeleteClosure: return nameTable[gName[-1] - OC_metatype + (OC_vcall - OC_asdiv + 1) + (OC_assub - OC_index + 1) + (OC_unequal - OC_new + 1)];
				case OC_udtthunk:
					operatorName = nameTable[gName[-1] - OC_metatype + (OC_vcall - OC_asdiv + 1) + (OC_assub - OC_index + 1) + (OC_unequal - OC_new + 1)];
					tmpName = getOperatorName();
					if(!tmpName.isEmpty() && tmpName.isUDTThunk()) return DN_invalid;
					return operatorName + tmpName;
				case OC_eh_init: break;
				case OC_rtti_init:
					operatorName = nameTable[gName[-1] - OC_metatype + (OC_vcall - OC_asdiv + 1) + (OC_assub - OC_index + 1) + (OC_unequal - OC_new + 1)];
					tmpName = rttiTable[gName[0] - OC_rtti_TD];
					switch(*gName++)
					{
						case OC_rtti_TD:
						{
							DName	result = getDataType(NULL);
							return result + ' ' + operatorName + tmpName;
						}
						break;
						case OC_rtti_BCD:
						{
							DName	result = operatorName + tmpName;
							result += getSignedDimension() + ',';
							result += getSignedDimension() + ',';
							result += getSignedDimension() + ',';
							result += getDimension() + ')';
							return result + '\'';
						}
						break;
						case OC_rtti_BCA:
						case OC_rtti_CHD:
						case OC_rtti_COL: return operatorName + tmpName; break;
						default: gName--; return DN_truncated;
					}
					break;
				case OC_arrayNew:
				case OC_arrayDelete: operatorName = DName(nameTable[gName[-1] - OC_new]) + "[]"; break;
				default: return	DN_invalid;
			}
			break;
		default: return	DN_invalid;
	}
	if(udcSeen) operatorName.setIsUDC();
	else if(!operatorName.isEmpty()) operatorName = "operator" + operatorName;
	return operatorName;
}

DName UnDecorator::getScope(void)
{
	DName scope;
	while((scope.status() == DN_valid) && *gName && (*gName != '@'))
	{
		if(!scope.isEmpty()) scope = "::" + scope;
		if(*gName == '?') switch(*++gName)
		{
			case '?': if(!doNameOnly()) scope = '`' + getDecoratedName() + '\'' + scope; else getDecoratedName(); break;
			case '$': gName--; scope = getZName() + scope; break;
			case '%': while(*gName != '@') gName++; gName++; scope = "`anonymous namespace'" + scope; break;
			default: if(!doNameOnly()) scope = getLexicalFrame() + scope; else getLexicalFrame(); break;
		}
		else scope = getZName() + scope;
	}
	switch(*gName)
	{
		case 0: if(scope.isEmpty()) scope = DN_truncated; else scope = DName(DN_truncated) + "::" + scope; break;
		case '@': break;
		default: scope = DN_invalid; break;
	}
	return	scope;
}

DName UnDecorator::getDimension()
{
	if(!*gName) return	DN_truncated;
	else if((*gName >= '0') && (*gName <= '9')) return	DName((unsigned long)(*gName++ - '0' + 1));
	else
	{
		unsigned long dim = 0L;
		while(*gName != '@')
		{
			if(!*gName) return	DN_truncated;
			else if((*gName >= 'A') && (*gName <= 'P')) dim = (dim << 4) + (*gName - 'A');
			else return	DN_invalid;
			gName++;
		}
		if(*gName++ != '@') return	DN_invalid;
		return	dim;
	}
}

int	UnDecorator::getNumberOfDimensions()
{
	if(!*gName) return	0;
	else if((*gName >= '0') && (*gName <= '9')) return	((*gName++ - '0') + 1);
	else
	{
		int	dim = 0;
		while(*gName != '@')
		{
			if(!*gName) return	0;
			else if((*gName >= 'A') && (*gName <= 'P')) dim = (dim << 4) + (*gName - 'A');
			else return	-1;
			gName++;
		}
		if(*gName++ != '@') return -1;
		return	dim;
	}
}

DName UnDecorator::getTemplateName(void)
{
	if(gName[0] != '?' || gName[1] != '$') return DN_invalid;
	gName += 2;
	Replicator * pSaveArgList = pArgList;
	Replicator * pSaveZNameList = pZNameList;
	Replicator * pSaveTemplateArgList = pTemplateArgList;
	Replicator localArgList, localZNameList, localTemplateArgList;
	pArgList = &localArgList;
	pZNameList = &localZNameList;
	pTemplateArgList = &localTemplateArgList;
	DName templateName = getZName();
	if(!templateName.isEmpty()) templateName += '<' + getTemplateArgumentList() + '>';
	pArgList = pSaveArgList;
	pZNameList = pSaveZNameList;
	pTemplateArgList = pSaveTemplateArgList;
	return	templateName;
}

DName UnDecorator::getTemplateArgumentList(void)
{
	int	first = TRUE;
	DName aList;
	while((aList.status() == DN_valid) && *gName && (*gName != AT_endoflist))
	{
		if(first) first = FALSE; else aList += ',';
		int	argIndex = *gName - '0';
		if((argIndex >= 0) && (argIndex <= 9)) { gName++; aList += (*pTemplateArgList)[argIndex]; }
		else
		{
			ssh_ccs oldGName = gName;
			DName arg;
			if(*gName == DT_void) { gName++; arg = "void"; }
			else if((*gName == '$') && (gName[1] != '$')) { gName++; arg = getTemplateConstant(); }
			else if(*gName == '?') { arg = "`template-parameter" + getSignedDimension() + "'"; }
			else arg = getPrimaryDataType(DName());
			if(((gName - oldGName) > 1) && !pTemplateArgList->isFull()) *pTemplateArgList += arg;
			aList += arg;
		}
	}
	return	aList;
}

DName UnDecorator::getTemplateConstant(void)
{
	switch(*gName++)
	{
		case TC_integral: return getSignedDimension();
		case TC_address: return (*gName == TC_nullptr ? "NULL" : getDecoratedName());
		case TC_fp:
		{
			DName mantissa(getSignedDimension());
			DName exponent(getSignedDimension());
			if(mantissa.isValid() && exponent.isValid())
			{
				char buf[100];
				if(!mantissa.getString(&(buf[1]), 100)) return	DN_invalid;
				buf[0] = buf[1];
				if(buf[0] == '-') { buf[1] = buf[2]; buf[2] = '.'; }
				else buf[1] = '.';
				return DName(buf) + 'e' + exponent;
			}
			else return DN_truncated;
		}
		case '\0': --gName; return	DN_truncated;
		default: return	DN_invalid;
	}
}

inline	DName UnDecorator::composeDeclaration(const DName & symbol)
{
	DName declaration;
	unsigned int typeCode = getTypeEncoding();
	int	symIsUDC = symbol.isUDC();
	if(TE_isbadtype(typeCode)) return	DN_invalid;
	else if(TE_istruncated(typeCode)) return (DN_truncated + symbol);
	if(TE_isfunction(typeCode) && !((TE_isthunk(typeCode) && TE_islocaldtor(typeCode)) || (TE_isthunk(typeCode) && (TE_istemplatector(typeCode) || TE_istemplatedtor(typeCode)))))
	{
		if(TE_isbased(typeCode)) if(doMSKeywords() && doAllocationModel()) declaration = ' ' + getBasedType(); else declaration |= getBasedType();
		if(TE_isthunk(typeCode) && TE_isvcall(typeCode))
		{
			declaration += symbol + '{' + getCallIndex() + ',';
			declaration += getVCallThunkType() + "}' ";
			if(doMSKeywords() && doAllocationLanguage()) declaration = ' ' + getCallingConvention() + ' ' + declaration; else declaration |= getCallingConvention();
		}
		else
		{
			DName vtorDisp;
			DName adjustment;
			DName thisType;
			if(TE_isthunk(typeCode))
			{
				if(TE_isvtoradj(typeCode)) vtorDisp = getDisplacement();
				adjustment = getDisplacement();
			}
			if(TE_ismember(typeCode) && !TE_isstatic(typeCode)) if(doThisTypes()) thisType = getThisType(); else thisType |= getThisType();
			if(doMSKeywords())
			{
				if(doAllocationLanguage()) declaration = getCallingConvention() + declaration; else declaration |= getCallingConvention();
			}
			else declaration |= getCallingConvention();
			if(!symbol.isEmpty()) if(!declaration.isEmpty() && !doNameOnly()) declaration += ' ' + symbol; else declaration = symbol;
			DName* pDeclarator = 0;
			DName returnType;
			if(symIsUDC) { declaration += " " + getReturnType(); if(doNameOnly()) return declaration; }
			else { pDeclarator = gnew DName; returnType = getReturnType(pDeclarator); }
			if(TE_isthunk(typeCode))
			{
				if(TE_isvtoradj(typeCode)) declaration += "`vtordisp{" + vtorDisp + ','; else declaration += "`adjustor{";
				declaration += adjustment + "}' ";
			}
			declaration += '(' + getArgumentTypes() + ')';
			if(TE_ismember(typeCode) && !TE_isstatic(typeCode)) declaration += thisType;
			if(doThrowTypes()) declaration += getThrowTypes(); else declaration |= getThrowTypes();
			if(doFunctionReturns() && pDeclarator) { *pDeclarator = declaration; declaration = returnType; }
		}
	}
	else
	{
		declaration += symbol;
		if(TE_isvftable(typeCode)) return getVfTableType(declaration);
		else if(TE_isvbtable(typeCode)) return	getVbTableType(declaration);
		else if(TE_isguard(typeCode)) return (declaration + '{' + getGuardNumber() + "}'");
		else if(TE_isthunk(typeCode) && TE_islocaldtor(typeCode)) declaration += "`local static destructor helper'";
		else if(TE_isthunk(typeCode) && TE_istemplatector(typeCode)) declaration += "`template static data member constructor helper'";
		else if(TE_isthunk(typeCode) && TE_istemplatedtor(typeCode)) declaration += "`template static data member destructor helper'";
		else if(TE_ismetaclass(typeCode)) return declaration;
		if(TE_isthunk(typeCode) && (TE_istemplatector(typeCode) || TE_istemplatedtor(typeCode))) { declaration = " " + declaration; }
		else declaration = getExternalDataType(declaration);
	}
	if(TE_ismember(typeCode))
	{
		if(doMemberTypes())
		{
			if(TE_isstatic(typeCode)) declaration = "static " + declaration;
			if(TE_isvirtual(typeCode) || (TE_isthunk(typeCode) && (TE_isvtoradj(typeCode) || TE_isadjustor(typeCode)))) declaration = "virtual " + declaration;
		}
		if(doAccessSpecifiers()) if(TE_isprivate(typeCode)) declaration = "private: " + declaration;
		else if(TE_isprotected(typeCode)) declaration = "protected: " + declaration;
		else if(TE_ispublic(typeCode)) declaration = "public: " + declaration;
	}
	if(TE_isthunk(typeCode)) declaration = "[thunk]:" + declaration;
	return	declaration;
}

inline int UnDecorator::getTypeEncoding(void)
{
	unsigned int typeCode = 0u;
	if(*gName == '_') { TE_setisbased(typeCode); gName++; }
	if((*gName >= 'A') && (*gName <= 'Z'))
	{
		int	code = *gName++ - 'A';
		TE_setisfunction(typeCode);
		if(code & TE_far) TE_setisfar(typeCode);
		else TE_setisnear(typeCode);
		if(code < TE_external)
		{
			TE_setismember(typeCode);
			switch(code & TE_access)
			{
				case TE_private: TE_setisprivate(typeCode); break;
				case TE_protect: TE_setisprotected(typeCode); break;
				case TE_public: TE_setispublic(typeCode); break;
				default: TE_setisbadtype(typeCode); return	typeCode;
			}
			switch(code & TE_adjustor)
			{
				case TE_adjustor: TE_setisadjustor(typeCode); break;
				case TE_virtual: TE_setisvirtual(typeCode); break;
				case TE_static: TE_setisstatic(typeCode); break;
				case TE_member: break;
				default: TE_setisbadtype(typeCode); return	typeCode;
			}
		}
	}
	else if(*gName == '$')
	{
		switch(*(++gName))
		{
			case SHF_localdtor:	TE_setislocaldtor(typeCode); break;
			case SHF_vcall:	TE_setisvcall(typeCode); break;
			case SHF_templateStaticDataMemberCtor: TE_setistemplatector(typeCode); break;
			case SHF_templateStaticDataMemberDtor: TE_setistemplatedtor(typeCode); break;
			case 0: TE_setistruncated(typeCode); break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			{
				int	code = *gName - '0';
				TE_setisfunction(typeCode);
				TE_setismember(typeCode);
				TE_setisvtoradj(typeCode);
				if(code & TE_far) TE_setisfar(typeCode); else TE_setisnear(typeCode);
				switch(code & TE_access_vadj)
				{
					case TE_private_vadj: TE_setisprivate(typeCode); break;
					case TE_protect_vadj: TE_setisprotected(typeCode); break;
					case TE_public_vadj: TE_setispublic(typeCode); break;
					default: TE_setisbadtype(typeCode); return	typeCode;
				}
			}
			break;
			default: TE_setisbadtype(typeCode); return	typeCode;
		}
		gName++;
	}
	else if((*gName >= TE_static_d) && (*gName <= TE_metatype))
	{
		int	code = *gName++;
		TE_setisdata(typeCode);
		switch(code)
		{
			case (TE_static_d | TE_private_d) : TE_setisstatic(typeCode); TE_setisprivate(typeCode); break;
			case (TE_static_d | TE_protect_d) : TE_setisstatic(typeCode); TE_setisprotected(typeCode); break;
			case (TE_static_d | TE_public_d) : TE_setisstatic(typeCode); TE_setispublic(typeCode); break;
			case TE_global: TE_setisglobal(typeCode); break;
			case TE_guard: TE_setisguard(typeCode); break;
			case TE_local: TE_setislocal(typeCode); break;
			case TE_vftable: TE_setisvftable(typeCode); break;
			case TE_vbtable: TE_setisvbtable(typeCode); break;
			case TE_metatype: TE_setismetaclass(typeCode); break;
			default: TE_setisbadtype(typeCode); return	typeCode;
		}
	}
	else if(*gName) TE_setisbadtype(typeCode); else TE_setistruncated(typeCode);
	return	typeCode;
}

DName UnDecorator::getBasedType(void)
{
	DName basedDecl(UScore(TOK_basedLp));
	if(*gName)
	{
		switch(*gName++)
		{
			case BT_void: basedDecl += "void"; break;
			case BT_nearptr: basedDecl += getScopedName(); break;
			case BT_basedptr: return DN_invalid;
		}
	}
	else basedDecl += DN_truncated;
	basedDecl += ") ";
	return	basedDecl;
}

DName UnDecorator::getScopedName(void)
{
	DName name;
	name = getZName();
	if((name.status() == DN_valid) && *gName && (*gName != '@')) name = getScope() + "::" + name;
	if(*gName == '@') gName++;
	else if(*gName) name = DN_invalid;
	else if(name.isEmpty()) name = DN_truncated;
	else name = DName(DN_truncated) + "::" + name;
	return	name;
}

DName UnDecorator::getEnumName(void)
{
	DName ecsuName;

	if(*gName)
	{
		switch(*gName)
		{
			case ET_schar:
			case ET_uchar: ecsuName = "char "; break;
			case ET_sshort:
			case ET_ushort: ecsuName = "short "; break;
			case ET_sint: break;
			case ET_uint: ecsuName = "int "; break;
			case ET_slong:
			case ET_ulong: ecsuName = "long "; break;
			default: return	DN_invalid;
		}
		switch(*gName++)
		{
			case ET_uchar:
			case ET_ushort:
			case ET_uint:
			case ET_ulong: ecsuName = "unsigned " + ecsuName; break;
		}
		return	ecsuName + getECSUName();
	}
	else return	DN_truncated;
}

DName UnDecorator::getCallingConvention(void)
{
	if(*gName)
	{
		unsigned int callCode = ((unsigned int)*gName++) - 'A';
		if((callCode >= CC_cdecl) && (callCode <= CC_interrupt))
		{
			DName callType;
			if(doMSKeywords())
			{
				switch(callCode & ~CC_saveregs)
				{
					case CC_cdecl: callType = UScore(TOK_cdecl); break;
					case CC_pascal: callType = UScore(TOK_pascal); break;
					case CC_thiscall: callType = UScore(TOK_thiscall); break;
					case CC_stdcall: callType = UScore(TOK_stdcall); break;
					case CC_fastcall: callType = UScore(TOK_fastcall); break;

				}
			}
			return	callType;
		}
		else return	DN_invalid;
	}
	else return	DN_truncated;
}

DName UnDecorator::getDataType(DName * pDeclarator)
{
	DName superType(pDeclarator);
	switch(*gName)
	{
		case 0: return (DN_truncated + superType);
		case DT_void: gName++; if(superType.isEmpty()) return "void"; else return "void " + superType;
		case '?':
		{
			gName++;
			superType = getDataIndirectType(superType, 0, DName(), 0);
			return getPrimaryDataType(superType);
			return superType;
		}
		default: return	getPrimaryDataType(superType);
	}
}

DName UnDecorator::getPrimaryDataType(const DName & superType)
{
	DName cvType;
	switch(*gName)
	{
		case 0: return (DN_truncated + superType);
		case PDT_volatileReference: cvType = "volatile"; if(!superType.isEmpty()) cvType += ' ';
		case PDT_reference:
		{
			DName super(superType);
			gName++;
			return	getReferenceType(cvType, super.setPtrRef());
		}
		case PDT_extend:
		{
			if(gName[1] != PDT_extend) if(gName[1] == '\0') return DN_truncated + superType; else return DN_invalid;
			gName += 2;
			switch(*gName)
			{
				case PDT_ex_function: gName++; return getFunctionIndirectType(superType);
				case PDT_ex_other: gName++; return getPtrRefDataType(superType, /* isPtr = */ TRUE);
				case PDT_ex_qualified: gName++; return (getBasicDataType(getDataIndirectType(superType, 0, DName(), 0)));
				case 0: return (DN_truncated + superType);
				default: return DN_invalid;
			}
		}
		default: return getBasicDataType(superType);
	}
}

DName UnDecorator::getArgumentTypes(void)
{
	switch(*gName)
	{
		case AT_ellipsis: return (gName++, "...");
		case AT_void: return (gName++, "void");
		default:
		{
			DName arguments(getArgumentList());
			if(arguments.status() == DN_valid) switch(*gName)
			{
				case 0: return arguments;
				case AT_ellipsis: return (gName++, arguments + ",...");
				case AT_endoflist: return (gName++, arguments);
				default: return	DN_invalid;
			}
			else return arguments;
		}
	}
}

DName UnDecorator::getArgumentList(void)
{
	int first = TRUE;
	DName aList;
	while((aList.status() == DN_valid) && (*gName != AT_endoflist) && (*gName != AT_ellipsis))
	{
		if(first) first = FALSE; else aList += ',';
		if(*gName)
		{
			int argIndex = *gName - '0';
			if((argIndex >= 0) && (argIndex <= 9)) { gName++; aList += (*pArgList)[argIndex]; }
			else
			{
				ssh_ccs oldGName = gName;
				DName arg(getPrimaryDataType(DName()));
				if(((gName - oldGName) > 1) && !pArgList->isFull()) *pArgList += arg;
				aList += arg;
			}
		}
		else { aList += DN_truncated; break; }
	}
	return	aList;
}

DName UnDecorator::getBasicDataType(const DName& superType)
{
	if(*gName)
	{
		unsigned char bdtCode = *gName++;
		unsigned char extended_bdtCode;
		int pCvCode = -1;
		DName basicDataType;
		switch(bdtCode)
		{
			case BDT_schar:
			case BDT_char:
			case (BDT_char | BDT_unsigned) : basicDataType = "char"; break;
			case BDT_short:
			case (BDT_short | BDT_unsigned) : basicDataType = "short"; break;
			case BDT_int:
			case (BDT_int | BDT_unsigned) : basicDataType = "int"; break;
			case BDT_long:
			case (BDT_long | BDT_unsigned) : basicDataType = "long"; break;
			case BDT_float: basicDataType = "float"; break;
			case BDT_longdouble: basicDataType = "long ";
			case BDT_double: basicDataType += "double"; break;
			case BDT_pointer:
			case (BDT_pointer | BDT_const) :
			case (BDT_pointer | BDT_volatile) :
			case (BDT_pointer | BDT_const | BDT_volatile) : pCvCode = (bdtCode & (BDT_const | BDT_volatile)); break;
			case BDT_extend:
				switch(extended_bdtCode = *gName++)
				{
					case BDT_bool: basicDataType = "bool"; break;
					case BDT_int8:
					case (BDT_int8 | BDT_unsigned) : basicDataType = "__int8"; break;
					case BDT_int16:
					case (BDT_int16 | BDT_unsigned) : basicDataType = "__int16"; break;
					case BDT_int32:
					case (BDT_int32 | BDT_unsigned) : basicDataType = "__int32"; break;
					case BDT_int64:
					case (BDT_int64 | BDT_unsigned) : basicDataType = "__int64"; break;
					case BDT_int128:
					case (BDT_int128 | BDT_unsigned) : basicDataType = "__int128"; break;
					case BDT_wchar_t: basicDataType = "wchar_t"; break;
					default: basicDataType = "UNKNOWN"; break;
				}
				break;
			default:
				gName--;
				basicDataType = getECSUDataType();
				if(basicDataType.isEmpty()) return basicDataType;
				break;
		}
		if(pCvCode == -1)
		{
			switch(bdtCode)
			{
				case (BDT_char | BDT_unsigned) :
				case (BDT_short | BDT_unsigned) :
				case (BDT_int | BDT_unsigned) :
				case (BDT_long | BDT_unsigned) : basicDataType = "unsigned " + basicDataType; break;
				case BDT_schar: basicDataType = "signed " + basicDataType; break;
				case BDT_extend:
					switch(extended_bdtCode)
					{
						case (BDT_int8 | BDT_unsigned) :
						case (BDT_int16 | BDT_unsigned) :
						case (BDT_int32 | BDT_unsigned) :
						case (BDT_int64 | BDT_unsigned) :
						case (BDT_int128 | BDT_unsigned) : basicDataType = "unsigned " + basicDataType; break;
					}
					break;
			}
			if(!superType.isEmpty()) basicDataType += ' ' + superType;
			return	basicDataType;
		}
		else
		{
			DName cvType;
			DName super(superType);
			if(superType.isEmpty())
			{
				if(pCvCode & BDT_const) { cvType = "const"; if(pCvCode & BDT_volatile) cvType += " volatile"; }
				else if(pCvCode & BDT_volatile) cvType = "volatile";
			}
			return getPointerType(cvType, super);
		}
	}
	else return (DN_truncated + superType);
}

DName UnDecorator::getECSUDataType(int ecsuMods)
{
	DName ecsuDataType;
	if(ecsuMods) if(ecsuMods == ECSU_invalid) return DN_invalid;
	else if(ecsuMods == ECSU_truncated) ecsuDataType = DN_truncated;
	else switch(ecsuMods & ECSU_modelmask) { case ECSU_based: if(doMSKeywords() && doReturnUDTModel()) ecsuDataType = getBasedType(); else ecsuDataType |= getBasedType(); break; }
	switch(*gName++)
	{
		case 0: gName--; return "`unknown ecsu'" + ecsuDataType + DN_truncated;
		case BDT_union: if(!doNameOnly()) ecsuDataType = "union " + ecsuDataType; break;
		case BDT_struct: if(!doNameOnly()) ecsuDataType = "struct " + ecsuDataType; break;
		case BDT_class: if(!doNameOnly()) ecsuDataType = "class " + ecsuDataType; break;
		case BDT_enum: return "enum " + ecsuDataType + getEnumName();
	}
	if(ecsuMods & ECSU_volatile) ecsuDataType = "volatile " + ecsuDataType;
	if(ecsuMods & ECSU_const) ecsuDataType = "const " + ecsuDataType;
	ecsuDataType += getECSUName();
	return	ecsuDataType;
}

DName UnDecorator::getFunctionIndirectType(const DName & superType)
{
	if(!*gName) return DN_truncated + superType;
	if(!IT_isfunction(*gName)) return DN_invalid;
	int fitCode = *gName++ - '6';
	if(fitCode == ('_' - '6'))
	{
		if(*gName)
		{
			fitCode = *gName++ - 'A' + FIT_based;
			if((fitCode < FIT_based) || (fitCode >(FIT_based | FIT_far | FIT_member))) fitCode = -1;
		}
		else return (DN_truncated + superType);
	}
	else if((fitCode < FIT_near) || (fitCode >(FIT_far | FIT_member))) fitCode = -1;
	if(fitCode == -1) return DN_invalid;
	DName thisType;
	DName fitType = superType;
	if(fitCode & FIT_member)
	{
		fitType = "::" + fitType;
		if(*gName) fitType = ' ' + getScope() + fitType; else fitType = DN_truncated + fitType;
		if(*gName) if(*gName == '@') gName++; else return DN_invalid; else return (DN_truncated + fitType);
		if(doThisTypes()) thisType = getThisType(); else thisType |= getThisType();
	}
	if(fitCode & FIT_based) if(doMSKeywords()) fitType = ' ' + getBasedType() + fitType; else fitType |= getBasedType();
	if(doMSKeywords()) fitType = getCallingConvention() + fitType; else fitType |= getCallingConvention();
	if(!superType.isEmpty()) fitType = '(' + fitType + ')';
	DName* pDeclarator = gnew DName;
	DName returnType(getReturnType(pDeclarator));
	fitType += '(' + getArgumentTypes() + ')';
	if(doThisTypes() && (fitCode & FIT_member)) fitType += thisType;
	if(doThrowTypes()) fitType += getThrowTypes();
	else fitType |= getThrowTypes();
	if(pDeclarator) *pDeclarator = fitType; else return	DN_error;
	return	returnType;
}

DName UnDecorator::getPtrRefType(const DName & cvType, const DName & superType, int isPtr)
{
	if(*gName)
	{
		if(IT_isfunction(*gName))
		{
			DName fitType = (isPtr ? '*' : '&');
			if(!cvType.isEmpty() && (superType.isEmpty() || !superType.isPtrRef())) fitType += cvType;
			if(!superType.isEmpty()) fitType += superType;
			return getFunctionIndirectType(fitType);
		}
		else
		{
			DName innerType(getDataIndirectType(superType, (isPtr ? '*' : '&'), cvType));
			return getPtrRefDataType(innerType, isPtr);
		}
	}
	else
	{
		DName trunk(DN_truncated);
		trunk += (isPtr ? '*' : '&');
		if(!cvType.isEmpty()) trunk += cvType;
		if(!superType.isEmpty())
		{
			if(!cvType.isEmpty()) trunk += ' ';
			trunk += superType;
		}
		return	trunk;
	}
}

DName UnDecorator::getDataIndirectType(const DName & superType, char prType, const DName & cvType, int thisFlag)
{
	if(*gName)
	{
		unsigned int ditCode = (*gName - ((*gName >= 'A') ? (unsigned int)'A' : (unsigned int)('0' - 26)));
		DName msExtension;
		if(doMSKeywords())
		{
			int fContinue = TRUE;
			do
			{
				switch(ditCode & DIT_modelmask)
				{
					case DIT_ptr64:
						if(!msExtension.isEmpty()) msExtension = msExtension + ' ' + UScore(TOK_ptr64); else msExtension = UScore(TOK_ptr64);
						gName++;
						ditCode = (*gName - ((*gName >= 'A') ? (unsigned int)'A' : (unsigned int)('0' - 26)));
						break;
					case DIT_restrict:
						if(!msExtension.isEmpty()) msExtension = msExtension + ' ' + UScore(TOK_restrict); else msExtension = UScore(TOK_restrict);
						gName++;
						ditCode = (*gName - ((*gName >= 'A') ? (unsigned int)'A' : (unsigned int)('0' - 26)));
						break;
					default: fContinue = FALSE; break;
				}
			} while(fContinue);
		}
		gName++;
		if((ditCode >= DIT_near) && (ditCode <= (DIT_const | DIT_volatile | DIT_modelmask | DIT_member)))
		{
			DName ditType(prType);
			if(!msExtension.isEmpty()) ditType = ditType + ' ' + msExtension;
			if(ditCode & DIT_member)
			{
				if(thisFlag) return	DN_invalid;
				if(prType != '\0')
				{
					ditType = "::" + ditType;
					if(*gName) ditType = getScope() + ditType; else ditType = DN_truncated + ditType;
				}
				else if(*gName) ditType |= getScope();
				if(!*gName) ditType += DN_truncated;
				else if(*gName++ != '@') return	DN_invalid;
			}
			if(doMSKeywords())
			{
				switch(ditCode & DIT_modelmask)
				{
					case DIT_based: if(thisFlag) return	DN_invalid; ditType = getBasedType() + ditType; break;
				}
			}
			else if((ditCode & DIT_modelmask) == DIT_based) ditType |= getBasedType();
			if(!doConst()) { if(ditCode & DIT_volatile) ditType = "volatile " + ditType; }
			if(!doVolatile()) { if(ditCode & DIT_const) ditType = "const " + ditType; }
			if(!thisFlag)
			{
				if(!superType.isEmpty())
				{
					if(superType.isPtrRef() || cvType.isEmpty()) ditType += ' ' + superType; else ditType += ' ' + cvType + ' ' + superType;
				}
			}
			else if(!cvType.isEmpty()) ditType += ' ' + cvType;
			ditType.setPtrRef();
			return ditType;
		}
		else return DN_invalid;
	}
	else if(!thisFlag && !superType.isEmpty())
	{
		if(superType.isPtrRef() || cvType.isEmpty()) return (DN_truncated + superType); else return (DN_truncated + cvType + ' ' + superType);
	}
	else if(!thisFlag && !cvType.isEmpty()) return (DN_truncated + cvType);
	else return	DN_truncated;
}

DName UnDecorator::getPtrRefDataType(const DName & superType, int isPtr)
{
	if(*gName)
	{
		if(isPtr && (*gName == PoDT_void))
		{
			gName++;
			if(superType.isEmpty()) return "void"; else return	"void " + superType;
		}
		if(*gName == RDT_array)
		{
			gName++;
			return getArrayType(superType);
		}
		return getBasicDataType(superType);
	}
	else return (DN_truncated + superType);
}

DName UnDecorator::getArrayType(const DName & superType)
{
	if(*gName)
	{
		int	noDimensions = getNumberOfDimensions();
		if(!noDimensions) return getBasicDataType(DName('[') + DN_truncated + ']');
		else
		{
			DName arrayType;
			if(!doArray())
			{
				while(noDimensions--) getDimension();
			}
			else
			{
				while(noDimensions--) arrayType += '[' + getDimension() + ']';
				if(!superType.isEmpty()) arrayType = '(' + superType + ')' + arrayType;
			}
			return getPrimaryDataType(arrayType);
		}
	}
	else if(!superType.isEmpty()) return getBasicDataType('(' + superType + ")[" + DN_truncated + ']');
	return getBasicDataType(DName('[') + DN_truncated + ']');
}

DName UnDecorator::getVfTableType(const DName & superType)
{
	DName vxTableName = superType;
	if(vxTableName.isValid() && *gName)
	{
		vxTableName = getStorageConvention() + ' ' + vxTableName;
		if(vxTableName.isValid())
		{
			if(*gName != '@')
			{
				vxTableName += "{for ";
				while(vxTableName.isValid() && *gName && (*gName != '@'))
				{
					vxTableName += '`' + getScope() + '\'';
					if(*gName == '@') gName++;
					if(vxTableName.isValid() && (*gName != '@')) vxTableName += "s ";
				}
				if(vxTableName.isValid())
				{
					if(!*gName) vxTableName += DN_truncated;
					vxTableName += '}';
				}
			}
			if(*gName == '@') gName++;
		}
	}
	else if(vxTableName.isValid()) vxTableName = DN_truncated + vxTableName;
	return	vxTableName;
}

DNameNode* DNameNode::clone()
{
	return gnew pDNameNode(gnew DName(this));
}

DName::DName(DNameStatus st) : DName()
{
	stat = (((st == DN_invalid) || (st == DN_error)) ? st : DN_valid);
	node = gnew DNameStatusNode(st);
	if(!node) stat = DN_error;
}

DName::DName(DName* pd) : DName()
{
	if(pd) { node = gnew pDNameNode(pd); stat = (node ? DN_valid : DN_error); }
	else { stat = DN_valid; node = 0; }
}

DName::DName(ssh_d num) : DName()
{
	char buf[11];
	char* pBuf = buf + 10;
	*pBuf = 0;
	do
	{
		*(--pBuf) = (char)((num % 10) + '0');
		num /= 10UL;
	} while(num);
	doPchar(pBuf, (int)(10 - (pBuf - buf)));
}

DName::DName(ssh_ccs& name, char terminator) : DName()
{
	// возможно неверно перенес
	if(name)
	{
		if(*name)
		{
			int	len = 0;
			ssh_ccs s;
			for(s = name; *name && (*name != terminator); name++) if(isValidIdentChar(*name)) len++; else { stat = DN_invalid; return; }
			doPchar(s, len);
			if(*name) { if(*name++ != terminator) { stat = DN_error; node = 0; } else stat = DN_valid; }
			else if(status() == DN_valid) stat = DN_truncated;
		}
		else stat = DN_truncated;
	}
	else stat = DN_invalid;
}

DName& DName::operator = (DName* pd)
{
	if((status() == DN_valid) || (status() == DN_truncated))
	{
		if(pd) { isIndir = 0; isAUDC = 0; isAUDTThunk = 0; node = gnew pDNameNode(pd); if(!node) stat = DN_error; }
		else *this = DN_error;
	}
	return	*this;
}

DName& DName::operator = (DNameStatus st)
{
	if((st == DN_invalid) || (st == DN_error)) { node = 0; if(status() != DN_error) stat = st; }
	else if((status() == DN_valid) || (status() == DN_truncated)) { isIndir = 0; isAUDC = 0; isAUDTThunk = 0; node = gnew DNameStatusNode(st); if(!node) stat = DN_error; }
	return	*this;
}

DName& DName::operator += (DName* pd)
{
	if(pd)
	{
		if(isEmpty()) *this = pd;
		else if((pd->status() == DN_valid) || (pd->status() == DN_truncated))
		{
			DNameNode* pNew = gnew pDNameNode(pd);
			if(pNew) { node = node->clone(); if(node) *node += pNew; }
			else node = 0;
			if(!node) stat = DN_error;
		}
		else *this += pd->status();
	}
	return	*this;
}
DName& DName::operator += (DNameStatus st)
{
	if(isEmpty() || ((st == DN_invalid) || (st == DN_error))) *this = st;
	else
	{
		DNameNode* pNew = gnew DNameStatusNode(st);
		if(pNew) { node = node->clone(); if(node) *node += pNew; }
		else node = 0;
		if(!node) stat = DN_error;
	}
	return	*this;
}

ssh_cs*	DName::getString(ssh_cs* buf, int max) const
{
	if(!isEmpty())
	{
		if(!buf) { max = length() + 1; buf = gnew char[max]; }
		if(buf)
		{
			int curLen = max;
			DNameNode* curNode = node;
			ssh_cs* curBuf = buf;
			while(curNode && (curLen > 0))
			{
				int fragLen = curNode->length();
				ssh_cs*	fragBuf = 0;
				if(fragLen)
				{
					if((curLen - fragLen) < 0) fragLen = curLen;
					fragBuf = curNode->getString(curBuf, fragLen);
					if(fragBuf) { curLen -= fragLen; curBuf += fragLen; }
				}
				curNode = curNode->nextNode();
			}
			*curBuf = 0;
		}
	}
	else if(buf) *buf = 0;
	return	buf;
}

void* HeapManager::getMemory(size_t sz, int noBuffer)
{
	sz = ((sz + PACK_SIZE - 1) & ~(PACK_SIZE - 1));
	if(noBuffer) return	::malloc(sz);
	else
	{
		if(!sz) sz = 1;
		if(blockLeft < sz)
		{
			if(sz > memBlockSize) return 0;
			Block* pNewBlock = rnew Block;
			if(pNewBlock)
			{
				if(tail) tail = tail->next = pNewBlock; else head = tail = pNewBlock;
				blockLeft = memBlockSize - sz;
			}
			else return	0;
		}
		else blockLeft -= sz;
		return &(tail->memBlock[blockLeft]);
	}
}

void DName::doPchar(ssh_ccs str, int len)
{
	if(!((status() == DN_invalid) || (status() == DN_error)))
	{
		if(node) *this = DN_error;
		else if(str && len)
		{
			switch(len)
			{
				case 0: stat = DN_error; break;
				case 1: node = gnew charNode(*str); if(!node) stat = DN_error; break;
				default: node = gnew pcharNode(str, len); if(!node) stat = DN_error; break;
			}
		}
	}
	else stat = DN_invalid;
}
