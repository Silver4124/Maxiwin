#if defined(_WIN32)
#define _MAXIWIN_H_

/*
IN CASE OF USE GCC/GNU COMPILER!!!!!!!!! INCLUDE THIS FLAGS IN THE COMPILER LINE!!!!!!!!!!
-lwinmm -lgdi32 -lgdiplus
*/

#ifndef SIMD_DIRECTIVE
#if defined(__clang__)
  #define SIMD_DIRECTIVE _Pragma("clang loop vectorize(enable)")
  #define _MAXIWIN_H_SIMD_MSG "Using: clang loop vectorize(enable)"
#elif defined(__GNUC__) || defined(__GNUG__)
  #define SIMD_DIRECTIVE _Pragma("GCC ivdep")
  #define _MAXIWIN_H_SIMD_MSG "Using: GCC ivdep"
#elif defined(_MSC_VER)
  #define SIMD_DIRECTIVE __pragma(loop(ivdep))
  #define _MAXIWIN_H_SIMD_MSG "Using: MSVC loop(ivdep)"
#else
  #define SIMD_DIRECTIVE
  #define _MAXIWIN_H_SIMD_MSG "NO SIMD_DIRECTIVE DETECTED!"
#endif
#pragma message(_MAXIWIN_H_SIMD_MSG)
#endif

#include<stdlib.h>
#include<stdio.h>

#ifdef _WIN64
using size_t=unsigned long long;using ssize_t=long long;
#else
using size_t=unsigned int;using ssize_t=int;
#endif
using uint64_t=unsigned long long;using uint32_t=unsigned int;
using uint16_t=unsigned short;using uint8_t=unsigned char;
using int64_t=ssize_t;using int32_t=int;using int16_t=short;
using int8_t=signed char;

