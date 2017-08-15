/*{{{
Copyright © 2011-2017 Matthias Kretz <kretz@kde.org>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the names of contributing organizations nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

}}}*/

#ifndef VIR_DETAIL_ULP_H_
#define VIR_DETAIL_ULP_H_

#include "vc_fwd.h"
#include <cmath>
#include <limits>

namespace vir
{
namespace detail
{
template <class T,
          class = typename std::enable_if<std::is_floating_point<T>::value>::type>
using FloatingPoint = T;

/**
 * Returns the distance in ULP of \p ref from \p ref to \p val.
 *
 * This algorithm does not return the number of possible float representations between the given two
 * values. Instead it determines the value of the least significant bit in the mantissa of the
 * reference value \p ref. This is considered as one ULP - unit in the last place (or unit of least
 * precision). The function then returns abs(val - ref) / ULP. The order of arguments therefore
 * matters.
 *
 * Denormals, NaN, and infinities are not supported.
 *
 * If \p val and \p ref have a different sign, the reported distance is very large. Since having the
 * wrong sign can have huge effects on certain algorithms this is considered a feature rather than a
 * bug.
 */
template <typename T>
inline T ulpDiffToReference(FloatingPoint<T> val, FloatingPoint<T> ref)
{
  using namespace std;
  using std::isnan;  // work around old libc that defines ::isnan(double)
  if (val == ref || (isnan(val) && isnan(ref))) {
    return 0;
  }
  if (ref == T(0)) {
    return 1 + ulpDiffToReference(abs(val), numeric_limits<T>::min());
  }
  if (val == T(0)) {
    return 1 + ulpDiffToReference(numeric_limits<T>::min(), abs(ref));
  }

  int exp;
  /*tmp = */ frexp(ref, &exp);  // ref == tmp * 2 ^ exp => tmp == ref * 2 ^ -exp
  // tmp is now in the range [0.5, 1.0[
  // now we want to know how many times we can fit 2^-numeric_limits<T>::digits between
  // tmp and
  // val * 2 ^ -exp
  return ldexp(abs(ref - val), numeric_limits<T>::digits - exp);
}

template <typename T, typename A>
inline Vc::simd<T, A> ulpDiffToReference(const Vc::simd<T, A> &val_,
                                         const Vc::simd<T, A> &ref_)
{
  using V = Vc::simd<T, A>;
  using M = typename V::mask_type;

  V val = val_;
  V ref = ref_;

  V diff = V();

  M zeroMask = ref == V();
  where(zeroMask, val) = abs(val);
  where(zeroMask, ref) = std::numeric_limits<V>::min();
  where(zeroMask, diff) = 1;
  zeroMask = val == V();
  where(zeroMask, ref) = abs(ref);
  where(zeroMask, val) = std::numeric_limits<V>::min();
  where(zeroMask, diff) += 1;

  Vc::simd<int, Vc::simd_abi::fixed_size<V::size()>> exp;
  frexp(ref, &exp);
  diff += ldexp(abs(ref - val), std::numeric_limits<T>::digits - exp);
  where(val_ == ref_ || (isnan(val_) && isnan(ref_)), diff) = V();
  return diff;
}

template <typename T, typename A>
inline typename std::enable_if<std::is_floating_point<T>::value, Vc::simd<T, A>>::type
ulpDiffToReferenceSigned(const Vc::simd<T, A> &_val, const Vc::simd<T, A> &_ref)
{
  return copysign(ulpDiffToReference(_val, _ref), _val - _ref);
}

template <typename T, typename A>
inline typename std::enable_if<!std::is_floating_point<T>::value, Vc::simd<T, A>>::type
ulpDiffToReferenceSigned(const Vc::simd<T, A> &, const Vc::simd<T, A> &)
{
  return 0;
}

template <typename T>
inline T ulpDiffToReferenceSigned(FloatingPoint<T> val, FloatingPoint<T> ref)
{
  return ulpDiffToReference(val, ref) * (val - ref < 0 ? -1 : 1);
}

}  // namespace detail
}  // namespace vir

#endif  // VIR_DETAIL_ULP_H_
// vim: sw=2 et sts=2 foldmethod=marker
