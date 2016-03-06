// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "BufferingInputStreamFilter.hpp"

namespace MCF {

BufferingInputStreamFilter::~BufferingInputStreamFilter(){
}

int BufferingInputStreamFilter::Peek() const {
	return y_vBufferedStream.Peek();
}
int BufferingInputStreamFilter::Get(){
	return y_vBufferedStream.Get();
}
bool BufferingInputStreamFilter::Discard(){
	return y_vBufferedStream.Discard();
}

std::size_t BufferingInputStreamFilter::Peek(void *pData, std::size_t uSize) const {
	return y_vBufferedStream.Peek(pData, uSize);
}
std::size_t BufferingInputStreamFilter::Get(void *pData, std::size_t uSize){
	return y_vBufferedStream.Get(pData, uSize);
}
std::size_t BufferingInputStreamFilter::Discard(std::size_t uSize){
	return y_vBufferedStream.Discard(uSize);
}

}