namespace mini_traits{
	template<typename T,size_t N>size_t array_size(const T(&)[N]){return N;}
	struct true_type{static constexpr bool value=true;};struct false_type{static constexpr bool value=false;};
	template<bool B,typename T=void>struct enable_if{};template<typename T>struct enable_if<true,T>{using type=T;};
	template<bool B,typename T=void>using enable_if_t=typename enable_if<B,T>::type;
	template<typename T>T&& declval()noexcept;template<typename F, typename... Args> struct is_invocable{private:
		template<typename U>static auto test(U*)->decltype(declval<U>()(declval<Args>()...),true_type{});
		template<typename>static auto test(...)->false_type;public:
		static constexpr bool value=decltype(test<F>(nullptr))::value;};
	template<typename F,typename... Args>constexpr bool is_invocable_v=is_invocable<F, Args...>::value;
	template<typename T,typename U>struct is_same{static constexpr bool value = false;};
	template<typename T>struct is_same<T,T>{static constexpr bool value=true;};
	template<typename T,typename U>constexpr bool is_same_v = is_same<T,U>::value;
	template<typename T>struct remove_cv_ref{using type=T;};
	template<typename T>struct remove_cv_ref<const T>{using type=typename remove_cv_ref<T>::type;};
	template<typename T>struct remove_cv_ref<volatile T>{using type=typename remove_cv_ref<T>::type;};
	template<typename T>struct remove_cv_ref<const volatile T>{using type=typename remove_cv_ref<T>::type;};
	template<typename T>struct remove_cv_ref<T&>{using type=typename remove_cv_ref<T>::type;};
	template<typename T>struct remove_cv_ref<T&&>{using type=typename remove_cv_ref<T>::type;};
	template<typename T,typename U>constexpr bool is_same_cvref_type=is_same<typename remove_cv_ref<T>::type,
		typename remove_cv_ref<U>::type>::value;
	template<typename T,typename... Ts>struct is_same_value_impl{static constexpr bool value=true;};
	template<typename T,typename U,typename... Ts>struct is_same_value_impl<T, U, Ts...>{
		static constexpr bool value=is_same_cvref_type<T,U>&&is_same_value_impl<T,Ts...>::value;};
	template<typename T,typename... Ts>constexpr bool is_same_value=is_same_value_impl<T, Ts...>::value;
	template<typename T,typename... Ts>struct is_same_or_value_impl{static constexpr bool value = true;};
	template<typename T,typename U,typename... Ts>struct is_same_or_value_impl<T, U, Ts...>{
		static constexpr bool value = is_same_cvref_type<T, U> || is_same_or_value_impl<T, Ts...>::value; };
	template<typename T,typename... Ts>constexpr bool is_same_or_value = is_same_or_value_impl<T, Ts...>::value;
	template<typename T>constexpr bool is_numerical_type=is_same_cvref_type<T,short>||
		is_same_cvref_type<T,signed short>||is_same_cvref_type<T,unsigned short>||is_same_cvref_type<T,int>||
		is_same_cvref_type<T,signed int>||is_same_cvref_type<T,unsigned int>||is_same_cvref_type<T,long>||
		is_same_cvref_type<T,signed long>||is_same_cvref_type<T,unsigned long>||
		is_same_cvref_type<T,long long>||is_same_cvref_type<T,signed long long>||
		is_same_cvref_type<T,unsigned long long>||is_same_cvref_type<T,float>||is_same_cvref_type<T,double>||
		is_same_cvref_type<T,long double>;
	template<typename... Ts>struct all_numerical_value_impl;
	template<>struct all_numerical_value_impl<>{static constexpr bool value=true;};
	template<typename T, typename... Ts>struct all_numerical_value_impl<T, Ts...>{
	    static constexpr bool value=is_numerical_type<T>&&all_numerical_value_impl<Ts...>::value;};
	template<typename... Ts>constexpr bool all_numerical_value=all_numerical_value_impl<Ts...>::value;
	template<typename... Ts>struct any_numerical_value_impl;
	template<>struct any_numerical_value_impl<>{static constexpr bool value = false;};
	template<typename T, typename... Ts>struct any_numerical_value_impl<T, Ts...>{
	    static constexpr bool value=is_numerical_type<T>||any_numerical_value_impl<Ts...>::value;};
	template<typename... Ts>constexpr bool any_numerical_value=any_numerical_value_impl<Ts...>::value;
	template<typename T>constexpr bool is_char_type = is_same_cvref_type<T,char>||is_same_cvref_type<T,signed char>||
		is_same_cvref_type<T,unsigned char>||is_same_cvref_type<T,wchar_t>
#if __cplusplus >= 202002L
||is_same_cvref_type<T,char8_t>||is_same_cvref_type<T,char16_t>||is_same_cvref_type<T,char32_t>
#endif
;template<typename... Ts>struct all_char_value_impl;template<>struct all_char_value_impl<>{static constexpr bool value=true;};
	template<typename T,typename... Ts>struct all_char_value_impl<T,Ts...>{static constexpr bool value=
		is_char_type<T>&&all_char_value_impl<Ts...>::value;};
	template<typename... Ts>constexpr bool all_char_value=all_char_value_impl<Ts...>::value;
	template<typename... Ts>struct any_char_value_impl;template<>struct any_char_value_impl<>{static constexpr bool value = false;};
	template<typename T,typename... Ts>struct any_char_value_impl<T,Ts...>{static constexpr bool value=is_char_type<T>||
		any_char_value_impl<Ts...>::value;};template<typename... Ts>constexpr bool any_char_value=any_char_value_impl<Ts...>::value;
	template<typename T, size_t N>struct initializer_list{T data[N];operator T*(){return data;}operator const T*()const{return data;}};
	template<typename T, size_t N>constexpr initializer_list<T, N> make_list(const T(&arr)[N]){return initializer_list<T,N>(arr);}
	template<size_t Index,typename T,typename... Rest>struct GetArgument{static constexpr auto value(T,Rest... rest){
		return GetArgument<Index-1,Rest...>::value(rest...);}};
	template<typename T,typename... Rest>struct GetArgument<0,T,Rest...>{static constexpr T value(T primero,Rest...){return primero;}};
	template<size_t Index,typename... Args>constexpr auto get_argument_impl(Args... args){return GetArgument<Index,Args...>::value(args...);}
	template<bool Cond>struct if_constexpr_evaluator{template<typename F,typename... Args>static void eval(F&&,Args&&...){}};
	template<>struct if_constexpr_evaluator<true>{template<typename F,typename... Args>static void eval(F&& f,Args&&... args){f(args...);}};
	template<bool B,typename Func,typename... Args>void if_constexpr(Func&& f,Args&&... args){if_constexpr_evaluator<B>::eval(f,args...);}
	struct EmptyTuple{};template<size_t Index,typename T>struct TupleElement{T value;
		TupleElement()=default;TupleElement(const T& val):value(val){}T& get(){return value;}const T& get()const{return value;}};
	template<size_t Index,typename... Types>struct TupleImpl;template<size_t Index,typename Head, typename... Tail>
	struct TupleImpl<Index, Head, Tail...>:TupleElement<Index,Head>,TupleImpl<Index+1,Tail...>{TupleImpl()=default;
		TupleImpl(const Head& head,const Tail&... tail):TupleElement<Index, Head>(head),TupleImpl<Index+1,Tail...>(tail...){}};
	template<size_t Index>struct TupleImpl<Index>:EmptyTuple{};
	template<typename... Types>struct Tuple:TupleImpl<0,Types...>{using Impl=TupleImpl<0,Types...>;Tuple()=default;
		Tuple(const Types&... args):Impl(args...){}private:template<size_t N,typename ImplT>struct Accessor{
			static_assert(N!=N,"Index out of bounds in Tuple<>");};template<size_t N,typename Head,typename... Tail>struct Accessor
			<N,TupleImpl<0,Head,Tail...>>:Accessor<N-1,TupleImpl<1,Tail...>>{};template<typename Head,typename... Tail>struct Accessor
			<0,TupleImpl<0,Head,Tail...>>{using Type = Head;static Head& get(TupleImpl<0,Head,Tail...>& tuple){
			return static_cast<TupleElement<0, Head>&>(tuple).get();}static const Head& get(const TupleImpl<0,Head,Tail...>& tuple)
			{return static_cast<const TupleElement<0,Head>&>(tuple).get();}
		};public:template<size_t N>auto& get(){return Accessor<N,Impl>::get(*this);}
		template<size_t N>const auto& get()const{return Accessor<N,Impl>::get(*this);}
		template<size_t N,typename... T>friend auto& Tuple_get(Tuple<T...>& tuple);
		template<size_t N,typename... T>friend const auto& Tuple_get(const Tuple<T...>& tuple);
	};template<size_t N,typename... T>auto& Tuple_get(Tuple<T...>& tuple){return tuple.template get<N>();}
	template<size_t N,typename... T>const auto& Tuple_get(const Tuple<T...>& tuple){return tuple.template get<N>();}
	template<>struct Tuple<>:TupleImpl<0>{using Impl=TupleImpl<0>;Tuple() = default;template<size_t N>auto& get()
			{static_assert(N != N, "Error: acceso invalido a Indice en Tuple<> vacIa.");return *reinterpret_cast<void*>(0);}
	    template<size_t N>const auto& get()const{static_assert(N != N, "Error: acceso invalido a Indice en Tuple<> vacIa.");}};
	template<size_t N,typename... T>constexpr auto& get_argument_impl(Tuple<T...>& tuple){return tuple.template get<N>();}
	template<size_t N,typename... T>constexpr const auto& get_argument_impl(const Tuple<T...>& tuple){return tuple.template get<N>();}
	#define GET_ARGUMENT(N, ...) ([&]()->decltype(auto){return mini_traits::get_argument_impl<N>(__VA_ARGS__);}())
	struct input_iterator_tag{};struct output_iterator_tag{};struct forward_iterator_tag:input_iterator_tag{};
	struct bidirectional_iterator_tag:forward_iterator_tag{};struct random_access_iterator_tag:bidirectional_iterator_tag{};
	template<typename Iterator>struct iterator_traits{using value_type=typename Iterator::value_type;
	    using difference_type=typename Iterator::difference_type;using pointer=typename Iterator::pointer;
	    using reference=typename Iterator::reference;using iterator_category=typename Iterator::iterator_category;};
	template<typename T>struct iterator_traits<T*>{using value_type=T;using difference_type=int64_t;using pointer=T*;
		using reference=T&;using iterator_category=random_access_iterator_tag;};
	template<typename T>struct iterator_traits<const T*>{using value_type=T;using difference_type=int64_t;using pointer=const T*;
	    using reference=const T&;using iterator_category=random_access_iterator_tag;};
}
template<typename T> class DynArrayBase{T* _data;size_t _size;
    template<typename Iterator>void initialize_from(Iterator begin,Iterator end){if(!begin || !end){_size = 0;_data = nullptr;return;}
	_size = static_cast<size_t>(end - begin);if(_size>0){_data=new T[_size];size_t i = 0;for(auto it=begin;it!=end;++it,++i)_data[i]=*it;}}
	public:DynArrayBase():_data(nullptr),_size(0){} DynArrayBase(decltype(nullptr)):DynArrayBase(){}
	DynArrayBase(const DynArrayBase& other):_data(nullptr),_size(0){if(other._size > 0){_size=other._size;_data=new T[_size];
		for(size_t i=0;i<_size;++i)_data[i]=other._data[i];}}
	DynArrayBase(DynArrayBase&& other)noexcept:_data(other._data),_size(other._size){other._data = nullptr;other._size = 0;}
	DynArrayBase(const T& init):_data(new T[1]{init}),_size(1){}
	template<typename P>DynArrayBase(P* begin,P* end):_data(nullptr),_size(0){initialize_from(begin, end);}
	template<typename P>DynArrayBase(const DynArrayBase<P>& other):DynArrayBase(other.begin(),other.end()){}
	template<size_t N>DynArrayBase(const T(&arr)[N]):_data(new T[N]),_size(N){for(size_t i=0;i<N;i++)_data[i]=arr[i];}
	DynArrayBase(size_t count,const T& value=T()):_data(nullptr),_size(count){_data=
		new T[_size]{value};}DynArrayBase& operator=(const DynArrayBase& other){if(this!=&other){if(_data){for(size_t i=0;i<_size;++i)
		_data[i].~T();delete[] _data;}_size=other._size;_data=(_size>0)?new T[_size]:nullptr;for(size_t i=0;i<_size;++i)_data[i]=
		other._data[i];}return *this;}DynArrayBase& operator=(DynArrayBase&& other)noexcept{if(this!=&other){if(_data){for(size_t i=0;
		i<_size;++i)_data[i].~T();delete[] _data;}_data=other._data;_size=other._size;other._data = nullptr;other._size = 0;}
		return *this;}template<size_t N>DynArrayBase& operator=(const T (&array)[N]){if(_data != array){if(_data){for(size_t i=0;i<_size;
		++i)_data[i].~T();delete[] _data;}_size=N;_data=(_size>0)?new T[_size]:nullptr;for(size_t i=0;i<_size;++i)_data[i]=array[i];}
		return *this;}DynArrayBase& operator+=(const DynArrayBase& other){if(this==&other)return *this;if(_data){size_t newSize=
		_size+other._size;T* newData=new T[newSize];for(size_t i=0;i<_size;i++){newData[i]=_data[i];_data[i].~T();}
		for(size_t i=0;i<other._size;i++)newData[i+_size]=other._data[i];delete[] _data;_data=newData;_size=newSize;}else(*this)=other;
		return *this;}template<size_t N>DynArrayBase& operator+=(const T (&array)[N]){if(_data==array)return *this;if(_data){
		size_t newSize=_size+N;T* newData=new T[newSize];for(size_t i=0;i<_size;i++){newData[i]=_data[i];_data[i].~T();}
		for(size_t i=0;i<N;i++)newData[i+_size]=array[i];delete[] _data;_data=newData;_size=newSize;}else(*this)=array;return *this;}
	DynArrayBase operator+(const DynArrayBase& other){DynArrayBase<T> r=*this;r+=other;return r;}
	template<size_t N>DynArrayBase operator+(const T (&array)[N]){DynArrayBase<T> r=*this;r+=array;return r;}
	T& operator[](size_t i){return _data[i];} const T& operator[](size_t i) const{return _data[i];}size_t& size(){return _size;}
	const size_t& size()const{return _size;}bool empty()const{return _size==0;} T* data()const{return _data;} T* begin(){return _data;}
	const T* begin()const{return _data;}T* end(){return _data+_size;} const T* end()const{return _data+_size;}
	T& top(){return _data[0];}const T& top()const{return _data[0];}T& back(){return _data[_size-1];}
	const T& back()const{return _data[_size-1];}void clear(){if(_data){for(size_t i=0;i<_size;++i)_data[i].~T();delete[] _data;}
		_data=nullptr;_size=0;}void set(const T& value){for(size_t i=0;i<_size;i++)_data[i]=value;}
	void resize(size_t new_size){if(new_size==_size)return;T* newData = new T[new_size]{T()};size_t minSize=(_size<new_size)?_size:new_size;
    	for(size_t i=0;i<minSize;++i){newData[i]=_data[i];_data[i].~T();}if(_data)delete[] _data;_data=newData;_size=new_size;}
	void erase(size_t index,size_t count=1){if(count==0||index>=_size||(index+count)>_size){clear();return;}size_t new_size=_size-count;
		T* newData=new T[new_size];for(size_t i=0;i<index;++i)newData[i]=_data[i];for(size_t i=index+count;i<_size;++i)newData[i-count]=
		_data[i];resize(new_size);if(_data)delete[] _data;_data=newData;}void pop(){if(_size!=0)erase(_size-1,1);}
	void emplace(T* begin, T* end){clear();initialize_from(begin,end);}template<size_t N>void emplace(T (&array)[N]){return
		emplace(array, array + N);}~DynArrayBase(){clear();} void new_back(){resize(_size + 1);} void push_back(const T& value){
		resize(_size + 1);back()=value;}template<typename... Args>void emplace_back(Args&&... args){resize(_size + 1);back()=T(args...);}
	template<size_t N>bool operator==(const T (&values)[N]) const{if(_size!=N)return false;if(_data==values && values==nullptr)return true;
	    if(!_data || !values)return false;bool equals=true;for(size_t i=0;i<_size;i++)equals=equals && _data[i]==values[i];return equals;}
	template<typename P,size_t N>bool operator==(const P (&values)[N])const{return false;}
	bool operator==(const DynArrayBase& other)const{if(_size!=other._size)return false;if(_data==other._data && other._data==nullptr)
		return true;if(!_data || !other._data)return false;bool equals=true;for(size_t i=0;i<_size;i++)equals=equals && _data[i]==
		other._data[i];return equals;}template<typename P>bool operator==(const DynArrayBase<P>& other)const{return false;}
	template<typename P,size_t N>bool operator!=(const P (&values)[N])const{return !(*this==values);}
	template<typename P>bool operator!=(const DynArrayBase<P>& other)const{return !(*this==other);}
	template<size_t N>explicit operator mini_traits::initializer_list<T,N>()const{
        mini_traits::initializer_list<T, N> arr{};
        if(_size != N)throw "Tamaño no coincide";
        for(size_t i = 0; i < N; ++i)arr.data[i] = _data[i];
        return arr;
    }
};
template<typename T>class DynArray:public DynArrayBase<T>{public:using DynArrayBase<T>::DynArrayBase;};
template<>class DynArray<char>:public DynArrayBase<char>{public:using DynArrayBase<char>::DynArrayBase;
    DynArray(const wchar_t* begin, const wchar_t* end){resize(end-begin+1);wcstombs(data(),begin,size());data()[--size()]='\0';}
    DynArray(const wchar_t* cwstr){if(!cwstr){clear();return;} size_t len=0;while(cwstr[len]!=L'\0')len++;resize(len+1);
		for(size_t i=0;i<len;++i)data()[i]=cwstr[i];data()[--size()]='\0';}
	DynArray(const char* cstr){if(!cstr){clear();return;} size_t len=0;while(cstr[len]!='\0')len++;resize(len+1);
		for(size_t i=0;i<len;++i)data()[i]=cstr[i];data()[--size()]='\0';}
	template<size_t N>DynArray(const char (&str)[N]){if(!str){clear();return;}resize(N+1);
		for(size_t i=0;i<N;++i)data()[i]=str[i];data()[--size()]='\0';}
	DynArray& operator+=(const char* cstr){if(!cstr)return *this;size_t len=0;while(cstr[len]!='\0')len++;size_t ll=size();resize(ll+len+1);
		for(size_t i=0;i<len;++i)data()[ll+i]=cstr[i];data()[--size()]='\0';return *this;}
	DynArray& operator+=(const DynArray<char> other){return (*this)+=other.data();}
};
template<>class DynArray<wchar_t>:public DynArrayBase<wchar_t>{public:using DynArrayBase<wchar_t>::DynArrayBase;
    DynArray(const char* begin, const char* end){resize(end-begin+1);mbstowcs(data(),begin,size());data()[--size()]=L'\0';}
    DynArray(const char* cstr){if(!cstr){clear();return;} size_t len=0;while(cstr[len]!='\0')len++;resize(len+1);
		for(size_t i=0;i<len;++i)data()[i]=cstr[i];data()[--size()]=L'\0';}
	DynArray(const wchar_t* cwstr){if(!cwstr){clear();return;} size_t len=0;while(cwstr[len]!=L'\0')len++;resize(len+1);
		for(size_t i=0;i<len;++i)data()[i]=cwstr[i];data()[--size()]=L'\0';}
	template<size_t N>DynArray(const wchar_t (&wstr)[N]){if(!wstr){clear();return;}resize(N+1);
		for(size_t i=0;i<N;++i)data()[i]=wstr[i];data()[--size()]=L'\0';}
	DynArray& operator+=(const wchar_t* cwstr){if(!cwstr)return *this;size_t len=0;while(cwstr[len]!=L'\0')len++;size_t ll=size();resize(ll+len+1);
		for(size_t i=0;i<len;++i)data()[ll+i]=cwstr[i];data()[--size()]=L'\0';return *this;}
	DynArray& operator+=(const DynArray<wchar_t> other){return (*this)+=other.data();}
};
template<typename... Args>struct ICallable{virtual ~ICallable(){}virtual void operator()(Args... args)=0;virtual ICallable* clonar()const=0;};
template<>struct ICallable<>{virtual ~ICallable(){}virtual void operator()()=0;virtual ICallable* clonar()const=0;};
template<typename T, typename... Args>struct CallableImpl:ICallable<Args...>{
    T(*fn)(Args...);
    CallableImpl(T(*f)(Args...)):fn(f){}
    void operator()(Args... args)override{if(fn)fn(args...);}
	ICallable<Args...>* clonar()const override{return new CallableImpl<T,Args...>(fn);}
};
template<typename T>struct CallableImpl<T> : ICallable<> {
    T(*fn)();
    CallableImpl(T(*f)()):fn(f){}
    void operator()() override{if(fn)fn();}
    ICallable<>* clonar()const override{return new CallableImpl<T>(fn);}
};
template<typename... Args>class Callback{ICallable<Args...>* ptr=nullptr;mini_traits::Tuple<Args...> args;public:
	Callback()=default;
	template<typename F>Callback(F(*fn)(Args...)){ptr = new CallableImpl<F, Args...>(fn);}
	Callback(const Callback& other){if(other.ptr)ptr=other.ptr->clonar();args=other.args;}
	Callback(Callback&& other)noexcept:ptr(other.ptr),args(other.args){other.ptr=nullptr;other.args={};}
	Callback& operator=(const Callback& other){if(this==&other)return *this;delete ptr;ptr=other.ptr?other.ptr->clonar():nullptr;
	    args=other.args;return *this;}Callback& operator=(Callback&& other)noexcept{if(this==&other)return *this;delete ptr;
		ptr=other.ptr;args=other.args;other.ptr=nullptr;other.args={};return *this;}~Callback(){delete ptr;}void operator()(Args... new_args)
	{if(ptr)((*ptr)(new_args...));}bool valido()const{return ptr!=nullptr;}void limpiar(){delete ptr;ptr=nullptr;}
	template<typename... OthArgs>ICallable<Args...>* set(const Callback<OthArgs...>& other){*this=other;return ptr;}
	ICallable<Args...>* set(decltype(nullptr)){limpiar();return ptr;}ICallable<Args...>* get()const{return ptr;}
};
template<>class Callback<>{ICallable<>* ptr=nullptr;mini_traits::Tuple<> args;public:
	Callback()=default;
	template<typename F>Callback(F(*fn)()){ptr = new CallableImpl<F>(fn);}
	Callback(const Callback& other){if(other.ptr)ptr=other.ptr->clonar();args=other.args;}
	Callback(Callback&& other)noexcept:ptr(other.ptr),args(other.args)
	{other.ptr=nullptr;other.args={};}Callback& operator=(const Callback& other){if(this==&other)return *this;delete ptr;
		ptr=other.ptr?other.ptr->clonar():nullptr;args=other.args;return *this;}Callback& operator=(Callback&& other)noexcept
		{if(this==&other)return *this;delete ptr;ptr=other.ptr;args=other.args;other.ptr=nullptr;other.args={};return *this;}
	~Callback(){delete ptr;}void operator()(){if(ptr)((*ptr)());}bool valido()const{return ptr!=nullptr;}
	void limpiar(){delete ptr;ptr=nullptr;}ICallable<>* set(const Callback<>& other)
		{*this=other;return ptr;}ICallable<>* set(decltype(nullptr)){limpiar();return ptr;}ICallable<>* get()const{return ptr;}
};

