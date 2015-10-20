#include "../vectup.hpp"
#include <iostream>
#include "typetostr.h"

using namespace std;
using namespace uniformcoll;

template<typename T1, typename T2>
std::ostream &operator<<(std::ostream &os, const std::pair<T1,T2> &v) {
	return os << '<' << v.first << ',' << v.second << '>';
}

template<typename E>
std::ostream &operator<<(std::ostream &os, const std::vector<E> &v) {
	os << '[';
	for(size_t i=0;i<v.size();i++) {
		if (i) os << ',';
		os << v[i];
	}
	return os << ']';
}

struct printfn {
	template<typename T>
	pair<std::ostream &,bool> operator()(
			pair<std::ostream &,bool> in, const T &t) {
		if (in.second) in.first << ',';
		in.first << t;
		return {in.first,true};
	}
};

template<typename... Ts>
std::ostream &operator<<(std::ostream &os, const vectup<staticalloc,Ts...> &v) {
	os << '<';
	foldl(printfn{},pair<std::ostream &,bool>{os,false},v);
	return os << '>';
}

template<typename T, int N>
std::ostream &operator<<(std::ostream &os, const vectup<N,T> &v) {
	os << '(';
	for(size_t i=0;i<v.size();i++) {
		if (i) os << ',';
		os << v[i];
	}
	return os << ')';
}

struct makepair {
	template<typename T, typename S>
	constexpr auto
	operator()(T &&t, S &&s) const {
		return std::make_pair(std::forward<T>(t),std::forward<S>(s));
	}
};

struct aresame {
	template<typename T, typename S>
	constexpr bool operator()(const T &t, const S &s) const {
		return false;
	}

	template<typename T>
	constexpr bool operator()(const T &t, const T &s) const {
		return t==s;
	}
};

template<typename T>
std::string tostring(T &&t) { return std::to_string(std::forward<T>(t)); }

std::string tostring(std::string s) { return s; }
std::string tostring(char c) { return {c}; }

struct concat {
	template<typename T1, typename T2>
	std::string operator()(const T1 &x1, const T2 &x2) const {
		return tostring(x1)+tostring(x2);
	}
};

template<typename T>
void check(const std::string s, const T &t) {
	cout << s << ": " << endl;
	cout << '\t' << typetostr<T>() << endl;
	cout << '\t' << t << endl;
}

int main(int argc, char **argv) {
	vectup<dynamicalloc,char> c1{'a','b','c','z'};
	cout << c1 << endl;
	for(auto &x : c1) cout << x << ' ';
	cout << endl;

	constexpr vectup<staticalloc,char,double,char,int> c2{'!',3.14,'c',3};
	cout << c2 << endl;

	constexpr vectup<5,int> c3{1,1,2,3,5};
	for(auto &x : c3) cout << x << ' ';
	cout << endl;

	check("make pair 1&1"s, zip(makepair{},c1,c1));
	check("make pair 1&2"s, zip(makepair{},c1,c2));
	check("make pair 1&3"s, zip(makepair{},c1,c3));
	check("make pair 2&1"s, zip(makepair{},c2,c1));
	check("make pair 2&2"s, zip(makepair{},c2,c2));
	check("make pair 2&3"s, zip(makepair{},c2,c3));
	check("make pair 3&1"s, zip(makepair{},c3,c1));
	check("make pair 3&2"s, zip(makepair{},c3,c2));
	check("make pair 3&3"s, zip(makepair{},c3,c3));

	check("are same 1&1"s, zip(aresame{},c1,c1));
	check("are same 1&2"s, zip(aresame{},c1,c2));
	check("are same 1&3"s, zip(aresame{},c1,c3));
	check("are same 2&1"s, zip(aresame{},c2,c1));
	check("are same 2&2"s, zip(aresame{},c2,c2));
	check("are same 2&3"s, zip(aresame{},c2,c3));
	check("are same 3&1"s, zip(aresame{},c3,c1));
	check("are same 3&2"s, zip(aresame{},c3,c2));
	check("are same 3&3"s, zip(aresame{},c3,c3));

	check("foldl concat 1"s, foldl(concat{},""s,c1));
	check("foldr concat 1"s, foldr(concat{},""s,c1));
	check("foldrflip concat 1"s, foldrflip(concat{},""s,c1));
	check("foldl concat 2"s, foldl(concat{},""s,c2));
	check("foldr concat 2"s, foldr(concat{},""s,c2));
	check("foldrflip concat 2"s, foldrflip(concat{},""s,c2));
	check("foldl concat 3"s, foldl(concat{},""s,c3));
	check("foldr concat 3"s, foldr(concat{},""s,c3));
	check("foldrflip concat 3"s, foldrflip(concat{},""s,c3));

	cout << typetostr<make_rev_index_sequence<0>>() << endl;
}
