// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014 LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "LzmaFilters.hpp"
#include "../Core/Exception.hpp"
#include "../../External/lzmalite/lzma.h"
using namespace MCF;

namespace {

constexpr std::size_t STEP_SIZE		= 0x4000;
constexpr lzma_stream INIT_STREAM	= LZMA_STREAM_INIT;

unsigned long LzmaErrorToWin32Error(lzma_ret eLzmaError) noexcept {
	switch(eLzmaError){
	case LZMA_OK:
		return ERROR_SUCCESS;

	case LZMA_STREAM_END:
		return ERROR_HANDLE_EOF;

	case LZMA_NO_CHECK:
	case LZMA_UNSUPPORTED_CHECK:
	case LZMA_GET_CHECK:
		return ERROR_INVALID_FUNCTION;

	case LZMA_MEM_ERROR:
	case LZMA_MEMLIMIT_ERROR:
		return ERROR_NOT_ENOUGH_MEMORY;

	case LZMA_FORMAT_ERROR:
		return ERROR_BAD_FORMAT;

	case LZMA_OPTIONS_ERROR:
		return ERROR_INVALID_PARAMETER;

	case LZMA_DATA_ERROR:
		return ERROR_INVALID_DATA;

	case LZMA_BUF_ERROR:
		return ERROR_SUCCESS;

	case LZMA_PROG_ERROR:
		return ERROR_INVALID_DATA;

	default:
		return ERROR_INVALID_FUNCTION;
	}
}

struct LzmaStreamCloser {
	void operator()(lzma_stream *pStream) const noexcept {
		::lzma_end(pStream);
	}
};

lzma_options_lzma MakeOptions(unsigned int uLevel, unsigned long ulDictSize){
	lzma_options_lzma vRet;
	if(::lzma_lzma_preset(&vRet, uLevel)){
		MCF_THROW(ERROR_INVALID_PARAMETER, L"::lzma_lzma_preset() 失败。"_wso);
	}
	vRet.dict_size = ulDictSize;
	return std::move(vRet);
}

class LzmaEncoderDelegate : CONCRETE(LzmaEncoder) {
private:
	const lzma_options_lzma xm_vOptions;