#ifndef RGBA
inline uint32_t RGBA(uint32_t r, uint32_t g, uint32_t b, uint32_t a){return((a&0xFF)<<24)|((r&0xFF)<<16)|((g&0xFF)<<8)|(b&0xFF);}
inline uint32_t GetRed(uint32_t _c){return(_c>>16)&0xFF;}inline uint32_t GetGreen(uint32_t _c){return(_c>>8)&0xFF;}
inline uint32_t GetBlue(uint32_t _c){return _c&0xFF;}inline uint32_t GetAlpha(uint32_t _c){return(_c>>24)&0xFF;}
#define rgba(r,g,b,a) RGBA(r,g,b,a)
#define rgb(r,g,b) RGBA(r,g,b,255)
#endif
#ifndef RGB
#define RGB(r,g,b) RGBA(r,g,b,255)
#endif
#ifndef mezclar_colores
inline uint32_t mezclar_colores(uint32_t A, uint32_t B){
	uint32_t aA=GetAlpha(A),aB=GetAlpha(B),invAlphaB=255-aB,r=(GetRed(B)*aB+GetRed(A)*invAlphaB+127)/255,
	g=(GetGreen(B)*aB+GetGreen(A)*invAlphaB+127)/255,b=(GetBlue(B)*aB+GetBlue(A)*invAlphaB+127)/255,a=(aB+(aA*invAlphaB+127)/255);
    return((aB==0)?A:(((aB==255)||(aA==0))?B:RGBA(r,g,b,a)));
}
#endif
#ifndef multiplicar_colores
inline uint32_t multiplicar_colores(uint32_t A, uint32_t B){
	return(A==0xFFFFFFFF)?B:((B==0xFFFFFFFF)?A:((A==0||B==0)?0:RGBA(((GetRed(A)*GetRed(B)+128)+((GetRed(A)*GetRed(B)+128)>>8))>>8,
	((GetGreen(A)*GetGreen(B)+128)+((GetGreen(A)*GetGreen(B)+128)>>8))>>8,
	((GetBlue(A)*GetBlue(B)+128)+((GetBlue(A)*GetBlue(B)+128)>>8))>>8,((GetAlpha(A)*GetAlpha(B)+128)+((GetAlpha(A)*GetAlpha(B)+128)>>8))>>8)
	));}
