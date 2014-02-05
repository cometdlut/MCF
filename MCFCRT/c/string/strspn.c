// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_strmanip.h"

size_t strspn(const char *s1, const char *s2){
	unsigned char table[256];
	__MCF_CRT_StrBuildTable(table, s2);
	return (size_t)(__MCF_CRT_StrFindFirstNotOf(s1, table) - s1);
}
