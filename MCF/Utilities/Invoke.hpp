// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_INVOKE_HPP_
#define MCF_UTILITIES_INVOKE_HPP_

#include <type_traits>
#include <utility>

namespace MCF {

namespace Impl {
	template<typename PrototypeT>
	struct Invoker {
		template<typename FuncT, typename ...ParamsT>
		decltype(auto) operator()(FuncT &&vFunc, ParamsT &&...vParams) const {
			return std::forward<FuncT>(vFunc)(std::forward<ParamsT>(vParams)...);
		}
	};

#define DEFINE_NON_STATIC_MEMBER_FUNCTION_INVOKER_(cv_)	\
	template<typename PtRetT, typename PtClassT, typename ...PtParamsT>	\
	struct Invoker<PtRetT (PtClassT::*)(PtParamsT...) cv_> {	\
		template<typename ClassT, typename ...ParamsT,	\
			std::enable_if_t<std::is_base_of<PtClassT, std::decay_t<ClassT>>::value,	\
				int> = 0>	\
		decltype(auto) operator()(PtRetT (PtClassT::*pMemFunc)(PtParamsT...) cv_, ClassT *pObj, ParamsT &&...vParams) const {	\
			return (pObj->*pMemFunc)(std::forward<ParamsT>(vParams)...);	\
		}	\
		template<typename ClassT, typename ...ParamsT,	\
			std::enable_if_t<std::is_base_of<PtClassT, std::decay_t<ClassT>>::value,	\
				int> = 0>	\
		decltype(auto) operator()(PtRetT (PtClassT::*pMemFunc)(PtParamsT...) cv_, ClassT &&vObj, ParamsT &&...vParams) const {	\
			return (std::forward<ClassT>(vObj).*pMemFunc)(std::forward<ParamsT>(vParams)...);	\
		}	\
		template<typename ClassT, typename ...ParamsT,	\
			std::enable_if_t<!std::is_convertible<std::decay_t<ClassT>, const volatile PtClassT *>::value && !std::is_base_of<PtClassT, std::decay_t<ClassT>>::value,	\
				int> = 0>	\
		decltype(auto) operator()(PtRetT (PtClassT::*pMemFunc)(PtParamsT...) cv_, ClassT &&vObj, ParamsT &&...vParams) const {	\
			return ((*std::forward<ClassT>(vObj)).*pMemFunc)(std::forward<ParamsT>(vParams)...);	\
		}	\
	};

DEFINE_NON_STATIC_MEMBER_FUNCTION_INVOKER_(const volatile)
DEFINE_NON_STATIC_MEMBER_FUNCTION_INVOKER_(const)
DEFINE_NON_STATIC_MEMBER_FUNCTION_INVOKER_(volatile)
DEFINE_NON_STATIC_MEMBER_FUNCTION_INVOKER_()

#undef DEFINE_NON_STATIC_MEMBER_FUNCTION_INVOKER_

}

template<typename FuncT, typename ...ParamsT>
decltype(auto) Invoke(FuncT &&vFunc, ParamsT &&...vParams){
	return Impl::Invoker<std::decay_t<FuncT>>()(std::forward<FuncT>(vFunc), std::forward<ParamsT>(vParams)...);
}

}

#endif
