#ifndef VECTUP_H
#define VECTUP_H

#include <vector>
#include <tuple>

namespace uniformcoll {

// a general ordered collection
// can be a vector (if length not known at compile time)
// or a tuple (if element types known at compile time)
// in the latter case, all operations are constexpr
//
// C++14 only
// Will be even more useful once template lambdas are allowed in constexpr

using alloctype = int;

const alloctype staticalloc = -1;
const alloctype dynamicalloc = -2;

template<typename...> struct typeset;

template<alloctype,typename...> struct vectup;

template<bool V, typename T=void>
using enable = typename std::enable_if<V,T>::type;

template<typename ET>
struct vectup<dynamicalloc,ET> : public std::vector<ET> {
	using std::vector<ET>::vector;
	typedef ET datatype;

	template<std::size_t I>
	constexpr auto get() const { return (*this).at(I); }
	template<std::size_t I>
	constexpr auto get() { return (*this)[I]; }
};

template<typename ET0, typename... ETS>
struct vectup<staticalloc,ET0,ETS...> : public std::tuple<ET0,ETS...> {
	using std::tuple<ET0,ETS...>::tuple;
	
	typedef typeset<ET0,ETS...> datatype;

	template<std::size_t I>
	constexpr auto get() const { return std::get<I>(*this); }
	template<std::size_t I>
	constexpr auto get() { return std::get<I>(*this); }

	constexpr auto front() const { return std::get<0>(*this); }
	constexpr auto front() { return std::get<0>(*this); }
	constexpr auto back() const { return std::get<std::tuple_size<std::tuple<ET0,ETS...>>::value-1>(*this); }
	constexpr auto back() { return std::get<std::tuple_size<std::tuple<ET0,ETS...>>::value-1>(*this); }

	constexpr std::size_t size() const { return std::tuple_size<std::tuple<ET0,ETS...>>::value; }
};

template<>
struct vectup<staticalloc> : public std::tuple<> {
	using std::tuple<>::tuple;

	typedef typeset<> datatype;

	constexpr std::size_t size() const { return 0; }
};

template<alloctype N,typename ET>
struct vectup<N,ET> {

	typedef ET datatype;

	ET data[N];

	template<typename...T>
	constexpr vectup(T &&...x) : data{std::forward<T>(x)...} {}

	constexpr auto begin() const { return std::begin(data); }
	constexpr auto begin() { return std::begin(data); }
	constexpr auto end() const { return std::end(data); }
	constexpr auto end() { return std::end(data); }
	constexpr auto front() const { return data[0]; }
	constexpr auto front() { return data[0]; }
	constexpr auto back() const { return data[N-1]; }
	constexpr auto back() { return data[N-1]; }

	template<std::size_t I>
	constexpr auto get() const { return data[I]; }
	template<std::size_t I>
	constexpr auto get() { return data[I]; }

	constexpr auto operator[](std::size_t i) const { return data[i]; }
	constexpr auto operator[](std::size_t i) { return data[i]; }

	constexpr std::size_t size() const { return N; }
};

namespace {

	template<typename F, typename ET1, typename ET2> struct allsame {
		static const bool value = true;
		typedef decltype(std::declval<F>()(std::declval<ET1>(),std::declval<ET2>())) type;
	};

	template<typename F, typename ET1, typename ET2, typename... Ts>
	struct allsame<F,ET1,typeset<ET2,Ts...>> {
		typedef decltype(std::declval<F>()(std::declval<ET1>(),std::declval<ET2>())) type;
		typedef allsame<F,ET1,typeset<Ts...>> rectype;
		static const bool value = sizeof...(Ts)==0 ||
				(std::is_same<type,typename rectype::type>::value && rectype::value);
	};

	template<typename F, typename ET1, typename ET2, typename... Ts>
	struct allsame<F,typeset<ET1,Ts...>,ET2> {
		typedef decltype(std::declval<F>()(std::declval<ET1>(),std::declval<ET2>())) type;
		typedef allsame<F,typeset<Ts...>,ET2> rectype;
		static const bool value = sizeof...(Ts)==0 ||
				(std::is_same<type,typename rectype::type>::value && rectype::value);
	};

	template<typename F, typename ET1, typename ET2, typename... T1s, typename... T2s>
	struct allsame<F,typeset<ET1,T1s...>,typeset<ET2,T2s...>> {
		typedef decltype(std::declval<F>()(std::declval<ET1>(),std::declval<ET2>())) type;
		typedef allsame<F,typeset<T1s...>,typeset<T2s...>> rectype;
		static const bool value = sizeof...(T1s)==0 || sizeof...(T2s)==0 || 
				(std::is_same<type,typename rectype::type>::value && rectype::value);
	};

