// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_MULTI_INDEXED_HPP__
#define __MCF_CRT_MULTI_INDEXED_HPP__

#include "../../c/ext/offset_of.h"
#include "../../c/ext/unref_param.h"
#include "../../env/avl_tree.h"
#include <tuple>
#include <type_traits>

namespace MCF {

struct BinaryLess {
	template<class Tx, class Ty>
	constexpr bool operator()(const Tx &lhs, const Ty &rhs) const {
		return lhs < rhs;
	}
};

template<typename Index_t, class Comparer_t = BinaryLess>
struct Index {
	Index_t m_vIndex;

	template<typename... Params_t>
	Index(Params_t &&... vParams)
		: m_vIndex(std::forward<Params_t>(vParams)...)
	{
	}

	template<typename Comparand_t>
	bool operator()(const Comparand_t &lhs, int) const {
		return Comparer_t()(lhs, m_vIndex);
	}
	template<typename Comparand_t>
	bool operator()(const Comparand_t &rhs) const {
		return Comparer_t()(m_vIndex, rhs);
	}
};

template<typename Index_t>
struct MakeIndex {
	typedef Index<Index_t> IndexType;
};
template<typename Index_t, typename Comparer_t>
struct MakeIndex<Index<Index_t, Comparer_t>> {
	typedef Index<Index_t, Comparer_t> IndexType;
};

template<class Element_t, class... Indexes_t>
class MultiIndexedMap {
public:
	enum : std::size_t {
		INDEX_COUNT = sizeof...(Indexes_t)
	};

	typedef typename std::tuple<typename MakeIndex<Indexes_t>::IndexType...> Indexes;

	static_assert(INDEX_COUNT > 0, "No index?");

	class Node {
		friend MultiIndexedMap;

	private:
		Element_t xm_vElement;
		Indexes xm_vIndexes;

#ifndef NDEBUG
		const MultiIndexedMap *xm_pOwner;
#endif

	private:
		AvlNodeHeader xm_arHeaders[INDEX_COUNT];

	public:
		template<typename ElementParam_t, typename FirstIndexParam_t, typename... IndexParams_t>
		constexpr Node(ElementParam_t &&vElementParam, FirstIndexParam_t &&vFirstIndexParam, IndexParams_t &&... vIndexParams)
			: xm_vElement(std::forward<ElementParam_t>(vElementParam))
			, xm_vIndexes(std::forward<FirstIndexParam_t>(vFirstIndexParam), std::forward<IndexParams_t>(vIndexParams)...)
#ifndef NDEBUG
			, xm_pOwner(nullptr)
#endif
		{
		}

	public:
		constexpr const Element_t &GetElement() const noexcept {
			return xm_vElement;
		}
		Element_t &GetElement() noexcept {
			return xm_vElement;
		}

		template<std::size_t INDEX>
		constexpr auto GetIndex() const noexcept -> const decltype(std::get<INDEX>(xm_vIndexes).m_vIndex) & {
			static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

			return std::get<INDEX>(xm_vIndexes).m_vIndex;
		}
	};

private:
	template<typename Tx, typename Ty>
	struct xComparer;

	template<typename Index_t, class Comparer_t, typename Comparand_t>
	struct xComparer<Index<Index_t, Comparer_t>, Comparand_t> {
		bool operator()(std::intptr_t lhs, std::intptr_t rhs) const {
			return (*(const Index<Index_t, Comparer_t> *)lhs)(*(const Comparand_t *)rhs);
		}
	};
	template<typename Comparand_t, typename Index_t, class Comparer_t>
	struct xComparer<Comparand_t, Index<Index_t, Comparer_t>> {
		bool operator()(std::intptr_t lhs, std::intptr_t rhs) const {
			return (*(const Index<Index_t, Comparer_t> *)rhs)(*(const Comparand_t *)lhs, 0);
		}
	};
	template<typename Index1_t, typename Index2_t, class Comparer_t>
	struct xComparer<Index<Index1_t, Comparer_t>, Index<Index2_t, Comparer_t>> {
		bool operator()(std::intptr_t lhs, std::intptr_t rhs) const {
			return (*(const Index<Index1_t, Comparer_t> *)lhs)(((const Index<Index2_t, Comparer_t> *)rhs)->m_vIndex);
		}
	};

private:
	template<std::size_t INDEX>
	static const AvlNodeHeader *xAvlFromNode(const Node *pNode) noexcept {
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		if(!pNode){
			return nullptr;
		}
		return &(pNode->xm_arHeaders[INDEX]);
	}
	template<std::size_t INDEX>
	static AvlNodeHeader *xAvlFromNode(Node *pNode) noexcept {
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		if(!pNode){
			return nullptr;
		}
		return &(pNode->xm_arHeaders[INDEX]);
	}

