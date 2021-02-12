/* ------------------------------------------------------------------ */
/* decConfig.h header                                                 */
/* ------------------------------------------------------------------ */
/* Copyright (c) IBM Corporation, 2006.  All rights reserved.         */
/*                                                                    */
/* This software is made available under the terms of the IBM         */
/* alphaWorks License Agreement (distributed with this software as    */
/* alphaWorks-License.txt).  Your use of this software indicates      */
/* your acceptance of the terms and conditions of that Agreement.     */
/*                                                                    */
/* Please send comments, suggestions, and corrections to the author:  */
/*   klarer@ca.ibm.com                                                */
/*   Robert Klarer                                                    */
/* ------------------------------------------------------------------ */

#ifndef _DECCONFIG_H_
#define _DECCONFIG_H_

/*
   The decNumber++ library recognizes a number of configuration macros.
   Set these as required for your compiler and architecture.

   Byte order settings (#define exactly one of these):
   ---------------------------------------------------
   _DEC_BIG_ENDIAN        -- #define this macro if you are targeting a
                             big endian architecture.
   _DEC_LITTLE_ENDIAN     -- #define this macro if you are targeting a
                             little endian architecture.

   Long double format settings (#define exactly one of these):
   -----------------------------------------------------------
   _DEC_LONGDOUBLE64      -- #define this macro if your compiler uses the
                             64-bit IEEE binary floating-point format to
                             represent a long double.
   _DEC_LONGDOUBLE80      -- #define this macro if your compiler uses the
                             Intel 80-bit extended-precision floating-point
                             format to represent a long double
                             (valid only when _DEC_LITTLE_ENDIAN is #defined).
   _DEC_LONGDOUBLE128     -- #define this macro if your compiler uses the
                             128-bit IEEE binary floating-point format to
                             represent a long double.

   C++ compiler language support settings:
   ---------------------------------------
   _DEC_HAS_C99_MATH      -- #define this macro if your environment supports
                             the C99 math library.
   _DEC_HAS_TR1_MATH      -- #define this macro if your C++ environment
                             supports the C++ TR1 math library facilities.
                             In particular, if the header file <cmath>
                             contains the template std::tr1::fpclassify<>,
                             you can safely define this macro. 
   _DEC_HAS_TYPE_TRAITS   -- This library uses some C++ TR1 type_traits
                             internally.  You should #define this macro if
			     your C++ environment includes support for the
			     C++ TR1 header <type_traits>.
                             If this macro is not #defined, decNumber++ will
			     use the alternative type_traits provided in this
                             file.
   _DEC_STATIC_ASSERT     -- #define this macro to expand to the C++ keyword
                             "static_assert" if your C++ compiler supports
                             C++0x static_assert declarations.
                             Some compilers may support static_assert
                             functionality under an alternative
                             (non-standard) keyword, such as __static_assert.
                             To use an alternative form, #define this macro
                             to expand to the supported keyword
                             (eg. #define _DEC_STATIC_ASSERT __static_assert).
                             If your compiler supports neither form, leave
                             this macro undefined.
*/

/*************************************************************************
  Configuration macros:
  You may wish to add configuration #definitions here for your C++ compiler
  and environment.
*************************************************************************/

// Configuration for IBM xlC++ on AIX:

#if defined(__IBMCPP__)

#  if defined(_AIX)
#    define _DEC_HAS_C99_MATH 1
#  endif /* defined(_AIX) */

#  if (__IBMCPP__ >= 800)
#    define _DEC_HAS_TYPE_TRAITS 1
#  endif /* __IBMCPP__ >= 800 */

#  if (__IBMCPP__ >= 700)
#    define _DEC_STATIC_ASSERT __static_assert
#  endif

#  define _DEC_BIG_ENDIAN 1

#  if defined(__LONGDOUBLE128)
#    define _DEC_LONGDOUBLE128 1
#  else
#    define _DEC_LONGDOUBLE64 1
#  endif /* defined(__LONGDOUBLE128) */

#  if !defined(_LONG_LONG)
#    error "Please enable support for the long long int data type."
#  endif /* defined(_LONG_LONG) */

#endif /* defined(__IBMCPP__) */

/* mauro: define things according to our needs here.

   TODO: it would be nice to have that adapt to the machine we are running
         on automagically using something like 

            if defined(__CELL)
                ...
            endif

*/
#define _DEC_LITTLE_ENDIAN 1
#define _DEC_LONGDOUBLE80 1

// Configuration for MinGW:

#if defined(__MINGW32__)
#  define _DEC_LITTLE_ENDIAN 1
#  define _DEC_LONGDOUBLE80 1
#endif /* defined(__MINGW__) */

