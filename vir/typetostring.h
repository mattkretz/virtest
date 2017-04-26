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
namespace detail
{
template <typename T> constexpr auto typeToStringRecurse();

template <int N> class constexpr_string
{
  const std::array<char, N> chars;
  static constexpr std::make_index_sequence<N> index_seq{};

  template <int M, std::size_t... Ls, std::size_t... Rs>
  constexpr constexpr_string<N + M> concat(const constexpr_string<M> &rhs,
                                           std::index_sequence<Ls...>,
                                           std::index_sequence<Rs...>) const
  {
    return {chars[Ls]..., rhs[Rs]...};
  }

public:
  constexpr constexpr_string(const char c) : chars{c} {}
  constexpr constexpr_string(const std::initializer_list<char> &c)
      : constexpr_string(&*c.begin(), index_seq)
  {
  }
  constexpr constexpr_string(const char str[N]) : constexpr_string(str, index_seq) {}
  template <std::size_t... Is>
  constexpr constexpr_string(const char str[N], std::index_sequence<Is...>)
      : chars{str[Is]...}
  {
  }

  template <int M>
  constexpr constexpr_string<N + M> operator+(const constexpr_string<M> &rhs) const
  {
    return concat(rhs, std::make_index_sequence<N>(), std::make_index_sequence<M>());
  }

  constexpr char operator[](size_t i) const { return chars[i]; }
  operator std::string() const { return {&chars[0], N}; }

  friend std::string operator+(const std::string &lhs, const constexpr_string &rhs)
  {
    return lhs + static_cast<std::string>(rhs);
  }
  friend std::string operator+(const constexpr_string &lhs, const std::string &rhs)
  {
    return static_cast<std::string>(lhs) + rhs;
  }

  friend std::ostream &operator<<(std::ostream &lhs, const constexpr_string &rhs)
  {
    return lhs.write(&rhs.chars[0], N);
  }
};

constexpr constexpr_string<1> number_to_string(std::integral_constant<int, 0>) { return '0'; }
constexpr constexpr_string<1> number_to_string(std::integral_constant<int, 1>) { return '1'; }
constexpr constexpr_string<1> number_to_string(std::integral_constant<int, 2>) { return '2'; }
constexpr constexpr_string<1> number_to_string(std::integral_constant<int, 3>) { return '3'; }
constexpr constexpr_string<1> number_to_string(std::integral_constant<int, 4>) { return '4'; }
constexpr constexpr_string<1> number_to_string(std::integral_constant<int, 5>) { return '5'; }
constexpr constexpr_string<1> number_to_string(std::integral_constant<int, 6>) { return '6'; }
constexpr constexpr_string<1> number_to_string(std::integral_constant<int, 7>) { return '7'; }
constexpr constexpr_string<1> number_to_string(std::integral_constant<int, 8>) { return '8'; }
constexpr constexpr_string<1> number_to_string(std::integral_constant<int, 9>) { return '9'; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 10>) { return "10"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 11>) { return "11"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 12>) { return "12"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 13>) { return "13"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 14>) { return "14"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 15>) { return "15"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 16>) { return "16"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 17>) { return "17"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 18>) { return "18"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 19>) { return "19"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 20>) { return "20"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 21>) { return "21"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 22>) { return "22"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 23>) { return "23"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 24>) { return "24"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 25>) { return "25"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 26>) { return "26"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 27>) { return "27"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 28>) { return "28"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 29>) { return "29"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 30>) { return "30"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 31>) { return "31"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 32>) { return "32"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 33>) { return "33"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 34>) { return "34"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 35>) { return "35"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 36>) { return "36"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 37>) { return "37"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 38>) { return "38"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 39>) { return "39"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 40>) { return "40"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 41>) { return "41"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 42>) { return "42"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 43>) { return "43"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 44>) { return "44"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 45>) { return "45"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 46>) { return "46"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 47>) { return "47"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 48>) { return "48"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 49>) { return "49"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 50>) { return "50"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 51>) { return "51"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 52>) { return "52"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 53>) { return "53"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 54>) { return "54"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 55>) { return "55"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 56>) { return "56"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 57>) { return "57"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 58>) { return "58"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 59>) { return "59"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 60>) { return "60"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 61>) { return "61"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 62>) { return "62"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 63>) { return "63"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 64>) { return "64"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 65>) { return "65"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 66>) { return "66"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 67>) { return "67"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 68>) { return "68"; }
constexpr constexpr_string<2> number_to_string(std::integral_constant<int, 69>) { return "69"; }
// fallback
template <int N> inline std::string number_to_string(std::integral_constant<int, N>)
{
  return std::to_string(N);
}

// std::array<T, N> {{{1
template <typename T, std::size_t N>
constexpr auto typeToString_impl(std::array<T, N> *)
{
  return constexpr_string<6>("array<") + typeToStringRecurse<T>() + constexpr_string<2>(", ") +
         number_to_string(std::integral_constant<int, N>()) + constexpr_string<1>('>');
}

// std::vector<T> {{{1
template <typename T> constexpr auto typeToString_impl(std::vector<T> *)
{
  return constexpr_string<7>("vector<") + typeToStringRecurse<T>() + constexpr_string<1>('>');
}

// std::integral_constant<T, N> {{{1
template <typename T, T N>
constexpr auto typeToString_impl(std::integral_constant<T, N> *)
{
  return constexpr_string<18>("integral_constant<") +
         number_to_string(std::integral_constant<int, N>()) + constexpr_string<1>('>');
}

// template parameter pack to a comma separated string {{{1
constexpr_string<2> typeToString_impl(Typelist<> *) { return "{}"; }

template <typename T0, typename... Ts>
std::string typeToString_impl(Typelist<T0, Ts...> *)
{
  std::stringstream s;
  s << '{' << typeToStringRecurse<T0>();
  const auto &x = {(s << ", " << typeToStringRecurse<Ts>(), 0)...};
  [](decltype(x)) {}(x);  // silence "x is unused" warning
  s << '}';
  return s.str();
}

// Vc::datapar to string {{{1
template <int N> constexpr auto typeToString_impl(Vc::datapar_abi::fixed_size<N> *)
{
  return number_to_string(std::integral_constant<int, N>());
}
constexpr constexpr_string<6> typeToString_impl(Vc::datapar_abi::scalar *) { return "scalar"; }
constexpr constexpr_string<3> typeToString_impl(Vc::datapar_abi::sse *) { return "sse"; }
constexpr constexpr_string<3> typeToString_impl(Vc::datapar_abi::avx *) { return "avx"; }
constexpr constexpr_string<6> typeToString_impl(Vc::datapar_abi::avx512 *) { return "avx512"; }
constexpr constexpr_string<3> typeToString_impl(Vc::datapar_abi::knc *) { return "knc"; }
constexpr constexpr_string<4> typeToString_impl(Vc::datapar_abi::neon *) { return "neon"; }
template <class T, class A> constexpr auto typeToString_impl(Vc::datapar<T, A> *)
{
  return constexpr_string<8>("datapar<") + typeToStringRecurse<T>() + constexpr_string<2>(", ") +
         typeToStringRecurse<A>() + constexpr_string<1>('>');
}
template <class T, class A> constexpr auto typeToString_impl(Vc::mask<T, A> *)
{
  return constexpr_string<5>("mask<") + typeToStringRecurse<T>() + constexpr_string<2>(", ") +
         typeToStringRecurse<A>() + constexpr_string<1>('>');
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

constexpr constexpr_string<0> typeToString_impl(void *);// { return ""; }
constexpr constexpr_string<6> typeToString_impl(long double *) { return "ldoubl"; }
constexpr constexpr_string<6> typeToString_impl(double *) { return "double"; }
constexpr constexpr_string<6> typeToString_impl(float *) { return " float"; }
constexpr constexpr_string<6> typeToString_impl(long long *) { return " llong"; }
constexpr constexpr_string<6> typeToString_impl(unsigned long long *) { return "ullong"; }
constexpr constexpr_string<6> typeToString_impl(long *) { return "  long"; }
constexpr constexpr_string<6> typeToString_impl(unsigned long *) { return " ulong"; }
constexpr constexpr_string<6> typeToString_impl(int *) { return "   int"; }
constexpr constexpr_string<6> typeToString_impl(unsigned int *) { return "  uint"; }
constexpr constexpr_string<6> typeToString_impl(short *) { return " short"; }
constexpr constexpr_string<6> typeToString_impl(unsigned short *) { return "ushort"; }
constexpr constexpr_string<6> typeToString_impl(char *) { return "  char"; }
constexpr constexpr_string<6> typeToString_impl(unsigned char *) { return " uchar"; }
constexpr constexpr_string<6> typeToString_impl(signed char *) { return " schar"; }

template <typename T> constexpr auto typeToStringRecurse()
{
  using tag = T *;
  return typeToString_impl(tag());
}
//}}}1
}  // namespace detail

// typeToString specializations {{{1
template <typename T> static std::string typeToString()
{
  using tag = T *;
  static const std::string tmp = detail::typeToString_impl(tag());
  return tmp;
}

//}}}1
}  // namespace vir

// vim: foldmethod=marker
#endif  // VIR_TYPETOSTRING_H_