	template<std::size_t INDEX>
	static const Node *xNodeFromAvl(const AvlNodeHeader *pNode) noexcept {
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		if(!pNode){
			return nullptr;
		}
		return DOWN_CAST(const Node, xm_arHeaders[INDEX], pNode);
	}
	template<std::size_t INDEX>
	static Node *xNodeFromAvl(AvlNodeHeader *pNode) noexcept {
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		if(!pNode){
			return nullptr;
		}
		return DOWN_CAST(Node, xm_arHeaders[INDEX], pNode);
	}

public:
	template<std::size_t INDEX>
	static const Node *GetPrev(const Node *pNode) noexcept {
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		return xNodeFromAvl<INDEX>(AvlPrev(xAvlFromNode<INDEX>(pNode)));
	}
	template<std::size_t INDEX>
	static Node *GetPrev(Node *pNode) noexcept {
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		return xNodeFromAvl<INDEX>(AvlPrev(xAvlFromNode<INDEX>(pNode)));
	}

	template<std::size_t INDEX>
	static const Node *GetNext(const Node *pNode) noexcept {
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		return xNodeFromAvl<INDEX>(AvlNext(xAvlFromNode<INDEX>(pNode)));
	}
	template<std::size_t INDEX>
	static Node *GetNext(Node *pNode) noexcept {
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		return xNodeFromAvl<INDEX>(AvlNext(xAvlFromNode<INDEX>(pNode)));
	}

public:
	struct {
		AvlNodeHeader *pFront;
		AvlNodeHeader *pRoot;
		AvlNodeHeader *pBack;
	} xm_arvIndexPointers[INDEX_COUNT];

	std::size_t xm_uSize;

public:
	MultiIndexedMap() noexcept {
		for(auto &vPointers : xm_arvIndexPointers){
			vPointers.pFront	= nullptr;
			vPointers.pRoot		= nullptr;
			vPointers.pBack		= nullptr;
		}
		xm_uSize = 0;
	}
	MultiIndexedMap(const MultiIndexedMap &rhs) noexcept
		: MultiIndexedMap()
	{
		xCloneRecur(rhs);
	}
	MultiIndexedMap &operator=(const MultiIndexedMap &rhs) noexcept {
		if(this != &rhs){
			Clear();
			xCloneRecur(rhs);
		}
		return *this;
	}
	MultiIndexedMap(MultiIndexedMap &&rhs) noexcept
		: MultiIndexedMap()
	{
		Swap(rhs);
	}
	MultiIndexedMap &operator=(MultiIndexedMap &&rhs) noexcept {
		Swap(rhs);
		return *this;
	}
	~MultiIndexedMap(){
		Clear();
	}

private:
	template<std::size_t INDEX>
	void xAttach(Node *pNode) noexcept {
		typedef typename std::tuple_element<INDEX, Indexes>::type KeyType;

		AvlAttach(
			&(xm_arvIndexPointers[INDEX].pRoot),
			(std::intptr_t)&std::get<INDEX>(pNode->xm_vIndexes),
			xAvlFromNode<INDEX>(pNode),
			xComparer<KeyType, KeyType>()
		);

		const auto pAvl = xAvlFromNode<INDEX>(pNode);
		const auto pPrev = AvlPrev(pAvl);
		if(!pPrev){
			xm_arvIndexPointers[INDEX].pFront = pAvl;
		}
		const auto pNext = AvlNext(pAvl);
		if(!pNext){
			xm_arvIndexPointers[INDEX].pBack = pAvl;
		}
	}
	template<std::size_t INDEX>
	void xAttachAll(Node *pNode, typename std::enable_if<INDEX != (std::size_t)-1, int>::type = 0) noexcept {
		ASSERT(pNode->xm_pOwner == nullptr);

		xAttach<INDEX>(pNode);
		xAttachAll<INDEX - 1>(pNode);
	}
	template<std::size_t INDEX>
	void xAttachAll(Node *pNode, typename std::enable_if<INDEX == (std::size_t)-1, int>::type = 0) noexcept {
		UNREF_PARAM(pNode);

#ifndef NDEBUG
		pNode->xm_pOwner = this;
#endif
	}

