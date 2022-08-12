/* -*- mode: c++; c-basic-offset: 2 -*-  */
#ifndef _CPP_14_H_
#define _CPP_14_H_


// TODO: to be removed with shared ptr

#if __cplusplus < 201402L
#include <memory>

namespace std
{
    // std::make_unique only available since C++14
    template<class T> struct _Unique_if {
        typedef unique_ptr<T> _Single_object;
    };
    template<class T> struct _Unique_if<T[]> {
        typedef unique_ptr<T[]> _Unknown_bound;
    };
    template<class T, size_t N> struct _Unique_if<T[N]> {
        typedef void _Known_bound;
    };
#ifndef _MSC_VER
    template<class T, class... Args>
        typename _Unique_if<T>::_Single_object
        make_unique(Args&&... args) {
            return unique_ptr<T>(new T(std::forward<Args>(args)...));
        }
#endif
    template<class T>
        typename _Unique_if<T>::_Unknown_bound
        make_unique(size_t n) {
            typedef typename remove_extent<T>::type U;
            return unique_ptr<T>(new U[n]());
        }
    template<class T, class... Args>
        typename _Unique_if<T>::_Known_bound
        make_unique(Args&&...) = delete;
}

#endif

#endif