#endif

namespace maxiwin{ // Funciones del MOTOR
	class  img;
	class anim;
	struct fnt;
	struct pnt;
	class  mdl;
	struct cam;
	
	void mensaje(const DynArray<char> msj, const DynArray<char> title = "Mensaje...", int method = 0);
	bool pregunta(const DynArray<char> msj, const DynArray<char> title = "Pregunta...", int method = 0, int result = 0);
	int consulta(const DynArray<char> msj, const DynArray<char> title = "Consulta...", int method = 0);
	
	void espera(int miliseg);
	
	bool tecla(int key);
	bool tecla_presionada(int key);
	enum teclas{
		NINGUNA = 0,
		ESCAPE,
		IZQUIERDA, DERECHA, ARRIBA, ABAJO,
		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
		ESPACIO,
		RETURN,
		BACKSPACE,
		SHIFT,
		CTRL,
		ALT,
		TAB,
		MAYUSCULAS,
		SUPR,
		LWIN, RWIN,
		APP
	};
	
	bool  raton(float& x, float& y);
	bool  raton_dentro();
	float raton_x();
	float raton_y();
	void  raton_botones(bool& izq, bool& der);
	bool  raton_boton_izq();
	bool  raton_boton_der();
	bool  raton_boton_izq_justo();
	bool  raton_boton_der_justo();
	void  raton_capturar();
	void  raton_liberar();
	