	template<std::size_t INDEX>
	void xDetach(Node *pNode) noexcept {
		const auto pAvl = xAvlFromNode<INDEX>(pNode);
		if(pAvl == xm_arvIndexPointers[INDEX].pFront){
			xm_arvIndexPointers[INDEX].pFront = AvlNext(pAvl);
		}
		if(pAvl == xm_arvIndexPointers[INDEX].pBack){
			xm_arvIndexPointers[INDEX].pBack = AvlPrev(pAvl);
		}

		AvlDetach(pAvl);
	}
	template<std::size_t INDEX>
	void xDetachAll(Node *pNode, typename std::enable_if<INDEX != (std::size_t)-1, int>::type = 0) noexcept {
		ASSERT(pNode->xm_pOwner == this);

		xDetach<INDEX>(pNode);
		xDetachAll<INDEX - 1>(pNode);
	}
	template<std::size_t INDEX>
	void xDetachAll(Node *pNode, typename std::enable_if<INDEX == (std::size_t)-1, int>::type = 0) noexcept {
		UNREF_PARAM(pNode);

#ifndef NDEBUG
		pNode->xm_pOwner = nullptr;
#endif
	}

	void xCloneRecur(const MultiIndexedMap &rhs){
		ASSERT(this != &rhs);

		AvlNodeHeader *pAvl = rhs.xm_arvIndexPointers[0].pFront;
		while(pAvl){
			Node *const pNode = xNodeFromAvl<0>(pAvl);
			pAvl = AvlNext(pAvl);
			Insert(*pNode);
		}
	}

public:
	template<typename... Params_t>
	Node *Insert(Params_t &&... vParams){
		const auto pNode = new Node(std::forward<Params_t>(vParams)...);
		xAttachAll<INDEX_COUNT - 1>(pNode);
		++xm_uSize;
		return pNode;
	}
	void Erase(Node *pNode) noexcept {
		xDetachAll<INDEX_COUNT - 1>(pNode);
		--xm_uSize;
		delete pNode;
	}
	void Clear() noexcept {
		AvlNodeHeader *pAvl = xm_arvIndexPointers[0].pFront;
		while(pAvl){
			Node *const pNode = xNodeFromAvl<0>(pAvl);
			pAvl = AvlNext(pAvl);
			delete pNode;
		}

		for(auto &vPointers : xm_arvIndexPointers){
			vPointers.pFront	= nullptr;
			vPointers.pRoot		= nullptr;
			vPointers.pBack		= nullptr;
		}
		xm_uSize = 0;
	}

	std::size_t GetSize() const noexcept {
		return xm_uSize;
	}
	bool IsEmpty() const noexcept {
		return xm_uSize == 0;
	}

	void Swap(MultiIndexedMap &rhs) noexcept {
		if(this != &rhs){
			std::swap(xm_arvIndexPointers, rhs.xm_arvIndexPointers);
			std::swap(xm_uSize, rhs.xm_uSize);
		}
	}