/*************************************************************************
  Configuration code:
  Do not modify the code below this line.
*************************************************************************/

// Handle _DEC_STATIC_ASSERT
#if defined(_DEC_STATIC_ASSERT)
#  define _DEC_NO_MIXED_RADIX \
        _DEC_STATIC_ASSERT(false, "Mixed-radix arithmetic is not permitted.");
#else
void _ERROR_DUE_TO_MIXED_RADIX_ARITHMETIC();
#  define _DEC_NO_MIXED_RADIX \
        _ERROR_DUE_TO_MIXED_RADIX_ARITHMETIC();
#endif /* defined(_DEC_STATIC_ASSERT) */

// Handle _DEC_HAS_TYPE_TRAITS:

#ifdef _DEC_HAS_TYPE_TRAITS
#  include <type_traits>
#else

namespace std {
namespace tr1 {

template <class _T>
struct _Is_floating_point_aux {
        static const bool value = false;
};

template <>
struct _Is_floating_point_aux<float> {
        static const bool value = true;
};

template <>
struct _Is_floating_point_aux<double> {
        static const bool value = true;
};

template <>
struct _Is_floating_point_aux<long double> {
        static const bool value = true;
};

template <class _T>
struct _Is_signed_aux {
        static const bool value = false;
};

template <>
struct _Is_signed_aux<signed char> {
        static const bool value = true;
};

template <>
struct _Is_signed_aux<short> {
        static const bool value = true;
};

template <>
struct _Is_signed_aux<int> {
        static const bool value = true;
};

template <>
struct _Is_signed_aux<long> {
        static const bool value = true;
};

template <>
struct _Is_signed_aux<long long> {
        static const bool value = true;
};

template <class _T>
struct _Is_unsigned_aux {
        static const bool value = false;
};

template <>
struct _Is_unsigned_aux<unsigned char> {
        static const bool value = true;
};

template <>
struct _Is_unsigned_aux<unsigned short> {
        static const bool value = true;
};

template <>
struct _Is_unsigned_aux<unsigned int> {
        static const bool value = true;
};

template <>
struct _Is_unsigned_aux<unsigned long> {
        static const bool value = true;
};

template <>
struct _Is_unsigned_aux<unsigned long long> {
        static const bool value = true;
};

template <class _T, class _U>
struct _Is_same_aux {
        static const bool value = false;
};

template <class _T>
struct _Is_same_aux<_T, _T> {
        static const bool value = true;
};

template <class _T>
struct _Is_integral_aux {
        static const bool value =
                _Is_signed_aux<_T>::value ||
                _Is_unsigned_aux<_T>::value ||
                _Is_same_aux<_T, bool>::value ||
                _Is_same_aux<_T, char>::value ||
                _Is_same_aux<_T, wchar_t>::value;
};

template <class _T,
          _T _V>
struct integral_constant {
        static const _T value = _V;
        typedef _T value_type;
        typedef integral_constant<_T, _V> type;
};

typedef integral_constant<bool, true> true_type;
typedef integral_constant<bool, false> false_type;

// transformation trait remove_const
//
template <class _T>
struct remove_const {
        typedef _T type;
};

template <class _T>
struct remove_const<_T const> {
        typedef _T type;
};

// transformation trait remove_volatile
//
template <class _T>
struct remove_volatile {
        typedef _T type;
};

template <class _T>
struct remove_volatile<_T volatile> {
        typedef _T type;
};

// transformation trait remove_cv
//
template <class _T>
struct remove_cv {
        typedef typename
                remove_const<typename remove_volatile<_T>::type>::type
                type;
};

// type trait is_integral 
//
template <class _T>
struct is_integral
        : integral_constant<bool,
          _Is_integral_aux<typename remove_cv<_T>::type>::value > {
        typedef bool value_type;
        typedef integral_constant<value_type, 
                _Is_integral_aux<typename remove_cv<_T>::type>::value> type;
        operator type() const { return type(); }
};

// type trait is_floating_point
//
template <class _T>
struct is_floating_point
        : integral_constant<bool,
          _Is_floating_point_aux<typename remove_cv<_T>::type>::value >{
        typedef bool value_type;
        typedef integral_constant<value_type,
                _Is_floating_point_aux<typename remove_cv<_T>::type>::value>
                type;
        operator type() const { return type(); }
};
#endif /* ndef _DEC_HAS_TYPE_TRAITS */

} // namespace tr1
} // namespace std

#endif /* ndef _DECCONFIG_H_ */