	int  pancho();
	int  palto();
	
	DynArray<char> vtitulo();
	void v_fijar_titulo(const DynArray<char> title);
	int  vposicion_x();
	int  vposicion_y();
	int  vancho();
	int  valto();
	void vredimensiona(int ample, int alt);
	void vmover(int x, int y);
	void vcentrar();
	void vminimiza();
	void vmaximiza();
	void vcompleta();
	img vicono();
	void v_fijar_icono(const img& hImage);
	void voculta();
	void vmuestra();
	void vcierra();
	
	uint32_t color();
	void fijar_color(const uint32_t& _c);
	float lapiz();
	void fijar_lapiz(float size = 1.0f);
	cam camara();
	void fijar_camara(const cam& _c);
	
	void borra();
	void refresca();
	void balde(float x, float y);
	void punto(float x, float y);
	void linea(float x_ini, float y_ini, float x_fin, float y_fin);
	void rectangulo(float izq, float arr, float der, float aba);
	void rectangulo_lleno(float izq, float arr, float der, float aba);
	void circulo(float x_cen, float y_cen, float radio);
	void circulo_lleno(float x_cen, float y_cen, float radio);
	void elipse(float x_cen, float y_cen, float radio, float anguloInicio, float anguloFinal);
	void elipse_lleno(float x_cen, float y_cen, float radio, float anguloInicio, float anguloFinal);
	void arco(float x1, float y1, float x2, float y2, float x3, float y3);
	void arco_lleno(float x1, float y1, float x2, float y2, float x3, float y3);
	void triangulo(float x1, float y1, float x2, float y2, float x3, float y3);
	void triangulo_lleno(float x1, float y1, float x2, float y2, float x3, float y3);
	void poligono(DynArray<float> _x, DynArray<float> _y);
	template<size_t N>void poligono(const float(&_x)[N],const float(&_y)[N]){return poligono(DynArray<float>(_x),DynArray<float>(_y));}
	void poligono_lleno(DynArray<float> _x, DynArray<float> _y);
	template<size_t N>void poligono_lleno(const float(&_x)[N],const float(&_y)[N]){return poligono_lleno(DynArray<float>(_x),DynArray<float>(_y));}
	void texto(float x, float y, const DynArray<char>& txt);
	void imagen(float x, float y, const DynArray<char>& filePath);
	void imagen(float x, float y, const img& hImage);
	void imagen(float x, float y, anim& hAnimation);
	void imagen(float x, float y, const anim& hAnimation){ return imagen(x, y, (anim&)hAnimation); }
	void imagen(const pnt& hPoint);
	void imagen(float x, float y, const mdl& hModel);
	void imagen(const mdl& hModel);
	