	template<std::size_t INDEX, typename... Params_t>
	void SetIndex(Node *pNode, Params_t &&... vParams){
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		auto vNewIndex = std::tuple_element<INDEX, Indexes>(std::forward<Params_t>(vParams)...);

		xDetach<INDEX>(pNode);
		try {
			std::get<INDEX>(pNode->xm_vIndexes) = std::move(vNewIndex);
			xAttach<INDEX>(pNode);
		} catch(...){
			xAttach<INDEX>(pNode);
			throw;
		}
	}

	template<std::size_t INDEX>
	const Node *GetBegin() const noexcept {
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		return xNodeFromAvl<INDEX>(xm_arvIndexPointers[INDEX].pFront);
	}
	template<std::size_t INDEX>
	Node *GetBegin() noexcept {
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		return xNodeFromAvl<INDEX>(xm_arvIndexPointers[INDEX].pFront);
	}

	template<std::size_t INDEX>
	const Node *GetRBegin() const noexcept {
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		return xNodeFromAvl<INDEX>(xm_arvIndexPointers[INDEX].pBack);
	}
	template<std::size_t INDEX>
	Node *GetRBegin() noexcept {
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		return xNodeFromAvl<INDEX>(xm_arvIndexPointers[INDEX].pBack);
	}

	template<std::size_t INDEX, typename Comparand_t>
	const Node *GetLowerBound(const Comparand_t &vComparand) const
		noexcept(
			noexcept	(xComparer<typename std::tuple_element<INDEX, Indexes>::type, Comparand_t>()(0, 0))
			&& noexcept	(xComparer<Comparand_t, typename std::tuple_element<INDEX, Indexes>::type>()(0, 0))
		)
	{
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		typedef typename std::tuple_element<INDEX, Indexes>::type KeyType;

		return xNodeFromAvl<INDEX>(AvlLowerBound(
			&(xm_arvIndexPointers[INDEX].pRoot),
			(std::intptr_t)&vComparand,
			xComparer<KeyType, Comparand_t>(),
			xComparer<Comparand_t, KeyType>()
		));
	}
	template<std::size_t INDEX, typename Comparand_t>
	Node *GetLowerBound(const Comparand_t &vComparand)
		noexcept(
			noexcept	(xComparer<typename std::tuple_element<INDEX, Indexes>::type, Comparand_t>()(0, 0))
			&& noexcept	(xComparer<Comparand_t, typename std::tuple_element<INDEX, Indexes>::type>()(0, 0))
		)
	{
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		typedef typename std::tuple_element<INDEX, Indexes>::type KeyType;

		return xNodeFromAvl<INDEX>(AvlLowerBound(
			&(xm_arvIndexPointers[INDEX].pRoot),
			(std::intptr_t)&vComparand,
			xComparer<KeyType, Comparand_t>(),
			xComparer<Comparand_t, KeyType>()
		));
	}

	template<std::size_t INDEX, typename Comparand_t>
	const Node *GetUpperBound(const Comparand_t &vComparand) const
		noexcept(
			noexcept	(xComparer<typename std::tuple_element<INDEX, Indexes>::type, Comparand_t>()(0, 0))
			&& noexcept	(xComparer<Comparand_t, typename std::tuple_element<INDEX, Indexes>::type>()(0, 0))
		)
	{
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		typedef typename std::tuple_element<INDEX, Indexes>::type KeyType;

		return xNodeFromAvl<INDEX>(AvlUpperBound(
			&(xm_arvIndexPointers[INDEX].pRoot),
			(std::intptr_t)&vComparand,
			xComparer<KeyType, Comparand_t>(),
			xComparer<Comparand_t, KeyType>()
		));
	}
	template<std::size_t INDEX, typename Comparand_t>
	Node *GetUpperBound(const Comparand_t &vComparand)
		noexcept(
			noexcept	(xComparer<typename std::tuple_element<INDEX, Indexes>::type, Comparand_t>()(0, 0))
			&& noexcept	(xComparer<Comparand_t, typename std::tuple_element<INDEX, Indexes>::type>()(0, 0))
		)
	{
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		typedef typename std::tuple_element<INDEX, Indexes>::type KeyType;

		return xNodeFromAvl<INDEX>(AvlUpperBound(
			&(xm_arvIndexPointers[INDEX].pRoot),
			(std::intptr_t)&vComparand,
			xComparer<KeyType, Comparand_t>(),
			xComparer<Comparand_t, KeyType>()
		));
	}