	template<typename F>
	struct allsame<F,typeset<>,typeset<>> {
		typedef void type;
		static const bool value = true;
	};

	template<typename F, typename ET>
	struct allsame<F,typeset<>,ET> {
		typedef void type;
		static const bool value = true;
	};

	template<typename F, typename ET>
	struct allsame<F,ET,typeset<>> {
		typedef void type;
		static const bool value = true;
	};

	template<typename F, typename E1, typename E2>
	constexpr bool allsame_v
		= allsame<std::decay_t<F>,typename E1::datatype,typename E2::datatype>::value;
	template<typename F, typename E1, typename E2>
	using allsame_t
		= typename allsame<std::decay_t<F>,typename E1::datatype,typename E2::datatype>::type;
		
	template <std::size_t... I, typename F, typename VT1, typename VT2>
	constexpr auto
	zipNarr(F &&f, const VT1 &c1, const VT2 &c2, std::index_sequence<I...>) {
		return vectup<sizeof...(I), allsame_t<F,VT1,VT2>>
				{f(c1.template get<I>(),c2.template get<I>())...};
	}

	template <std::size_t... I, typename F, typename VT1, typename VT2,
			typename Enable=enable<!allsame_v<F,VT1,VT2>>>
	constexpr auto
	zipN(F &&f, const VT1 &c1, const VT2 &c2, std::index_sequence<I...>) {
		return vectup<staticalloc,
					decltype(f(c1.template get<I>(),c2.template get<I>()))...
				>
			{f(c1.template get<I>(),c2.template get<I>())...};
	}

	template <std::size_t... I, typename F, typename VT1, typename VT2,
			typename Enable=enable<allsame_v<F,VT1,VT2>>>
	constexpr auto
	zipN(F &&f, const VT1 &c1, const VT2 &c2, const std::index_sequence<I...> &i) {
		return zipNarr(std::forward<F>(f),c1,c2,i);
	}

	template<typename F, typename VT1, typename VT2>
	constexpr auto
	zipdyn(F &&f, const VT1 &c1, const VT2 &c2) {
		vectup<dynamicalloc,allsame_t<F,VT1,VT2>> ret;
		std::size_t len = std::min(c1.size(),c2.size());
		ret.reserve(len);
		for(std::size_t i=0;i<len;i++)
			ret.emplace_back(f(c1[i],c2[i]));
		return ret;
	}
}

template<typename F, typename ARG1, typename ARG2>
constexpr auto
zip(F &&f,
		const vectup<dynamicalloc,ARG1> &c1,
		const vectup<dynamicalloc,ARG2> &c2) {
	return zipdyn(std::forward<F>(f),c1,c2);
}

template<typename F, typename ARG1, typename... ARG2>
constexpr auto
zip(F &&f,
		const vectup<dynamicalloc,ARG1> &c1,
		const vectup<staticalloc,ARG2...> &c2) {
	return zipN(std::forward<F>(f),c1,c2,std::make_index_sequence<sizeof...(ARG2)>{});
}

template<typename F, typename... ARG1, typename ARG2>
constexpr auto
zip(F &&f,
		const vectup<staticalloc,ARG1...> &c1,
		const vectup<dynamicalloc,ARG2> &c2) {
	return zipN(std::forward<F>(f),c1,c2,std::make_index_sequence<sizeof...(ARG1)>{});
}

template<typename F, typename... ARG1, typename... ARG2>
constexpr auto
zip(F &&f,
		const vectup<staticalloc,ARG1...> &c1,
		const vectup<staticalloc,ARG2...> &c2) {
	return zipN(std::forward<F>(f),c1,c2,std::make_index_sequence<
			std::min(sizeof...(ARG1),sizeof...(ARG2))>{});
}

template<typename F, typename... ARG1, typename ARG2, int N,
	typename Enable=enable<0<=N>>
constexpr auto
zip(F &&f,
		const vectup<dynamicalloc,ARG1...> &c1,
		const vectup<N,ARG2> &c2) {
	return zipdyn(std::forward<F>(f),c1,c2);
}

template<typename F, typename ARG1, typename... ARG2, int N,
	typename Enable=enable<0<=N>>
constexpr auto
zip(F &&f,
		const vectup<N,ARG1> &c1,
		const vectup<dynamicalloc,ARG2...> &c2) {
	return zipdyn(std::forward<F>(f),c1,c2);
}

template<typename F, typename... ARG1, typename ARG2, int N,
	typename Enable=enable<0<=N>>
constexpr auto
zip(F &&f,
		const vectup<staticalloc,ARG1...> &c1,
		const vectup<N,ARG2> &c2) {
	return zipN(std::forward<F>(f),c1,c2,std::make_index_sequence<
		std::min(sizeof...(ARG1),(std::size_t)(N))>{});
}

template<typename F, typename ARG1, typename... ARG2, int N,
	typename Enable=enable<0<=N>>
constexpr auto
zip(F &&f,
		const vectup<N,ARG1> &c1,
		const vectup<staticalloc,ARG2...> &c2) {
	return zipN(std::forward<F>(f),c1,c2,std::make_index_sequence<
		std::min((std::size_t)(N),sizeof...(ARG2))>{});
}

template<typename F, typename ARG1, typename ARG2, int N1, int N2,
	typename Enable=enable<0<=N1 && 0<=N2>>
constexpr auto
zip(F &&f,
		const vectup<N1,ARG1> &c1,
		const vectup<N2,ARG2> &c2) {
	return zipNarr(std::forward<F>(f),c1,c2,std::make_index_sequence<
		(std::size_t)(std::min(N1,N2))>{});
}


namespace{

