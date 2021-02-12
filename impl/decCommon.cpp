/* ------------------------------------------------------------------ */
/* decCommon.cpp source file                                          */
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

#include <climits>
#include <cmath>
#include <limits>

#define DECNUMDIGITS 45 // IMPORTANT: must appear first!
extern "C" {
#include "decNumber.h"
#include "decNumberLocal.h"
#include "decimal128.h"
#include "decimal64.h"
#include "decimal32.h"
}
#include "decimal"

namespace std {
namespace decimal {

static const size_t lsu_extent = DECNUMUNITS;

rounding _ContextBase::_RoundingMode = DEC_ROUND_HALF_UP;

const char   _DecNumber::_InfStr[] = "infinity";
const size_t _DecNumber::_InfStrLen = sizeof(_InfStr);
const char   _DecNumber::_INFStr[] = "INFINITY";
const size_t _DecNumber::_INFStrLen = sizeof(_INFStr);
const char   _DecNumber::_NanStr[] = "nan";
const size_t _DecNumber::_NanStrLen = sizeof(_InfStr);
const char   _DecNumber::_NANStr[] = "NAN";
const size_t _DecNumber::_NANStrLen = sizeof(_INFStr);

// class to aid conversions involving objects of long double type
struct _LongDouble {

        _LongDouble()
                { as_ldbl = +0.0L; } 
        _LongDouble(long double ld)
                { as_ldbl = ld; }
        _LongDouble(const _DecNumber & dec)
                { decimalToBinary(dec); }

        operator long double() const
                { return as_ldbl; }
        operator _DecNumber() const
                { return binaryToDecimal(); }

        uint16_t _GetBiasedExponent() const;
        int16_t  _GetExponent() const;
        void     _SetExponent(int16_t);
        void     _SetSignFlag(bool b);

        bool _IsSubnormal() const
                { return ((_GetBiasedExponent() == 0) && (as_ldbl != 0)); }
        bool _IsNeg() const
                { return as_ldbl < 0; }
        bool _IsNAN() const
                { return isnan(as_ldbl); }
        bool _IsINF() const
                { return isinf(as_ldbl); }

        void decimalToBinary(const _DecNumber &);
        _DecNumber binaryToDecimal() const;
 
        static const std::size_t rep_extent =
                sizeof(long double)/sizeof(uint8_t);

#if defined(_DEC_LONGDOUBLE80)
        static const uint32_t exp_bias = 16383;
#elif defined(_DEC_LONGDOUBLE64) || defined(_DEC_LONGDOUBLE128)
        static const uint32_t exp_bias = 1023;
#else
#  error "Invalid long double format setting.  Please update decConfig.h."
#endif /* defined(_DEC_LONGDOUBLExx) */

        union {
                uint8_t bytes[rep_extent];
                long double as_ldbl; 
        };

        struct _RepIterBase {
                _RepIterBase();
                void _NextBit()
                        {
                        if (mask == 0x01)
                                {
                                mask = 0x80; 
                                _NextIndex();
                                }
                        else
                                { mask >>= 1; }
                        }
                void _PrevBit()
                        {
                        if (mask == 0x80)
                                {
                                mask = 0x01; 
                                _PrevIndex();
                                }
                        else
                                { mask <<= 1; }
                        }
                bool _IsDone() const;
        private:
                void _NextIndex();
                void _PrevIndex();
        protected:
                size_t    index;
                uint8_t   mask;
        };

        struct _RepIter : _RepIterBase {
                _RepIter(uint8_t * p) : rep(p) {}
                void _SetBit()
                        { rep[index] |= mask; }
        private:
                uint8_t * rep;
        };