	template<std::size_t INDEX, typename Comparand_t>
	const Node *Find(const Comparand_t &vComparand) const
		noexcept(
			noexcept	(xComparer<typename std::tuple_element<INDEX, Indexes>::type, Comparand_t>()(0, 0))
			&& noexcept	(xComparer<Comparand_t, typename std::tuple_element<INDEX, Indexes>::type>()(0, 0))
		)
	{
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		typedef typename std::tuple_element<INDEX, Indexes>::type KeyType;

		return xNodeFromAvl<INDEX>(AvlFind(
			&(xm_arvIndexPointers[INDEX].pRoot),
			(std::intptr_t)&vComparand,
			xComparer<KeyType, Comparand_t>(),
			xComparer<Comparand_t, KeyType>()
		));
	}
	template<std::size_t INDEX, typename Comparand_t>
	Node *Find(const Comparand_t &vComparand)
		noexcept(
			noexcept	(xComparer<typename std::tuple_element<INDEX, Indexes>::type, Comparand_t>()(0, 0))
			&& noexcept	(xComparer<Comparand_t, typename std::tuple_element<INDEX, Indexes>::type>()(0, 0))
		)
	{
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		typedef typename std::tuple_element<INDEX, Indexes>::type KeyType;

		return xNodeFromAvl<INDEX>(AvlFind(
			&(xm_arvIndexPointers[INDEX].pRoot),
			(std::intptr_t)&vComparand,
			xComparer<KeyType, Comparand_t>(),
			xComparer<Comparand_t, KeyType>()
		));
	}

	template<std::size_t INDEX, typename Comparand_t>
	std::pair<const Node *, const Node *> GetEqualRange(const Comparand_t &vComparand) const
		noexcept(
			noexcept	(xComparer<typename std::tuple_element<INDEX, Indexes>::type, Comparand_t>()(0, 0))
			&& noexcept	(xComparer<Comparand_t, typename std::tuple_element<INDEX, Indexes>::type>()(0, 0))
		)
	{
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		typedef typename std::tuple_element<INDEX, Indexes>::type KeyType;

		AvlNodeHeader *pFrom, *pTo;
		AvlEqualRange(
			&pFrom,
			&pTo,
			&(xm_arvIndexPointers[INDEX].pRoot),
			(std::intptr_t)&vComparand,
			xComparer<KeyType, Comparand_t>(),
			xComparer<Comparand_t, KeyType>()
		);
		return std::make_pair(
			xNodeFromAvl<INDEX>(pFrom),
			xNodeFromAvl<INDEX>(pTo)
		);
	}
	template<std::size_t INDEX, typename Comparand_t>
	std::pair<Node *, Node *> GetEqualRange(const Comparand_t &vComparand)
		noexcept(
			noexcept	(xComparer<typename std::tuple_element<INDEX, Indexes>::type, Comparand_t>()(0, 0))
			&& noexcept	(xComparer<Comparand_t, typename std::tuple_element<INDEX, Indexes>::type>()(0, 0))
		)
	{
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		typedef typename std::tuple_element<INDEX, Indexes>::type KeyType;

		AvlNodeHeader *pFrom, *pTo;
		AvlEqualRange(
			&pFrom,
			&pTo,
			&(xm_arvIndexPointers[INDEX].pRoot),
			(std::intptr_t)&vComparand,
			xComparer<KeyType, Comparand_t>(),
			xComparer<Comparand_t, KeyType>()
		);
		return std::make_pair(
			xNodeFromAvl<INDEX>(pFrom),
			xNodeFromAvl<INDEX>(pTo)
		);
	}
};

}

#endif
