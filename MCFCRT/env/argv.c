// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "argv.h"
#include "mcfwin.h"
#include <stdlib.h>
#include <wchar.h>

/*
	这里的内存布局可以如此表示：

	struct Storage {
		wchar_t cmdline[];		// 动态确定。
		MCF_ArgItem stub;		// pwszStr 指向 cmdline，uLen 是后面的 argv 的元素容量。
		MCF_ArgItem argv[];		// MCF_CRT_AllocArgv 返回的指针指向这里。
		MCF_ArgItem nil;		// pwszStr 为 NULL，uLen 为 0。
	};
*/

#define FIX_ALIGNMENT(cb_, as_)		(((cb_) + _Alignof(as_) - 1) / _Alignof(as_) * _Alignof(as_))

const MCF_ArgItem *MCF_CRT_AllocArgv(size_t *pArgc, const wchar_t *pwszCommandLine){
	const size_t uPrefixSize = FIX_ALIGNMENT((wcslen(pwszCommandLine) + 1) * sizeof(wchar_t), MCF_ArgItem);

	size_t uCapacity = 4;
	void *pStorage = malloc(uPrefixSize + (uCapacity + 2) * sizeof(MCF_ArgItem));
	if(!pStorage){
		return NULL;
	}
	MCF_ArgItem *pArgv = (MCF_ArgItem *)((char *)pStorage + uPrefixSize + sizeof(MCF_ArgItem));
	pArgv[-1].pwszStr = pStorage;
	pArgv[-1].uLen = uCapacity;

	const wchar_t *pwcRead = pwszCommandLine;
	wchar_t *pwcWrite = pStorage;
	*pArgc = 0;

	enum {
		ST_DELIM,
		ST_IN_ARG,
		ST_QUOTE_OPEN,
		ST_IN_QUOTE,
		ST_QUOTE_CLOSED,
	} eState = ST_DELIM;

	for(;;){
		const wchar_t wc = *(pwcRead++);
		if(wc == 0){
			break;
		}
		switch(eState){
		case ST_DELIM:
			if((wc == L' ') || (wc == L'\t')){
				// eState = ST_DELIM;
			} else {
				if(*pArgc == uCapacity){
					uCapacity = uCapacity * 3 / 2;
					void *pNewStorage = realloc(pStorage, uPrefixSize + (uCapacity + 2) * sizeof(MCF_ArgItem));
					if(!pNewStorage){
						goto error;
					}
					MCF_ArgItem *pArgv = (MCF_ArgItem *)((char *)pNewStorage + uPrefixSize + sizeof(MCF_ArgItem));
					pArgv[-1].pwszStr = pNewStorage;
					pArgv[-1].uLen = uCapacity;

					pwcWrite = (wchar_t *)pNewStorage + (pwcWrite - (wchar_t *)pStorage);

					pStorage = pNewStorage;
				}
				++*pArgc;
				pArgv[*pArgc - 1].pwszStr = pwcWrite;

				if(wc == L'\"'){
					eState = ST_QUOTE_OPEN;
				} else {
					*(pwcWrite++) = wc;
					eState = ST_IN_ARG;
				}
			}
			break;

		case ST_IN_ARG:
			if((wc == L' ') || (wc == L'\t')){
				pArgv[*pArgc - 1].uLen = (size_t)(pwcWrite - pArgv[*pArgc - 1].pwszStr);
				*(pwcWrite++) = 0;
				eState = ST_DELIM;
			} else if(wc == L'\"'){
				eState = ST_QUOTE_OPEN;
			} else {
				*(pwcWrite++) = wc;
				// eState = ST_IN_ARG;
			}
			break;

		case ST_QUOTE_OPEN:
			if(wc == L'\"'){
				eState = ST_QUOTE_CLOSED;
			} else {
				*(pwcWrite++) = wc;
				eState = ST_IN_QUOTE;
			}
			break;

		case ST_IN_QUOTE:
			if(wc == L'\"'){
				eState = ST_QUOTE_CLOSED;
			} else {
				*(pwcWrite++) = wc;
				// eState = ST_IN_QUOTE;
			}
			break;

		case ST_QUOTE_CLOSED:
			if((wc == L' ') || (wc == L'\t')){
				pArgv[*pArgc - 1].uLen = (size_t)(pwcWrite - pArgv[*pArgc - 1].pwszStr);
				*(pwcWrite++) = 0;
				eState = ST_DELIM;
			} else if(wc == L'\"'){
				*(pwcWrite++) = wc;
				eState = ST_IN_QUOTE;
			} else {
				*(pwcWrite++) = wc;
				eState = ST_IN_ARG;
			}
			break;
		}
	}
	switch(eState){
	case ST_DELIM:
		break;

	case ST_IN_ARG:
	case ST_QUOTE_OPEN:
	case ST_IN_QUOTE:
	case ST_QUOTE_CLOSED:
		pArgv[*pArgc - 1].uLen = (size_t)(pwcWrite - pArgv[*pArgc - 1].pwszStr);
		*(pwcWrite++) = 0;
		break;
	}

	pArgv[*pArgc].pwszStr = NULL;
	pArgv[*pArgc].uLen = 0;

	return pArgv;

error:
	free(pStorage);
	return NULL;
}

const MCF_ArgItem *MCF_CRT_AllocArgvFromCommandLine(size_t *pArgc){
	return MCF_CRT_AllocArgv(pArgc, GetCommandLineW());
}
void MCF_CRT_FreeArgv(const MCF_ArgItem *pArgItems){
	if(pArgItems){
		free((void *)(pArgItems[-1].pwszStr));
	}
}