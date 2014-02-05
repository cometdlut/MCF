// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include <wchar.h>

wchar_t *wcspbrk(const wchar_t *s1, const wchar_t *s2){
	const wchar_t *rp = s1;
	for(;;){
		const wchar_t ch = *rp;
		if(ch == 0){
			return NULL;
		}
		if(wcschr(s2, ch)){
			return (wchar_t *)rp;
		}
		++rp;
	}
}