	fnt fuente();
	void fijar_fuente(const fnt& fuente);
	enum class fuente_estilos : unsigned int{
		NEGRITA = 1,
		SUBRAYADA = 2,
		ITALICA = 4
	};
	void fijar_fuente(const DynArray<char>& fuente, unsigned int dimension = 12, unsigned int _styles = 0);
	DynArray<DynArray<char>> obtener_fuentes();
	
	img cargar_imagen(const DynArray<char>& filePath);
	anim cargar_animacion(const DynArray<char>& gifPath);
	int ancho_imagen(const DynArray<char>& filePath);
	int alto_imagen(const DynArray<char>& filePath);
	fnt cargar_fuente(const DynArray<char>& filePath);
	mdl cargar_modelo(const DynArray<char>& filePath, double scale = 1.0, bool load_textures = true);
	
	template<typename... Args> void hacer_al_salir(const Callback<Args...> doing);
	template<typename F,typename... Args>
	void hacer_al_salir(F(&doing)(Args...)){return hacer_al_salir(Callback<Args...>(&doing));}
	void cancelar_cierre();
	void aceptar_cierre();
	
	img captura_de_ventana();
	template<typename Func, typename... Args> img& lienzo(img& hImage, Func fn, Args&&... args);
	#define _MAXIWIN_H__imagen_ decltype(static_cast<void(*)(float,float,const img&)>(maxiwin::imagen))
	template<typename... Args> img& lienzo(img& hImage, _MAXIWIN_H__imagen_, Args&&... args);
	