        struct _ConstRepIter : _RepIterBase {
                _ConstRepIter(const uint8_t * p) : rep(p) {}
                bool _GetBit() const
                        { return rep[index] & mask; }
        private:
                uint8_t const * rep;
        };
};

// This macro indicates that the binary encoding of a long double
// represents the most significant bit of the significand implicitly,
// since it is always set.
// This macro will be #undefined below if the selected long double
// encoding is not packed.
//
#define _DEC_HAS_IMPLICIT_BIT 1

#if defined(_DEC_BIG_ENDIAN)

inline bool _LongDouble::_RepIterBase::_IsDone() const
        { return index == _LongDouble::rep_extent; }

inline void _LongDouble::_RepIterBase::_NextIndex()
        { ++index; }
inline void _LongDouble::_RepIterBase::_PrevIndex()
        { --index; }

#if defined(_DEC_LONGDOUBLE64)

inline _LongDouble::_RepIterBase::_RepIterBase() :
        index(1), mask(0x08) {}

inline uint16_t _LongDouble::_GetBiasedExponent() const
        { return ((bytes[0] & 0x7F) << 4) + ((bytes[1] & 0xF0) >> 4); }

inline void _LongDouble::_SetExponent(int16_t _Exp)
        {
        uint16_t exponent = _Exp + exp_bias;
        bytes[1] &= 0x0F;
        bytes[1] |= (exponent & 0x000F) << 4;
        bytes[0] &= 0x80;
        bytes[0] |= (exponent & 0x07FF) >> 4;
        }

inline void _LongDouble::_SetSignFlag(bool b)
        { b ? bytes[0] |= 0x80 : bytes[0] &= 0x7F; }

#elif defined(_DEC_LONGDOUBLE128)

inline _LongDouble::_RepIterBase::_RepIterBase() :
        index(2), mask(0x80) {}

inline uint16_t _LongDouble::_GetBiasedExponent() const
        { return ((bytes[0] & 0x7F) << 8) + (bytes[1] & 0xFF); }

inline void _LongDouble::_SetExponent(int16_t _Exp)
        {
        uint16_t exponent = _Exp + exp_bias;
        bytes[1] |= exponent & 0x00FF;
        bytes[0] &= 0x80;
        bytes[0] |= (exponent & 0x7F00) >> 8;
        }

inline void _LongDouble::_SetSignFlag(bool b)
        { b ? bytes[0] |= 0x80 : bytes[0] &= 0x7F; }

#else
#  error "Invalid long double format setting.  Please update decConfig.h."
#endif /* defined(_DEC_LONGDOUBLExx) */

#elif defined(_DEC_LITTLE_ENDIAN)

inline bool _LongDouble::_RepIterBase::_IsDone() const
        { return index > _LongDouble::rep_extent; }

inline void _LongDouble::_RepIterBase::_NextIndex()
        { --index; }
inline void _LongDouble::_RepIterBase::_PrevIndex()
        { ++index; }

#  if defined(_DEC_LONGDOUBLE64) || defined(_DEC_LONGDOUBLE128)

inline _LongDouble::_RepIterBase::_RepIterBase() :
        index(_LongDouble::rep_extent - 2), mask(0x08) {}

inline uint16_t _LongDouble::_GetBiasedExponent() const
        {
        return ((bytes[rep_extent - 1] & 0x7F) << 4) +
               ((bytes[rep_extent - 2] & 0xF0) >> 4);
        }

inline void _LongDouble::_SetExponent(int16_t _Exp)
        {
        uint16_t exponent = _Exp + exp_bias;
        bytes[rep_extent - 2] &= 0x0F;
        bytes[rep_extent - 2] |= (exponent & 0x000F) << 4;
        bytes[rep_extent - 1] &= 0x80;
        bytes[rep_extent - 1] = (exponent & 0x07FF) >> 4;
        }

inline void _LongDouble::_SetSignFlag(bool b)
        { b ? bytes[rep_extent - 1] |= 0x80 : bytes[rep_extent - 1] &= 0x7F; }

#  elif defined(_DEC_LONGDOUBLE80)

#  undef _DEC_HAS_IMPLICIT_BIT

inline _LongDouble::_RepIterBase::_RepIterBase() :
        index(_LongDouble::rep_extent - 5), mask(0x80) {}

inline uint16_t _LongDouble::_GetBiasedExponent() const
        {
        return ((bytes[rep_extent - 3] & 0x7F) << 8) + bytes[rep_extent - 4];
        }

inline void _LongDouble::_SetExponent(int16_t _Exp)
        {
        uint16_t exponent = _Exp + exp_bias;
        bytes[rep_extent - 4] = (exponent & 0x00FF);
        bytes[rep_extent - 3] &= 0x80;
        bytes[rep_extent - 3] |= (exponent & 0x7F00) >> 8;
        }

inline void _LongDouble::_SetSignFlag(bool b)
        { b ? bytes[rep_extent - 3] |= 0x80 : bytes[rep_extent - 3] &= 0x7F; }

#  else
#    error "Invalid long double format setting.  Please update decConfig.h."
#  endif /* defined(_DEC_LONGDOUBLExx) */

#else
#  error "Invalid byte order format setting.  Please update decConfig.h."
#endif /* defined(_DEC_xxx_ENDIAN) */

int16_t _LongDouble::_GetExponent() const
        { return _GetBiasedExponent() - exp_bias; }

void _LongDouble::decimalToBinary(const _DecNumber & num)
        {
        typedef _DecNumber::_Le _Le;

        // Handle special numbers.
        if (num._IsNAN())
                {
                as_ldbl = NAN;
                return;
                }
        if (num._IsINF())
                {
                as_ldbl = num._IsNeg() ? -INFINITY : INFINITY;
                return;
                }

        static const _DecNumber five(5);
        _DecNumber fracPartBase10 = num;
        _DecNumber powerOfHalf(1);

        int32_t fracPartExpBase2 = -1;
        bool    foundImplicitBit = false;
        as_ldbl = +0.0L;

        // Handle the integer part of the number.
        _ContextDefault ctx;
        ctx.digits = DECNUMDIGITS;
        ctx.round  = DEC_ROUND_DOWN;
        _DecNumber intPartBase10;
        decNumberToIntegralValue(&intPartBase10, &fracPartBase10, &ctx);

        long double intPartBase2 = +0.0L;
        if (!intPartBase10._IsZero())
                {
                for (size_t i = DECNUMUNITS; i != 0; --i)
                        {
                        intPartBase2 *= DECDPUNMAX + 1;
                        intPartBase2 += intPartBase10.lsu[i - 1];
                        }
                if (intPartBase10._IsNeg())
                         { intPartBase2 *= -1.0L; }
                ctx.round  = DEC_ROUND_HALF_UP;
                decNumberSubtract(&fracPartBase10, &fracPartBase10,
                                  &intPartBase10, &ctx);
                }
        else
                { ctx.round = DEC_ROUND_HALF_UP; }

        // Handle the sign.
        _SetSignFlag(fracPartBase10._IsNeg());
        fracPartBase10._ClearSign();

        // Handle the fractional part of the number.
        _RepIter iter(bytes);

        while (!iter._IsDone() && !fracPartBase10._IsZero())
                {
                // Divide powerOfHalf by 2.
                // We accomplish this by calculating (powerOfHalf * 5)/10.
                decNumberMultiply(&powerOfHalf, &powerOfHalf, &five, &ctx);
                --powerOfHalf.exponent; // divide by 10

                if (_DecNumber::_Compare<_Le>(powerOfHalf, fracPartBase10))
                        {
                        // This bit is one.
                        decNumberSubtract(&fracPartBase10, &fracPartBase10,
                                          &powerOfHalf, &ctx); 

                        if (!foundImplicitBit)
                                {
                                foundImplicitBit = true;
#if defined(_DEC_HAS_IMPLICIT_BIT)
                                iter._PrevBit();
#else
                                iter._SetBit();
#endif /* defined(_DEC_HAS_IMPLICIT_BIT) */
                                }
                        else
                                { iter._SetBit(); }
                        }

                if (foundImplicitBit)
                        { iter._NextBit(); }
                else
                        { --fracPartExpBase2; }
                }

        if (!fracPartBase10._IsZero())
                { decContextSetStatus(&ctx, DEC_Inexact); }

        if (as_ldbl != 0.0L || foundImplicitBit)
                { _SetExponent(fracPartExpBase2); }

        // Add the integer part of the number to the fractional part.
        as_ldbl += intPartBase2 * pow(10.0L, intPartBase10.exponent);
        }

_DecNumber _LongDouble::binaryToDecimal() const
        {
        static const _DecNumber five(5);
        _DecNumber acc(0);
        _DecNumber powerOfHalf(1);
        bool is_exact = false;
        int32_t exp = -1;

        _ContextDefault ctx;
        ctx.digits = DECNUMDIGITS;

        // Handle special numbers.
        if (_IsNAN())
                {
                acc.bits = DECNAN;
                return acc;
                }
        if (_IsINF())
                {
                acc.bits = _IsNeg() ? -DECINF : DECINF;
                return acc;
                }

        _ConstRepIter iter(bytes);

        // Handle "hidden" bit.
        if (!_IsSubnormal() && (as_ldbl != 0.0L))
                { decNumberAdd(&acc, &acc, &powerOfHalf, &ctx); }
#if !defined(_DEC_HAS_IMPLICIT_BIT)
        iter._NextBit();
#endif /* defined(_DEC_HAS_IMPLICIT_BIT) */

        while (!iter._IsDone())
                {
                // Divide power_of_half by 2.
                // We accomplish this by calculating (powerOfHalf * 5)/10.
                decNumberMultiply(&powerOfHalf, &powerOfHalf, &five, &ctx);
                --powerOfHalf.exponent; // divide by 10

                if (iter._GetBit())
                        { decNumberAdd(&acc, &acc, &powerOfHalf, &ctx); }
                iter._NextBit();
                }

        // do exponent
        static const _DecNumber two(2);
        _DecNumber exp2(_GetExponent());
        _DecNumber scale10; 
        decNumberPower(&scale10, &two, &exp2, &ctx);
        decNumberMultiply(&acc, &acc, &scale10, &ctx);
        decNumberNormalize(&acc, &acc, &ctx);

        // do sign
        if (_IsNeg())
                { acc.bits |= DECNEG; }

        // Check for subnormals.
        if (acc.exponent == ctx.emin)
                { decContextSetStatus(&ctx, DEC_Underflow); }

        return acc;
        }

_DecNumber::_DecNumber()
        {
        // These members cannot be initialized in a ctor-initializer
        // list because they are actually members of the base class.
        digits = 1;
        exponent = 0;
        bits = 0;
        for (size_t i = 0; i != lsu_extent; ++i)
                { lsu[i] = 0; }
        }

_DecNumber::_DecNumber(_DecBase<_FmtTraits<decimal32> > source)
        {
        for (size_t i = 0; i != lsu_extent; ++i)
                { lsu[i] = 0; }
        decimal32ToNumber(reinterpret_cast< ::decimal32 *>(&source), this);
        }

_DecNumber::_DecNumber(_DecBase<_FmtTraits<decimal64> > source)
        {
        for (size_t i = 0; i != lsu_extent; ++i)
                { lsu[i] = 0; }
        decimal64ToNumber(reinterpret_cast< ::decimal64 *>(&source), this);
        }

_DecNumber::_DecNumber(_DecBase<_FmtTraits<decimal128> > source)
        {
        for (size_t i = 0; i != lsu_extent; ++i)
                { lsu[i] = 0; }
        decimal128ToNumber(reinterpret_cast< ::decimal128 *>(&source), this);
        }

_DecNumber::_DecNumber(long double source)
        {
        _LongDouble ldbl(source);
        *this = ldbl.binaryToDecimal();
        }

_DecNumber::_DecNumber(const _DecNumber & source)
        {
        // These members cannot be initialized in a ctor-initializer
        // list because they are actually members of the base class.
        digits = source.digits;
        exponent = source.exponent;
        bits = source.bits;
        for (size_t i = 0; i != lsu_extent; ++i)
                { lsu[i] = source.lsu[i]; }
        }

_DecNumber & _DecNumber::operator=(const _DecNumber & source)
        {
        digits = source.digits;
        exponent = source.exponent;
        bits = source.bits;
        for (size_t i = 0; i != lsu_extent; ++i)
                { lsu[i] = source.lsu[i]; }
        return *this;
        }

long double _DecNumber::_ToLongDouble() const
        {
        _LongDouble ldbl(*this);
        return ldbl;
        }

void _DecNumber::_FromIntegral(unsigned long long source)
        {
        exponent = 0;

        if (source == 0)
                { digits = 1; }
        else
                { digits = (int32_t)log10(double(source)) + 1; }

        lsu[0] = source % (DECDPUNMAX + 1);
        for (size_t i = 1; i != lsu_extent; ++i)
                {
                source /= (DECDPUNMAX + 1);
                lsu[i] = source % (DECDPUNMAX + 1);
                }
        }

long long _DecNumber::_ToSignedIntegral() const
        {
        static const int32_t maxDigits =
                std::numeric_limits<long long>::digits10 + 1;

        long long target = 0L;

        if (bits & DECINF)
                { target = bits & DECNEG ? LLONG_MIN : LLONG_MAX; }
        else if (bits & DECSPECIAL)
                { target = LLONG_MIN; }
        else
                {
                _DecNumber tmp;
                _ContextDefault ctx;
                ctx.digits = maxDigits + 1;
                ctx.round = DEC_ROUND_DOWN;
                decNumberToIntegralValue(&tmp, this, &ctx);

                if (tmp.exponent > 0)
                        {
                        if (tmp.digits + tmp.exponent <= maxDigits)
                                {
                                _DecNumber zero(0);
                                decNumberQuantize(&tmp, &tmp, &zero, &ctx);
                                }
                        else
                                { return _IsNeg() ? LLONG_MIN : LLONG_MAX; }
                        }

                for (size_t i = DECNUMUNITS; i != 0; --i)
                        {
                        target *= DECDPUNMAX + 1;
                        target += tmp.lsu[i - 1];
                        }
                if (_IsNeg())
                        { target = -target; }
                }
        return target;
        }

void _DecNumber::_ToDecimal32(_DecBase<_FmtTraits<decimal32> > * _Tgt,
                              _Context32 * _Ctxt) const
        {
        decimal32FromNumber(reinterpret_cast< ::decimal32 *>(_Tgt),
                            this, _Ctxt);
        }

void _DecNumber::_ToDecimal64(_DecBase<_FmtTraits<decimal64> > * _Tgt,
                              _Context64 * _Ctxt) const
        {
        decimal64FromNumber(reinterpret_cast< ::decimal64 *>(_Tgt),
                            this, _Ctxt);
        }

void _DecNumber::_ToDecimal128(_DecBase<_FmtTraits<decimal128> > * _Tgt,
                               _Context128 * _Ctxt) const
        {
        decimal128FromNumber(reinterpret_cast< ::decimal128 *>(_Tgt),
                             this, _Ctxt);
        }

} // namespace decimal
} // namespace std