	lzma_stream xm_vStream;
	std::unique_ptr<lzma_stream, LzmaStreamCloser> xm_pStream;

public:
	LzmaEncoderDelegate(unsigned int uLevel, unsigned long ulDictSize)
		: xm_vOptions	(MakeOptions(uLevel, ulDictSize))
		, xm_vStream	(INIT_STREAM)
	{
	}

public:
	void Abort() noexcept {
		xm_pStream.reset();

		StreamFilterBase::Abort();
	}
	void Update(const void *pData, std::size_t uSize){
		if(!xm_pStream){
			const auto ulErrorCode = LzmaErrorToWin32Error(::lzma_alone_encoder(
				&xm_vStream, &xm_vOptions
			));
			if(ulErrorCode != ERROR_SUCCESS){
				MCF_THROW(ulErrorCode, L"::lzma_alone_encoder() 失败。"_wso);
			}

			xm_pStream.reset(&xm_vStream);
		}

		unsigned char abyTemp[STEP_SIZE];
		xm_pStream->next_out = abyTemp;
		xm_pStream->avail_out = sizeof(abyTemp);

		auto pbyRead = (const unsigned char *)pData;
		std::size_t uProcessed = 0;
		while(uProcessed < uSize){
			const auto uToProcess = Min(uSize - uProcessed, STEP_SIZE);

			xm_pStream->next_in = pbyRead;
			xm_pStream->avail_in = uToProcess;
			do {
				const auto ulErrorCode = LzmaErrorToWin32Error(::lzma_code(
					xm_pStream.get(), LZMA_RUN
				));
				if(ulErrorCode == ERROR_HANDLE_EOF){
					break;
				}
				if(ulErrorCode != ERROR_SUCCESS){
					MCF_THROW(ulErrorCode, L"::lzma_code() 失败。"_wso);
				}
				if(xm_pStream->avail_out == 0){
					xOutput(abyTemp, sizeof(abyTemp));

					xm_pStream->next_out = abyTemp;
					xm_pStream->avail_out = sizeof(abyTemp);
				}
			} while(xm_pStream->avail_in != 0);

			StreamFilterBase::Update(pbyRead, uToProcess);
			pbyRead += uToProcess;
			uProcessed += uToProcess;
		}
		if(xm_pStream->avail_out != 0){
			xOutput(abyTemp, sizeof(abyTemp) - xm_pStream->avail_out);
		}
	}
	void Finalize(){
		if(xm_pStream){
			unsigned char abyTemp[STEP_SIZE];
			xm_pStream->next_out = abyTemp;
			xm_pStream->avail_out = sizeof(abyTemp);

			xm_pStream->next_in = nullptr;
			xm_pStream->avail_in = 0;
			for(;;){
				const auto ulErrorCode = LzmaErrorToWin32Error(::lzma_code(
					xm_pStream.get(), LZMA_FINISH
				));
				if(ulErrorCode == ERROR_HANDLE_EOF){
					break;
				}
				if(ulErrorCode != ERROR_SUCCESS){
					MCF_THROW(ulErrorCode, L"::lzma_code() 失败。"_wso);
				}
				if(xm_pStream->avail_out == 0){
					xOutput(abyTemp, sizeof(abyTemp));

					xm_pStream->next_out = abyTemp;
					xm_pStream->avail_out = sizeof(abyTemp);
				}
			}
			if(xm_pStream->avail_out != 0){
				xOutput(abyTemp, sizeof(abyTemp) - xm_pStream->avail_out);
			}

			xm_pStream.reset();
		}
		StreamFilterBase::Finalize();
	}
};

class LzmaDecoderDelegate : CONCRETE(LzmaDecoder) {
private:
	lzma_stream xm_vStream;
	std::unique_ptr<lzma_stream, LzmaStreamCloser> xm_pStream;

public:
	LzmaDecoderDelegate()
		: xm_vStream(INIT_STREAM)
	{
	}

public:
	void Abort() noexcept {
		xm_pStream.reset();

		StreamFilterBase::Abort();
	}
	void Update(const void *pData, std::size_t uSize){
		if(!xm_pStream){
			const auto ulErrorCode = LzmaErrorToWin32Error(::lzma_alone_decoder(
				&xm_vStream, UINT64_MAX
			));
			if(ulErrorCode != ERROR_SUCCESS){
				MCF_THROW(ulErrorCode, L"::lzma_alone_decoder() 失败。"_wso);
			}
			xm_pStream.reset(&xm_vStream);
		}

		unsigned char abyTemp[STEP_SIZE];
		xm_pStream->next_out = abyTemp;
		xm_pStream->avail_out = sizeof(abyTemp);

		auto pbyRead = (const unsigned char *)pData;
		std::size_t uProcessed = 0;
		while(uProcessed < uSize){
			const auto uToProcess = Min(uSize - uProcessed, STEP_SIZE);

			xm_pStream->next_in = pbyRead;
			xm_pStream->avail_in = uToProcess;
			do {
				const auto ulErrorCode = LzmaErrorToWin32Error(::lzma_code(
					xm_pStream.get(), LZMA_RUN
				));
				if(ulErrorCode == ERROR_HANDLE_EOF){
					break;
				}
				if(ulErrorCode != ERROR_SUCCESS){
					MCF_THROW(ulErrorCode, L"::lzma_code() 失败。"_wso);
				}
				if(xm_pStream->avail_out == 0){
					xOutput(abyTemp, sizeof(abyTemp));

					xm_pStream->next_out = abyTemp;
					xm_pStream->avail_out = sizeof(abyTemp);
				}
			} while(xm_pStream->avail_in != 0);

			StreamFilterBase::Update(pbyRead, uToProcess);
			pbyRead += uToProcess;
			uProcessed += uToProcess;
		}
		if(xm_pStream->avail_out != 0){
			xOutput(abyTemp, sizeof(abyTemp) - xm_pStream->avail_out);
		}
	}
	void Finalize(){
		if(xm_pStream){
			unsigned char abyTemp[STEP_SIZE];
			xm_pStream->next_out = abyTemp;
			xm_pStream->avail_out = sizeof(abyTemp);

			xm_pStream->next_in = nullptr;
			xm_pStream->avail_in = 0;
			for(;;){
				const auto ulErrorCode = LzmaErrorToWin32Error(::lzma_code(
					xm_pStream.get(), LZMA_FINISH
				));
				if(ulErrorCode == ERROR_HANDLE_EOF){
					break;
				}
				if(ulErrorCode != ERROR_SUCCESS){
					MCF_THROW(ulErrorCode, L"::lzma_code() 失败。"_wso);
				}
				if(xm_pStream->avail_out == 0){
					xOutput(abyTemp, sizeof(abyTemp));

					xm_pStream->next_out = abyTemp;
					xm_pStream->avail_out = sizeof(abyTemp);
				}
			}
			if(xm_pStream->avail_out != 0){
				xOutput(abyTemp, sizeof(abyTemp) - xm_pStream->avail_out);
			}

			xm_pStream.reset();
		}
		StreamFilterBase::Finalize();
	}
};

}

// ========== LzmaEncoder ==========
// 静态成员函数。
std::unique_ptr<LzmaEncoder> LzmaEncoder::Create(unsigned int uLevel, unsigned long ulDictSize){
	return std::make_unique<LzmaEncoderDelegate>(uLevel, ulDictSize);
}

// ========== LzmaDecoder ==========
// 静态成员函数。
std::unique_ptr<LzmaDecoder> LzmaDecoder::Create(){
	return std::make_unique<LzmaDecoderDelegate>();
}