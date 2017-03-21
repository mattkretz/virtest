/*{{{
Copyright © 2014-2017 Matthias Kretz <kretz@kde.org>

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

#ifndef VIR_TYPETOSTRING_H_
#define VIR_TYPETOSTRING_H_

#include "typelist.h"
#include "detail/vc_fwd.h"
#include <array>
#include <sstream>
#include <string>
#include <typeinfo>
#include <type_traits>
#include <vector>

#ifdef __has_include
#  if __has_include(<cxxabi.h>)
#    include <cxxabi.h>
#    define VIR_HAVE_CXXABI_H 1
#  endif
#elif defined HAVE_CXX_ABI_H
#  include <cxxabi.h>
#  define VIR_HAVE_CXXABI_H 1
#endif // __has_include

namespace vir
{
template <typename T> inline std::string typeToString();

namespace detail
{
// unused{{{1
template <class T> static constexpr void unused(T &&) {}

// std::array<T, N> {{{1
template <typename T, std::size_t N>
inline std::string typeToString_impl(std::array<T, N> *)
{
  std::stringstream s;
  s << "array<" << typeToString<T>() << ", " << N << '>';
  return s.str();
}

// std::vector<T> {{{1
template <typename T> inline std::string typeToString_impl(std::vector<T> *)
{
  std::stringstream s;
  s << "vector<" << typeToString<T>() << '>';
  return s.str();
}

// std::integral_constant<T, N> {{{1
template <typename T, T N>
inline std::string typeToString_impl(std::integral_constant<T, N> *)
{
  std::stringstream s;
  s << "integral_constant<" << N << '>';
  return s.str();
}

// template parameter pack to a comma separated string {{{1
std::string typeToString_impl(Typelist<> *) { return "{}"; }

template <typename T0, typename... Ts>
std::string typeToString_impl(Typelist<T0, Ts...> *)
{
  std::stringstream s;
  s << '{' << typeToString<T0>();
  auto &&x = {(s << ", " << typeToString<Ts>(), 0)...};
  unused(x);
  s << '}';
  return s.str();
}

// Vc::datapar to string {{{1
template <int N> inline std::string typeToString_impl(Vc::datapar_abi::fixed_size<N> *)
{
    return std::to_string(N);
}
inline std::string typeToString_impl(Vc::datapar_abi::scalar *) { return "scalar"; }
inline std::string typeToString_impl(Vc::datapar_abi::sse *) { return "sse"; }
inline std::string typeToString_impl(Vc::datapar_abi::avx *) { return "avx"; }
inline std::string typeToString_impl(Vc::datapar_abi::avx512 *) { return "avx512"; }
inline std::string typeToString_impl(Vc::datapar_abi::knc *) { return "knc"; }
inline std::string typeToString_impl(Vc::datapar_abi::neon *) { return "neon"; }
template <class T, class A> inline std::string typeToString_impl(Vc::datapar<T, A> *)
{
    return "datapar<" + typeToString<T>() + ", " + typeToString<A>() + '>';
}
template <class T, class A> inline std::string typeToString_impl(Vc::mask<T, A> *)
{
    return "mask<" + typeToString<T>() + ", " + typeToString<A>() + '>';
}

// generic fallback (typeid::name) {{{1
template <typename T> inline std::string typeToString_impl(T *)
{
#ifdef VIR_HAVE_CXXABI_H
  char buf[1024];
  size_t size = 1024;
  abi::__cxa_demangle(typeid(T).name(), buf, &size, nullptr);
  return std::string{buf};
#else
  return typeid(T).name();
#endif
}

//}}}1
}  // namespace detail

// typeToString specializations {{{1
template <typename T> inline std::string typeToString()
{
  using tag = T *;
  return detail::typeToString_impl(tag());
}

template <> inline std::string typeToString<void>() { return ""; }
template <> inline std::string typeToString<long double>() { return "ldoubl"; }
template <> inline std::string typeToString<double>() { return "double"; }
template <> inline std::string typeToString<float>() { return " float"; }
template <> inline std::string typeToString<long long>() { return " llong"; }
template <> inline std::string typeToString<unsigned long long>() { return "ullong"; }
template <> inline std::string typeToString<long>() { return "  long"; }
template <> inline std::string typeToString<unsigned long>() { return " ulong"; }
template <> inline std::string typeToString<int>() { return "   int"; }
template <> inline std::string typeToString<unsigned int>() { return "  uint"; }
template <> inline std::string typeToString<short>() { return " short"; }
template <> inline std::string typeToString<unsigned short>() { return "ushort"; }
template <> inline std::string typeToString<char>() { return "  char"; }
template <> inline std::string typeToString<unsigned char>() { return " uchar"; }
template <> inline std::string typeToString<signed char>() { return " schar"; }

//}}}1
}  // namespace vir

// vim: foldmethod=marker
#endif  // VIR_TYPETOSTRING_H_