	namespace colores{ // Heladeria Maxiwin... Ã‚Â¿De que sabor desea ordenar su helado?
		const uint32_t MascaraAlpha             = 0xFF000000;
		const uint32_t AzulAlicia               = 0xFFF0F8FF;
		const uint32_t BlancoAntiguo            = 0xFFFAEBD7;
		const uint32_t Agua                     = 0xFF00FFFF;
		const uint32_t Aguamarina               = 0xFF7FFFD4;
		const uint32_t AzulCeleste              = 0xFFF0FFFF;
		const uint32_t Beige                    = 0xFFF5F5DC;
		const uint32_t Bizcocho                 = 0xFFFFE4C4;
		const uint32_t Negro                    = 0xFF000000;
		const uint32_t AlmendraBlanqueada       = 0xFFFFEBCD;
		const uint32_t Azul                     = 0xFF0000FF;
		const uint32_t AzulVioleta              = 0xFF8A2BE2;
		const uint32_t Marron                   = 0xFFA52A2A;
		const uint32_t MaderaRustica            = 0xFFDEB887;
		const uint32_t AzulCadete               = 0xFF5F9EA0;
		const uint32_t Chartreuse               = 0xFF7FFF00;
		const uint32_t Chocolate                = 0xFFD2691E;
		const uint32_t Coral                    = 0xFFFF7F50;
		const uint32_t AzulAciano               = 0xFF6495ED;
		const uint32_t SedaDeMaIz               = 0xFFFFF8DC;
		const uint32_t CarmesI                  = 0xFFDC143C;
		const uint32_t Cian                     = 0xFF00FFFF;
		const uint32_t AzulOscuro               = 0xFF00008B;
		const uint32_t CianOscuro               = 0xFF008B8B;
		const uint32_t OroViejoOscuro           = 0xFFB8860B;
		const uint32_t GrisOscuro               = 0xFFA9A9A9;
		const uint32_t VerdeOscuro              = 0xFF006400;
		const uint32_t CaquiOscuro              = 0xFFBDB76B;
		const uint32_t MagentaOscuro            = 0xFF8B008B;
		const uint32_t VerdeOlivaOscuro         = 0xFF556B2F;
		const uint32_t NaranjaOscuro            = 0xFFFF8C00;
		const uint32_t OrquIdeaOscura           = 0xFF9932CC;
		const uint32_t RojoOscuro               = 0xFF8B0000;
		const uint32_t SalmonOscuro             = 0xFFE9967A;
		const uint32_t VerdeMarOscuro           = 0xFF8FBC8F;
		const uint32_t AzulPizarraOscuro        = 0xFF483D8B;
		const uint32_t GrisPizarraOscuro        = 0xFF2F4F4F;
		const uint32_t TurquesaOscuro           = 0xFF00CED1;
		const uint32_t VioletaOscuro            = 0xFF9400D3;
		const uint32_t RosaIntenso              = 0xFFFF1493;
		const uint32_t AzulCieloIntenso         = 0xFF00BFFF;
		const uint32_t GrisTenue                = 0xFF696969;
		const uint32_t AzulDodger               = 0xFF1E90FF;
		const uint32_t LadrilloRefractario      = 0xFFB22222;
		const uint32_t BlancoFloral             = 0xFFFFFAF0;
		const uint32_t VerdeBosque              = 0xFF228B22;
		const uint32_t Fucsia                   = 0xFFFF00FF;
		const uint32_t GrisGainsboro            = 0xFFDCDCDC;
		const uint32_t BlancoFantasma           = 0xFFF8F8FF;
		const uint32_t Oro                      = 0xFFFFD700;
		const uint32_t VaraDeOro                = 0xFFDAA520;
		const uint32_t Gris                     = 0xFF808080;
		const uint32_t Verde                    = 0xFF008000;
		const uint32_t VerdeAmarillo            = 0xFFADFF2F;
		const uint32_t RocIoDeMiel              = 0xFFF0FFF0;
		const uint32_t RosaCaliente             = 0xFFFF69B4;
		const uint32_t RojoIndio                = 0xFFCD5C5C;
		const uint32_t Indigo                   = 0xFF4B0082;
		const uint32_t Marfil                   = 0xFFFFFFF0;
		const uint32_t Caqui                    = 0xFFF0E68C;
		const uint32_t Lavanda                  = 0xFFE6E6FA;
		const uint32_t RuborLavanda             = 0xFFFFF0F5;
		const uint32_t VerdeCesped              = 0xFF7CFC00;
		const uint32_t ChiffonDeLimon           = 0xFFFFFACD;
		const uint32_t AzulClaro                = 0xFFADD8E6;
		const uint32_t CoralClaro               = 0xFFF08080;
		const uint32_t CianClaro                = 0xFFE0FFFF;
		const uint32_t AmarilloOroClaro         = 0xFFFAFAD2;
		const uint32_t GrisClaro                = 0xFFD3D3D3;
		const uint32_t VerdeClaro               = 0xFF90EE90;
		const uint32_t RosaClaro                = 0xFFFFB6C1;
		const uint32_t SalmonClaro              = 0xFFFFA07A;
		const uint32_t VerdeMarClaro            = 0xFF20B2AA;
		const uint32_t AzulCieloClaro           = 0xFF87CEFA;
		const uint32_t GrisPizarraClaro         = 0xFF778899;
		const uint32_t AzulAceroClaro           = 0xFFB0C4DE;
		const uint32_t AmarilloClaro            = 0xFFFFFFE0;
		const uint32_t Lima                     = 0xFF00FF00;
		const uint32_t VerdeLima                = 0xFF32CD32;
		const uint32_t Lino                     = 0xFFFAF0E6;
		const uint32_t Magenta                  = 0xFFFF00FF;
		const uint32_t Granate                  = 0xFF800000;
		const uint32_t AguamarinaMedia          = 0xFF66CDAA;
		const uint32_t AzulMedio                = 0xFF0000CD;
		const uint32_t OrquIdeaMedia            = 0xFFBA55D3;
		const uint32_t PurpuraMedia             = 0xFF9370DB;
		const uint32_t VerdeMarMedio            = 0xFF3CB371;
		const uint32_t AzulPizarraMedio         = 0xFF7B68EE;
		const uint32_t VerdePrimaveraMedio      = 0xFF00FA9A;
		const uint32_t TurquesaMedia            = 0xFF48D1CC;
		const uint32_t RojoVioletaMedio         = 0xFFC71585;
		const uint32_t AzulMedianoche           = 0xFF191970;
		const uint32_t CremaDeMenta             = 0xFFF5FFFA;
		const uint32_t RosaBrumoso              = 0xFFFFE4E1;
		const uint32_t MocasIn                  = 0xFFFFE4B5;
		const uint32_t BlancoNavajo             = 0xFFFFDEAD;
		const uint32_t AzulMarino               = 0xFF000080;
		const uint32_t EncajeAntiguo            = 0xFFFDF5E6;
		const uint32_t Oliva                    = 0xFF808000;
		const uint32_t VerdeOliva               = 0xFF6B8E23;
		const uint32_t Naranja                  = 0xFFFFA500;
		const uint32_t RojoNaranja              = 0xFFFF4500;
		const uint32_t OrquIdea                 = 0xFFDA70D6;
		const uint32_t OroPalido                = 0xFFEEE8AA;
		const uint32_t VerdePalido              = 0xFF98FB98;
		const uint32_t TurquesaPalido           = 0xFFAFEEEE;
		const uint32_t RojoVioletaPalido        = 0xFFDB7093;
		const uint32_t PapayaSuave              = 0xFFFFEFD5;
		const uint32_t MelocotonSuave           = 0xFFFFDAB9;
		const uint32_t Peru                     = 0xFFCD853F;
		const uint32_t Rosa                     = 0xFFFFC0CB;
		const uint32_t Ciruela                  = 0xFFDDA0DD;
		const uint32_t AzulTalco                = 0xFFB0E0E6;
		const uint32_t Purpura                  = 0xFF800080;
		const uint32_t Rojo                     = 0xFFFF0000;
		const uint32_t MarronRosado             = 0xFFBC8F8F;
		const uint32_t AzulReal                 = 0xFF4169E1;
		const uint32_t MarronMontura            = 0xFF8B4513;
		const uint32_t Salmon                   = 0xFFFA8072;
		const uint32_t MarronArenoso            = 0xFFF4A460;
		const uint32_t VerdeMar                 = 0xFF2E8B57;
		const uint32_t ConchaDeMar              = 0xFFFFF5EE;
		const uint32_t Siena                    = 0xFFA0522D;
		const uint32_t Plata                    = 0xFFC0C0C0;
		const uint32_t AzulCielo                = 0xFF87CEEB;
		const uint32_t AzulPizarra              = 0xFF6A5ACD;
		const uint32_t GrisPizarra              = 0xFF708090;
		const uint32_t Nieve                    = 0xFFFFFAFA;
		const uint32_t VerdePrimavera           = 0xFF00FF7F;
		const uint32_t AzulAcero                = 0xFF4682B4;
		const uint32_t Bronceado                = 0xFFD2B48C;
		const uint32_t VerdeAzulado             = 0xFF008080;
		const uint32_t Cardo                    = 0xFFD8BFD8;
		const uint32_t Tomate                   = 0xFFFF6347;
		const uint32_t Transparente             = 0x00FFFFFF;
		const uint32_t Turquesa                 = 0xFF40E0D0;
		const uint32_t Violeta                  = 0xFFEE82EE;
		const uint32_t Trigo                    = 0xFFF5DEB3;
		const uint32_t Blanco                   = 0xFFFFFFFF;
		const uint32_t HumoBlanco               = 0xFFF5F5F5;
		const uint32_t Amarillo                 = 0xFFFFFF00;
		const uint32_t AmarilloVerde            = 0xFF9ACD32;
	}
	namespace mensajeria{ // Mensajeria Maxiwin... Ã‚Â¿Como desea escribir su carta a 'user'?
		namespace botoneria{ // Botoneria Maxiwin... Ã‚Â¿Necesita traducir lo que 'user' le dijo?
			const uint32_t CRUZ       = 0x0;
			const uint32_t OK         = 0x1;
			const uint32_t CANCELAR   = 0x2;
			const uint32_t ABORTAR    = 0x3;
			const uint32_t REINTENTAR = 0x4;
			const uint32_t IGNORAR    = 0x5;
			const uint32_t SI         = 0x6;
			const uint32_t NO         = 0x7;
			const uint32_t INTENTAR   = 0xA;
			const uint32_t CONTINUAR  = 0xB;
		}
		const uint32_t BOTN_OK                          = 0x00000000;
		const uint32_t BOTN_OK_CANCELAR                 = 0x00000001;
		const uint32_t BOTN_ABORTAR_REINTENTAR_IGNORAR  = 0x00000002;
		const uint32_t BOTN_SI_NO_CANCELAR              = 0x00000003;
		const uint32_t BOTN_SI_NO                       = 0x00000004;
		const uint32_t BOTN_REINTENTAR_CANCELAR         = 0x00000005;
		const uint32_t BOTN_CANCELAR_INTENTAR_CONTINUAR = 0x00000006;
		const uint32_t ICON_CRUZ                        = 0x00000010;
		const uint32_t ICON_PREGUNTA                    = 0x00000020;
		const uint32_t ICON_EXCLAMACION                 = 0x00000030;
		const uint32_t ICON_ASTERISCO                   = 0x00000040;
		const uint32_t ICON_USUARIO                     = 0x00000080;
		const uint32_t ICON_ADVERTENCIA                 = ICON_EXCLAMACION;
		const uint32_t ICON_ERROR                       = ICON_CRUZ;
		const uint32_t ICON_INFORMACION                 = ICON_ASTERISCO;
		const uint32_t ICON_PARAR                       = ICON_CRUZ;
		const uint32_t DEFT_BOTON_1                     = 0x00000000;
		const uint32_t DEFT_BOTON_2                     = 0x00000100;
		const uint32_t DEFT_BOTON_3                     = 0x00000200;
		const uint32_t DEFT_BOTON_4                     = 0x00000300;
		const uint32_t MODL_APP                         = 0x00000000;
		const uint32_t MODL_SISTEMA                     = 0x00001000;
		const uint32_t MODL_TAREA                       = 0x00002000;
		const uint32_t MODL_AYUDA                       = 0x00004000;
		const uint32_t MODL_NOFOCUS                     = 0x00008000;
		const uint32_t CTRL_FOREGROUND                  = 0x00010000;
		const uint32_t CTRL_DEFAULT_ESCRITORIO          = 0x00020000;
		const uint32_t CTRL_MASALTO                     = 0x00040000;
		const uint32_t CTRL_DERECHA                     = 0x00080000;
		const uint32_t CTRL_LECTURA_DERECHA             = 0x00100000;
		const uint32_t CTRL_NOTIFICACION                = 0x00200000;
		const uint32_t CTRL_NOTIFICACION_NT3X           = 0x00040000;
		const uint32_t MASK_BOTN                        = 0x0000000F;
		const uint32_t MASK_ICON                        = 0x000000F0;
		const uint32_t MASK_DEFT                        = 0x00000F00;
		const uint32_t MASK_MODL                        = 0x00003000;
		const uint32_t MASK_CTRL                        = 0x0000C000;
	}
}
#ifndef _MAXIWIN_H_TO_CPP_
#include "maxiwin.cpp"
#endif
#endif