	template<typename F, typename T, typename VT>
	constexpr auto
	foldlimpl(F &&f, T x, const VT &c, std::index_sequence<>) {
		return x;
	}

	template<typename F, typename T, typename VT, std::size_t I0, std::size_t... I>
	constexpr auto
	foldlimpl(F &&f, T x, const VT &c, std::index_sequence<I0,I...>) {
		return foldlimpl(std::forward<F>(f),f(x,c.template get<I0>()),c,
				std::index_sequence<I...>{});
	}

	template<std::size_t... I>
	struct nextrevindlst {
		typedef nextrevindlst<sizeof...(I),I...> type;
	};

	template<std::size_t I>
	struct revindlst {
		typedef typename revindlst<I-1>::type::type type;
	};

	template<>
	struct revindlst<0> {
		typedef nextrevindlst<> type;
	};

	template<typename> struct makerevseq {};

	template<std::size_t... I>
	struct makerevseq<nextrevindlst<I...>> {
		typedef std::index_sequence<I...> type;
	};

	template<std::size_t I>
	using make_rev_index_sequence = typename makerevseq<typename revindlst<I>::type>::type;

	template<typename F, typename T, typename VT>
	constexpr auto
	foldrimpl(F &&f, T x, const VT &c, std::index_sequence<>) {
		return x;
	}

	template<typename F, typename T, typename VT, std::size_t I0, std::size_t... I>
	constexpr auto
	foldrimpl(F &&f, T x, const VT &c, std::index_sequence<I0,I...>) {
		return foldrimpl(std::forward<F>(f),f(c.template get<I0>(),x),c,
				std::index_sequence<I...>{});
	}

	template<typename F, typename T, typename VT>
	constexpr auto
	foldrflipimpl(F &&f, T x, const VT &c, std::index_sequence<>) {
		return x;
	}

	template<typename F, typename T, typename VT, std::size_t I0, std::size_t... I>
	constexpr auto
	foldrflipimpl(F &&f, T x, const VT &c, std::index_sequence<I0,I...>) {
		return foldrflipimpl(std::forward<F>(f),f(x,c.template get<I0>()),c,
				std::index_sequence<I...>{});
	}

}

template<typename F, typename T, typename VT>
constexpr std::decay_t<T>
foldl(F &&f, T x, const VT &c) {
	for(auto &&e : c)
		x = f(std::move(x),e);
	return x;
}

template<typename F, typename T, typename... Xs>
constexpr auto
foldl(F &&f, T x, const vectup<staticalloc,Xs...> &c) {
	return foldlimpl(std::forward<F>(f),std::move(x),c,
			std::make_index_sequence<sizeof...(Xs)>{});
}

template<typename F, typename T, typename VT>
constexpr std::decay_t<T>
foldr(F &&f, T x, const VT &c) {
	for(std::size_t i=c.size();i>0;i--)
		x = f(c[i-1],std::move(x));
	return x;
}

template<typename F, typename T, typename VT>
constexpr std::decay_t<T>
foldrflip(F &&f, T x, const VT &c) {
	for(std::size_t i=c.size();i>0;i--)
		x = f(std::move(x),c[i-1]);
	return x;
}


template<typename F, typename T, typename... Xs>
constexpr auto
foldr(F &&f, T x, const vectup<staticalloc,Xs...> &c) {
	return foldrimpl(std::forward<F>(f),std::move(x),c,
			make_rev_index_sequence<sizeof...(Xs)>{});
}


template<typename F, typename T, typename... Xs>
constexpr auto
foldrflip(F &&f, T x, const vectup<staticalloc,Xs...> &c) {
	return foldrflipimpl(std::forward<F>(f),std::move(x),c,
			make_rev_index_sequence<sizeof...(Xs)>{});
}

} // end vectup namespace

#endif
