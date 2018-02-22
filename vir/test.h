/*{{{
Copyright © 2009-2017 Matthias Kretz <kretz@kde.org>

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

#ifndef VIR_TEST_H_
#define VIR_TEST_H_

#include "typelist.h"
#include "typetostring.h"
#include "detail/color.h"
#include "detail/ulp.h"
#include "detail/type_traits.h"
#include <array>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <typeinfo>
#include <vector>
#ifdef HAVE_CXX_ABI_H
#include <cxxabi.h>
#endif

#ifdef DOXYGEN

/**
 * \defgroup unittest Unit Testing
 * @{
 *
 * In Vc we use a unit testing framework that was developed for easy use with typelists (i.e. the Vc
 * SIMD types).
 * It simplifies test creation to the bare minimum. The following code suffices to
 * run a test:
 * \code
 * #include "tests/unittest.h"
 *
 * TEST(test_name) {
 *   int test = 1 + 1;
 *   COMPARE(test, 2) << "more details";
 *   VERIFY(1 > 0);
 * }
 * \endcode
 * This creates one test function (called "test_name"). This function is called
 * without any further code and executes to checks. If, for some reason, the
 * compiler would determine that test needs to have the value 3, then the output
 * would be:
   \verbatim
    FAIL: ┍ at tests/testfile.cpp:5 (0x40451f):
    FAIL: │ test (3) == 2 (2) -> false more details
    FAIL: ┕ test_name

    Testing done. 0 tests passed. 1 tests failed.
   \endverbatim
 * Let's take a look at what this tells us.
 * 1. The test macro that failed was in testfile.cpp in line 5.
 * 2. If you want to look at the disassembly, the failure was at 0x40451f.
 * 3. The COMPARE macro compared the expression `test` against the expression
 *    `2`. It shows that `test` had a value of `3` while `2` had a value of `2`
 *    (what a surprise). Since the values are not equal `test == 2` returns \c
 *    false.
 * 4. The COMPARE, FUZZY_COMPARE, VERIFY, and FAIL macros can be used as
 *    streams. The output will only appear on failure and will be printed right
 *    after the normal output of the macro.
 * 5. Finally the name of the failed test (the name specified inside the TEST()
 *    macro) is printed.
 * 6. At the end of the run, a summary of the test results is shown. This may be
 *    important when there are many TEST functions.
 *
 * If the test passed you'll see:
   \verbatim
    PASS: test_name

    Testing done. 1 tests passed. 0 tests failed.
   \endverbatim
 */

/**
 * \brief Defines a test function.
 *
 * Consider this to expand to `void
 * function_name()`. The function_name will also be the name that appears in the
 * output after PASS/FAIL.
 */
#define TEST(function_name)

/**
 * \brief Same as above, but expects the code to throw an exception of type \p
 * ExceptionType.
 *
 * If the code does not throw (or throws a different exception),
 * the test is considered failed.
 */
#define TEST_CATCH(function_name, ExceptionType)

/**
 * \brief Tests that should be tested with several types as template parameter
 * can use this macro.
 *
 * Your test function then has this signature: `template <typename
 * T> void function_name()`.
 */
#define TEST_BEGIN(T, function_name, typelist)

/**
 * \brief Test functions created with TEST_BEGIN need to end with TEST_END.
 */
#define TEST_END

/**
 * \brief Verifies that \p condition is \c true.
 */
#define VERIFY(condition)

/**
 * \brief Verifies that \p test_value is equal to \p reference.
 */
#define COMPARE(test_value, reference)

/**
 * \brief Verifies that the difference between \p test_value and \p reference is
 * smaller than \p allowed_difference.
 *
 * If the test fails the output will show the actual difference between \p
 * test_value and \p reference. If this value is positive \p test_value is too
 * large. If it is negative \p test_value is too small.
 */
#define COMPARE_ABSOLUTE_ERROR(test_value, reference, allowed_difference)

/**
 * \brief Verifies that the difference between \p test_value and \p reference is
 * smaller than `allowed_relative_difference * reference`.
 *
 * If the test fails the output will show the actual difference between \p
 * test_value and \p reference. If this value is positive \p test_value is too
 * large. If it is negative \p test_value is too small.
 *
 * The following example tests that `a` is no more than 1% different from `b`:
 * \code
 * COMPARE_ABSOLUTE_ERROR(a, b, 0.01);
 * \endcode
 *
 * \note This test macro still works even if \p reference is set to 0. It will
 * then compare the difference against `allowed_relative_difference * <smallest
 * positive normalized value of reference type>`.
 */
#define COMPARE_RELATIVE_ERROR(test_value, reference, allowed_relative_difference)

/**
 * \brief Verifies that \p test_value is equal to \p reference within a
 * pre-defined distance in units of least precision (ulp).
 *
 * If the test fails it will print the distance in ulp between \p test_value and
 * \p reference as well as the maximum allowed distance. Often this difference
 * is not visible in the value because the conversion of a double/float to a
 * string needs to round the value to a sensible length.
 *
 * The allowed distance can be modified by calling:
 * \code
 * vir::test::setFuzzyness<float>(4);
 * vir::test::setFuzzyness<double>(7);
 * \endcode
 *
 * ### ulp
 * Unit of least precision is a unit that is derived from the the least
 * significant bit in the mantissa of a floating-point value. Consider a
 * single-precision number (23 mantissa bits) with exponent \f$e\f$. Then 1
 * ulp is \f$2^{e-23}\f$. Thus, \f$\log_2(u)\f$ signifies the the number
 * incorrect mantissa bits (with \f$u\f$ the distance in ulp).
 *
 * If \p test_value and \p reference have a different exponent the meaning of
 * ulp depends on the variable you look at. The FUZZY_COMPARE code always uses
 * \p reference to determine the magnitude of 1 ulp.
 *
 * Example:
 * The value `1.f` is `0x3f800000` in binary. The value
 * `1.00000011920928955078125f` with binary representation `0x3f800001`
 * therefore has a distance of 1 ulp.
 * A positive distance means the \p test_value is larger than the \p reference.
 * A negative distance means the \p test_value is smaller than the \p reference.
 * * `FUZZY_COMPARE(1.00000011920928955078125f, 1.f)` will show a distance of 1
 * * `FUZZY_COMPARE(1.f, 1.00000011920928955078125f)` will show a distance of -1
 *
 * The value `0.999999940395355224609375f` with binary representation
 * `0x3f7fffff` has a smaller exponent than `1.f`:
 * * `FUZZY_COMPARE(0.999999940395355224609375f, 1.f)` will show a distance of
 * -0.5
 * * `FUZZY_COMPARE(1.f, 0.999999940395355224609375f)` will show a distance of 1
 *
 * ### Comparing to 0
 * Distance to 0 is implemented as comparing to `std::numeric_limits<T>::min()`
 * instead and adding 1 to the resulting distance.
 */
#define FUZZY_COMPARE(test_value, reference)

/**
 * \brief Call this to fail a test.
 */
#define FAIL()

/**
 * \brief Wrap code that should fail an assertion with this macro.
 */
#define EXPECT_ASSERT_FAILURE(code)

/**
 * @}
 */

#else

namespace vir
{
namespace test
{
template <typename T> inline void setFuzzyness(T fuzz);
/** \internal
 * Implementation namespace
 */
namespace detail
{
// using statements {{{1
using std::vector;
using std::get;

// value_type_or_T {{{1
template <class T> typename T::value_type value_type_or_T_impl(int);
template <class T> T value_type_or_T_impl(float);
template <class T> using value_type_or_T = decltype(value_type_or_T_impl<T>(int()));

// printPass {{{1
static inline void printPass()
{
  static const char *str = 0;
  if (str == 0) {
    if (vir::detail::may_use_color(std::cout)) {
      static const char *const pass = " \033[1;40;32mPASS:\033[0m ";
      str = pass;
    } else {
      static const char *const pass = " PASS: ";
      str = pass;
    }
  }
  std::cout << str;
}
static inline void printSkip()
{
  std::cout << vir::detail::color::yellow << " SKIP: " << vir::detail::color::normal;
}

class UnitTestFailure  //{{{1
{
};

struct SkippedTest  //{{{1
{
  std::string message;
};

using TestFunction = void (*)(void);  //{{{1

class UnitTester  //{{{1
{
public:
  UnitTester()
      : status(true)
      , expect_failure(false)
      , expect_assert_failure(false)
      , float_fuzzyness(1.f)
      , double_fuzzyness(1.)
      , only_name(0)
      , m_finalized(false)
      , failedTests(0)
      , passedTests(0)
      , skippedTests(0)
      , findMaximumDistance(false)
      , maximumDistance(0)
      , meanDistance(0)
      , meanCount(0)
  {
  }

  int finalize()
  {
    if (plotFile.is_open()) {
      plotFile.flush();
      plotFile.close();
    }
    m_finalized = true;
    std::cout << "\n Testing done. " << passedTests << " tests passed. " << failedTests
              << " tests failed. " << skippedTests << " tests skipped." << std::endl;
    return failedTests;
  }

  void runTestInt(TestFunction fun, const char *name);

  bool status;
  bool expect_failure;
  bool expect_assert_failure;
  float float_fuzzyness;
  double double_fuzzyness;
  const char *only_name;
  bool vim_lines = false;
  std::fstream plotFile;

  template <class T> T &fuzzyness();

private:
  bool m_finalized;
  int failedTests;

public:
  int passedTests;
  int skippedTests;
  bool findMaximumDistance;
  double maximumDistance;
  double meanDistance;
  int meanCount;
};
template <> float &UnitTester::fuzzyness<float>() { return float_fuzzyness; }
template <> double &UnitTester::fuzzyness<double>() { return double_fuzzyness; }

static UnitTester global_unit_test_object_;

static const char *failString()  // {{{1
{
  if (global_unit_test_object_.expect_failure) {
    return "XFAIL: ";
  }
  static const char *str = 0;
  if (str == 0) {
    if (vir::detail::may_use_color(std::cout)) {
      static const char *const fail = " \033[1;40;31mFAIL:\033[0m ";
      str = fail;
    } else {
      static const char *const fail = " FAIL: ";
      str = fail;
    }
  }
  return str;
}

void UnitTester::runTestInt(TestFunction fun, const char *name)  //{{{1
{
  if (global_unit_test_object_.only_name &&
      0 != std::strcmp(name, global_unit_test_object_.only_name)) {
    return;
  }
  global_unit_test_object_.status = true;
  global_unit_test_object_.expect_failure = false;
  try {
    setFuzzyness<float>(1);
    setFuzzyness<double>(1);
    maximumDistance = 0.;
    meanDistance = 0.;
    meanCount = 0;
    fun();
  } catch (const SkippedTest &skip) {
    printSkip();
    std::cout << name << ' ' << skip.message << std::endl;
    ++skippedTests;
    return;
  } catch (UnitTestFailure) {
  } catch (std::exception &e) {
    std::cout << failString() << "┍ " << name << " threw an unexpected exception:\n";
    std::cout << failString() << "│ " << e.what() << '\n';
    global_unit_test_object_.status = false;
  } catch (...) {
    std::cout << failString() << "┍ " << name
              << " threw an unexpected exception, of unknown type\n";
    global_unit_test_object_.status = false;
  }
  if (global_unit_test_object_.expect_failure) {
    if (!global_unit_test_object_.status) {
      std::cout << "XFAIL: " << name << std::endl;
    } else {
      std::cout << "unexpected PASS: " << name
                << "\n    This test should have failed but didn't. Check the code!"
                << std::endl;
      ++failedTests;
    }
  } else {
    if (!global_unit_test_object_.status) {
      if (findMaximumDistance) {
        std::cout << failString() << "│ with a maximal distance of " << maximumDistance
                  << " to the reference (mean: " << meanDistance / meanCount << ").\n";
      }
      std::cout << failString();
      if (!vim_lines) {
        std::cout << "┕ ";
      }
      std::cout << name << std::endl;
      if (vim_lines) {
        std::cout << '\n';
      }
      ++failedTests;
    } else {
      printPass();
      std::cout << name;
      if (findMaximumDistance) {
        if (maximumDistance > 0.) {
          std::cout << " with a maximal distance of " << maximumDistance
                    << " to the reference (mean: " << meanDistance / meanCount << ").";
        } else {
          std::cout << " all values matched the reference precisely.";
        }
      }
      std::cout << std::endl;
      ++passedTests;
    }
  }
}

// all_of {{{1
VIR_ALWAYS_INLINE bool all_of(bool x) { return x; }

// unittest_compareHelper {{{1
template <typename T1, typename T2>
VIR_ALWAYS_INLINE bool unittest_compareHelper(const T1 &a, const T2 &b)
{
  return all_of(a == b);
}

template <>
VIR_ALWAYS_INLINE bool unittest_compareHelper<std::type_info, std::type_info>(
    const std::type_info &a, const std::type_info &b)
{
  return &a == &b;
}

// ulpDiffToReferenceWrapper {{{1
namespace detail
{
using std::abs;
template <typename T, typename U = decltype(double(abs(std::declval<const T &>())))>
T ulpDiffToReferenceWrapper(T a, T b, int)
{
  using vir::detail::ulpDiffToReference;
  const T diff = ulpDiffToReference(a, b);
  if (VIR_IS_UNLIKELY(global_unit_test_object_.findMaximumDistance)) {
    using std::abs;
    using std::max;
    global_unit_test_object_.maximumDistance =
        max(double(abs(diff)), global_unit_test_object_.maximumDistance);
    global_unit_test_object_.meanDistance += abs(diff);
    ++global_unit_test_object_.meanCount;
  }
  return diff;
}

template <class T> struct UlpDiffToReferenceWrapperLambda {
  const T &a;
  const T &b;
  template <class U> auto operator()(U i) -> typename std::decay<decltype(a[i])>::type
  {
    using vir::detail::ulpDiffToReference;
    return ulpDiffToReference(a[i], b[i]);
  }
};
template <typename T> T ulpDiffToReferenceWrapper(const T a, const T b, float)
{
  UlpDiffToReferenceWrapperLambda<T> lambda = {a, b};
  return T(lambda);
}
}  // namespace detail

// unittest_fuzzyCompareHelper {{{1
template <typename T>
static inline bool unittest_fuzzyCompareHelper(const T &a, const T &b, std::true_type)
{
  using U = value_type_or_T<T>;
  return all_of(detail::ulpDiffToReferenceWrapper(a, b, int()) <=
                global_unit_test_object_.fuzzyness<U>());
}
template <typename T>
static inline bool unittest_fuzzyCompareHelper(const T &a, const T &b, std::false_type)
{
  return all_of(a == b);
}

class Compare  //{{{1
{
  // absoluteErrorTest{{{2
  template <typename T, typename ET>
  static bool absoluteErrorTest(const T &a, const T &b, ET error)
  {
    if (a > b) {  // don't use abs(a - b) because it doesn't work for unsigned
                  // integers
      return a - b > error;
    } else {
      return b - a > error;
    }
  }
  // relativeErrorTest{{{2
  template <typename T, typename ET>
  static bool relativeErrorTest(const T &a, const T &b, ET error)
  {
    if (b > 0) {
      error *= b;
    } else if (b < 0) {
      error *= -b;
    } else if (std::is_floating_point<T>::value) {
      // if the reference value is 0 then use the smallest normalized number
      error *= std::numeric_limits<T>::min();
    } else {
      // error *= 1;  // the smallest non-zero positive number is 1...
    }
    if (a > b) {  // don't use abs(a - b) because it doesn't work for unsigned
                  // integers
      return a - b > error;
    } else {
      return b - a > error;
    }
  }

public:
  // tag types {{{2
  struct Fuzzy {};
  struct NoEq {};
  struct AbsoluteError {};
  struct RelativeError {};
  struct Mem {};

  // Normal Compare ctor {{{2
#if (defined __x86_64__ || defined __amd64__ || defined __i686__ || defined __i386__) && \
    !defined __SSE2__
#define VIR_NEED_FUZZY_FLOAT_COMPARE_ 1
#else
#define VIR_NEED_FUZZY_FLOAT_COMPARE_ 0
#endif
  template <typename T1, typename T2>
  VIR_ALWAYS_INLINE Compare(
      const T1 &a, const T2 &b, const char *_a, const char *_b, const char *_file,
      typename std::enable_if<vir::detail::has_equality_operator<T1, T2>::value
#if VIR_NEED_FUZZY_FLOAT_COMPARE_
                                  &&
                                  !std::is_floating_point<value_type_or_T<T1>>::value &&
                                  !std::is_floating_point<value_type_or_T<T2>>::value
#endif
                              ,
                              int>::type _line)
      : m_ip(getIp()), m_failed(!unittest_compareHelper(a, b))
  {
    if (VIR_IS_UNLIKELY(m_failed)) {
      printFailure(a, b, _a, _b, _file, _line);
    }
  }

#if VIR_NEED_FUZZY_FLOAT_COMPARE_
  template <typename T1, typename T2,
            typename T = typename std::common_type<T1, T2>::type>
  VIR_ALWAYS_INLINE Compare(
      const T1 &a, const T2 &b, const char *_a, const char *_b, const char *_file,
      typename std::enable_if<vir::detail::has_equality_operator<T1, T2>::value &&
                                  std::is_floating_point<value_type_or_T<T>>::value,
                              int>::type _line)
      : m_ip(getIp())
      , m_failed(!all_of(detail::ulpDiffToReferenceWrapper(T(a), T(b), int()) <= 1))
  {
    if (VIR_IS_UNLIKELY(m_failed)) {
      printFirst();
      printPosition(_file, _line);
      print(_a);
      print(" (");
      print(std::setprecision(10));
      print(a);
      print(") ≈ ");
      print(_b);
      print(" (");
      print(std::setprecision(10));
      print(b);
      print(std::setprecision(6));
      print(") -> ");
      print(a == b);
      print("\ndistance: ");
      print(detail::ulpDiffToReferenceWrapper(T(a), T(b), int()));
      print(" ulp, allowed distance: ±");
      print(1);
      print(" ulp (automatic fuzzy compare to work around x87 quirks)");
    }
  }
#endif
#undef VIR_NEED_FUZZY_FLOAT_COMPARE_

  template <typename T1, typename T2>
  VIR_ALWAYS_INLINE Compare(
      const T1 &a, const T2 &b, const char *_a, const char *_b, const char *_file,
      typename std::enable_if<!vir::detail::has_equality_operator<T1, T2>::value,
                              int>::type _line)
      : Compare(a, b, _a, _b, _file, _line, Mem())
  {
  }

  // Mem Compare ctor {{{2
  template <typename T1, typename T2>
  VIR_ALWAYS_INLINE Compare(const T1 &valueA, const T2 &valueB, const char *variableNameA,
                           const char *variableNameB, const char *filename, int line, Mem)
      : m_ip(getIp()), m_failed(0 != std::memcmp(&valueA, &valueB, sizeof(T1)))
  {
    static_assert(
        sizeof(T1) == sizeof(T2),
        "MEMCOMPARE requires both of its arguments to have the same size (equal sizeof)");
    if (VIR_IS_UNLIKELY(m_failed)) {
      printFirst();
      printPosition(filename, line);
      print("MEMCOMPARE(");
      print(variableNameA);
      print(", ");
      print(variableNameB);
      const int endian_test = 1;
      if (reinterpret_cast<const char *>(&endian_test)[0] == 1) {
        print("), memory contents (little-endian):\n");
      } else {
        print("), memory contents (big-endian):\n");
      }
      printMem(valueA);
      print('\n');
      printMem(valueB);
    }
  }

  // NoEq Compare ctor {{{2
  template <typename T1, typename T2>
  VIR_ALWAYS_INLINE Compare(const T1 &a, const T2 &b, const char *_a, const char *_b,
                           const char *_file, int _line, NoEq)
      : m_ip(getIp()), m_failed(!all_of(a != b))
  {
    if (VIR_IS_UNLIKELY(m_failed)) {
      printFirst();
      printPosition(_file, _line);
      print(_a);
      print(" (");
      print(std::setprecision(10));
      print(a);
      print(") == ");
      print(_b);
      print(" (");
      print(std::setprecision(10));
      print(b);
      print(std::setprecision(6));
      print(')');
    }
  }

  // Fuzzy Compare ctor {{{2
  template <typename T>
  VIR_ALWAYS_INLINE Compare(const T &a, const T &b, const char *_a, const char *_b,
                           const char *_file, int _line, Fuzzy)
      : m_ip(getIp())
      , m_failed(!unittest_fuzzyCompareHelper(
            a, b, std::is_floating_point<value_type_or_T<T>>()))
  {
    if (VIR_IS_UNLIKELY(m_failed)) {
      printFirst();
      printPosition(_file, _line);
      print(_a);
      print(" (");
      print(std::setprecision(10));
      print(a);
      print(") ≈ ");
      print(_b);
      print(" (");
      print(std::setprecision(10));
      print(b);
      print(std::setprecision(6));
      print(") -> ");
      print(a == b);
      printFuzzyInfo(a, b);
    }
    if (global_unit_test_object_.plotFile.is_open()) {
      writePlotData(global_unit_test_object_.plotFile, a, b);
    }
  }

  // Absolute Error Compare ctor {{{2
  template <typename T, typename ET>
  VIR_ALWAYS_INLINE Compare(const T &a, const T &b, const char *_a, const char *_b,
                           const char *_file, int _line, AbsoluteError, ET error)
      : m_ip(getIp()), m_failed(absoluteErrorTest(a, b, error))
  {
    if (VIR_IS_UNLIKELY(m_failed)) {
      printFirst();
      printPosition(_file, _line);
      print(_a);
      print(" (");
      print(std::setprecision(10));
      print(a);
      print(") ≈ ");
      print(_b);
      print(" (");
      print(std::setprecision(10));
      print(b);
      print(std::setprecision(6));
      print(") -> ");
      print(a == b);
      print("\ndifference: ");
      if (a > b) {
        print(a - b);
      } else {
        print('-');
        print(b - a);
      }
      print(", allowed difference: ±");
      print(error);
      print("\ndistance: ");
      using vir::detail::ulpDiffToReferenceSigned;
      print(ulpDiffToReferenceSigned(a, b));
      print(" ulp");
    }
  }

  // Relative Error Compare ctor {{{2
  template <typename T, typename ET>
  VIR_ALWAYS_INLINE Compare(const T &a, const T &b, const char *_a, const char *_b,
                           const char *_file, int _line, RelativeError, ET error)
      : m_ip(getIp()), m_failed(relativeErrorTest(a, b, error))
  {
    if (VIR_IS_UNLIKELY(m_failed)) {
      printFirst();
      printPosition(_file, _line);
      print(_a);
      print(" (");
      print(std::setprecision(10));
      print(a);
      print(") ≈ ");
      print(_b);
      print(" (");
      print(std::setprecision(10));
      print(b);
      print(std::setprecision(6));
      print(") -> ");
      print(a == b);
      print("\nrelative difference: ");
      if (a > b) {
        print((a - b) / (b > 0 ? b : -b));
      } else {
        print('-');
        print((b - a) / (b > 0 ? b : -b));
      }
      print(", allowed: ±");
      print(error);
      print("\nabsolute difference: ");
      if (a > b) {
        print(a - b);
      } else {
        print('-');
        print(b - a);
      }
      print(", allowed: ±");
      print(error * (b > 0 ? b : -b));
      print("\ndistance: ");
      using vir::detail::ulpDiffToReferenceSigned;
      print(ulpDiffToReferenceSigned(a, b));
      print(" ulp");
    }
  }

  // VERIFY ctor {{{2
  VIR_ALWAYS_INLINE Compare(bool good, const char *cond, const char *_file, int _line)
      : m_ip(getIp()), m_failed(!good)
  {
    if (VIR_IS_UNLIKELY(m_failed)) {
      printFirst();
      printPosition(_file, _line);
      print(cond);
    }
  }

  // FAIL ctor {{{2
  VIR_ALWAYS_INLINE Compare(const char *_file, int _line) : m_ip(getIp()), m_failed(true)
  {
    printFirst();
    printPosition(_file, _line);
  }

  // stream operators {{{2
  template <typename T> VIR_ALWAYS_INLINE const Compare &operator<<(const T &x) const
  {
    if (VIR_IS_UNLIKELY(m_failed)) {
      print(x);
    }
    return *this;
  }

  VIR_ALWAYS_INLINE const Compare &operator<<(const char *str) const
  {
    if (VIR_IS_UNLIKELY(m_failed)) {
      print(str);
    }
    return *this;
  }

  VIR_ALWAYS_INLINE const Compare &operator<<(const char ch) const
  {
    if (VIR_IS_UNLIKELY(m_failed)) {
      print(ch);
    }
    return *this;
  }

  VIR_ALWAYS_INLINE const Compare &operator<<(bool b) const
  {
    if (VIR_IS_UNLIKELY(m_failed)) {
      print(b);
    }
    return *this;
  }

  VIR_ALWAYS_INLINE ~Compare() noexcept(false)
  {
    if (VIR_IS_UNLIKELY(m_failed)) {
      printLast();
    }
  }

  // }}}2
private:
  static VIR_ALWAYS_INLINE size_t getIp()  //{{{2
  {
    size_t _ip;
#ifdef __GNUC__
#ifdef __x86_64__
    asm volatile("lea 0(%%rip),%0" : "=r"(_ip));
#elif defined __i386__
    asm volatile("1: movl $1b,%0" : "=r"(_ip));
#elif defined __arm__
    asm volatile("mov %0,pc" : "=r"(_ip));
#else
    _ip = 0;
#endif
#else   //__GNUC__
    _ip = 0;
#endif  //__GNUC__
    return _ip;
  }

  static char hexChar(char x) { return x + (x > 9 ? 87 : 48); }
  template <typename T> static void printMem(const T &x)  // {{{2
  {
    constexpr std::size_t length = sizeof(T) * 2 + sizeof(T) / 4;
    std::array<char, length + 3> tmp;
    tmp[0] = '0';
    tmp[1] = 'x';
    char *s = &tmp[2];
    std::memset(s, '\'', length - 1);
    s[length - 1] = '\0';
    s[length] = '\0';
    const auto bytes = reinterpret_cast<const std::uint8_t *>(&x);
    for (std::size_t i = 0; i < sizeof(T); ++i) {
      s[i * 2 + i / 4] = hexChar(bytes[i] >> 4);
      s[i * 2 + 1 + i / 4] = hexChar(bytes[i] & 0xf);
    }
    std::cout << tmp.data();
  }

  // printFailure {{{2
  template <typename T1, typename T2>
  void printFailure(const T1 &a, const T2 &b, const char *_a, const char *_b,
                    const char *_file, int _line);

  // printFirst {{{2
  static void printFirst()
  {
    if (!global_unit_test_object_.vim_lines) {
      std::cout << failString() << "┍ ";
    }
  }
  // print overloads {{{2
  template <typename T, typename = decltype(std::cout << std::declval<const T &>())>
  static inline void printImpl(const T &x, int)
  {
    std::cout << x;
  }
  template <typename T> static inline void printImpl(const T &x, ...) { printMem(x); }
  template <typename T> static inline void print(const T &x) { printImpl(x, int()); }
  static void print(const std::type_info &x)
  {
#ifdef HAVE_CXX_ABI_H
    char buf[1024];
    size_t size = 1024;
    abi::__cxa_demangle(x.name(), buf, &size, nullptr);
    std::cout << buf;
#else
    std::cout << x.name();
#endif
  }
  static void print(const std::string &str) { print(str.c_str()); }
  static void print(const char *str)
  {
    const char *pos = 0;
    if (0 != (pos = std::strchr(str, '\n'))) {
      if (pos == str) {
        std::cout << '\n' << failString();
        if (!global_unit_test_object_.vim_lines) {
          std::cout << "│ ";
        }
        print(&str[1]);
      } else {
        const std::string left(str, pos - str);
        std::cout << left << '\n' << failString();
        if (!global_unit_test_object_.vim_lines) {
          std::cout << "│ ";
        }
        print(&pos[1]);
      }
    } else {
      std::cout << str;
    }
  }
  static void print(const unsigned char ch) { std::cout << int(ch); }
  static void print(const signed char ch) { std::cout << int(ch); }
  static void print(const char ch)
  {
    if (ch == '\n') {
      std::cout << '\n' << failString();
      if (!global_unit_test_object_.vim_lines) {
        std::cout << "│ ";
      }
    } else {
      std::cout << ch;
    }
  }
  static void print(bool b) { std::cout << (b ? "true" : "false"); }
  // printLast {{{2
  static void printLast()
  {
    std::cout << std::endl;
    global_unit_test_object_.status = false;
    throw UnitTestFailure();
  }
  // printPosition {{{2
  void printPosition(const char *_file, int _line)
  {
    if (global_unit_test_object_.vim_lines) {
      std::cout << _file << ':' << _line << ": (0x" << std::hex << m_ip << std::dec
                << "): ";
    } else {
      std::cout << "at " << _file << ':' << _line << " (0x" << std::hex << m_ip
                << std::dec << ')';
      print("):\n");
    }
  }
  template <typename T> static inline void writePlotData(std::fstream &file, T a, T b);
  template <typename T> static inline void printFuzzyInfo(T, T);
  template <typename T>
  static inline void printFuzzyInfoImpl(std::true_type, T a, T b, double fuzzyness)
  {
    print("\ndistance: ");
    using vir::detail::ulpDiffToReferenceSigned;
    print(ulpDiffToReferenceSigned(a, b));
    print(" ulp, allowed distance: ±");
    print(fuzzyness);
    print(" ulp");
  }
  template <typename T>
  static inline void printFuzzyInfoImpl(std::false_type, T, T, double)
  {
  }
  // member variables {{{2
  const size_t m_ip;
  const bool m_failed;
};

  // printFailure {{{2
template <typename T1, typename T2>
VIR_NEVER_INLINE void Compare::printFailure(const T1 &a, const T2 &b, const char *_a,
                                            const char *_b, const char *_file, int _line)
{
  printFirst();
  printPosition(_file, _line);
  print(_a);
  print(" (");
  print(std::setprecision(10));
  print(a);
  print(") == ");
  print(_b);
  print(" (");
  print(std::setprecision(10));
  print(b);
  print(std::setprecision(6));
  print(") -> ");
  print(a == b);
}

// PrintMemDecorator{{{1
template <typename T> struct PrintMemDecorator {
  T x;
};

// printFuzzyInfo specializations for float and double {{{1
template <typename T> inline void Compare::printFuzzyInfo(T a, T b)
{
  using U = value_type_or_T<T>;
  printFuzzyInfoImpl(std::is_floating_point<U>(), a, b,
                     global_unit_test_object_.fuzzyness<U>());
}
template <typename T>
static inline void writePlotDataImpl(std::true_type, std::fstream &file, T ref, T dist)
{
  for (size_t i = 0; i < T::size(); ++i) {
    file << std::setprecision(12) << ref[i] << "\t" << dist[i] << "\n";
  }
}
template <typename T>
static inline void writePlotDataImpl(std::false_type, std::fstream &file, T ref, T dist)
{
  file << std::setprecision(12) << ref << "\t" << dist << "\n";
}
template <typename T> inline void Compare::writePlotData(std::fstream &file, T a, T b)
{
  const T ref = b;
  using vir::detail::ulpDiffToReferenceSigned;
  const T dist = ulpDiffToReferenceSigned(a, b);
  writePlotDataImpl(Vc::is_simd<T>(), file, ref, dist);
}

// assert_impl (called from assert macro) {{{1
struct assert_impl {
  VIR_ALWAYS_INLINE assert_impl(bool ok, const char *code, const char *file,
                                int line)
  {
    if (VIR_IS_UNLIKELY(global_unit_test_object_.expect_assert_failure)) {
      if (ok) {
        out_ptr = new (&compare_storage) Compare(file, line);
        *out_ptr << "assert(" << code << ") should have failed.";
      }
    } else if (VIR_IS_UNLIKELY(!ok)) {
      out_ptr = new (&compare_storage) Compare(file, line);
      *out_ptr << "assert(" << code << ") failed.";
    }
  }
  VIR_ALWAYS_INLINE ~assert_impl() noexcept(false)
  {
    if (VIR_IS_UNLIKELY(out_ptr != nullptr)) {
      finalize();
    }
  }
  template <class T> VIR_ALWAYS_INLINE assert_impl &operator<<(T &&x)
  {
    if (VIR_IS_UNLIKELY(out_ptr != nullptr)) {
      print(std::forward<T>(x));
    }
    return *this;
  }

private:
  template <class T> void print(T &&x) const
  {
    *out_ptr << std::forward<T>(x);
  }
  void finalize() noexcept(false)
  {
    out_ptr->~Compare();  // throws
    out_ptr = nullptr;
  }
  typename std::aligned_storage<sizeof(Compare), alignof(Compare)>::type compare_storage;
  Compare *out_ptr = nullptr;
};

// TestData {{{1
struct TestData {
  template <class F, class S>
  TestData(F &&f_, S &&name_)
      : f(std::forward<F>(f_)), name(std::forward<S>(name_))
  {
  }
  TestFunction f;
  std::string name;
};
vector<TestData> allTests;

// class Test {{{1
template <typename TestWrapper, typename Exception = void>
struct Test : public TestWrapper {
  static void wrapper()
  {
    try {
      TestWrapper::run();
    } catch (const Exception &e) {
      return;
    }
    std::cout << failString() << "The test was expected to throw an exception of type '"
              << typeToString<Exception>() << "', but it did not throw anything." << std::endl;
    global_unit_test_object_.status = false;
    throw UnitTestFailure();
  }

  Test(std::string name) { allTests.emplace_back(wrapper, std::move(name)); }
};

template <typename TestWrapper> struct Test<TestWrapper, void> : public TestWrapper {
  Test(std::string name) { allTests.emplace_back(&TestWrapper::run, std::move(name)); }
};

// addTestInstantiations {{{1
template <template <typename> class TestWrapper, typename... Ts>
static int addTestInstantiations(const char *basename, Typelist<Ts...>)
{
  std::string name(basename);
  name += '<';
  const auto &x = {
      0, (allTests.emplace_back(&TestWrapper<Ts>::run, name + typeToString<Ts>() + '>'),
          0)...};
  [](decltype(x)) {}(x);  // silence "x is unused" warning
  return 0;
}

//}}}1
}  // namespace detail

// setFuzzyness {{{1
template <typename T> inline void setFuzzyness(T fuzz)
{
  detail::global_unit_test_object_.fuzzyness<T>() = fuzz;
}

// asBytes{{{1
template <typename T> detail::PrintMemDecorator<T> asBytes(const T &x) { return {x}; }

// FUZZY_COMPARE {{{1
// Workaround for clang: The "<< ' '" is only added to silence the warnings
// about unused return values.
#define FUZZY_COMPARE(a, b)                                                              \
  vir::test::detail::Compare(a, b, #a, #b, __FILE__, __LINE__,                           \
                             vir::test::detail::Compare::Fuzzy())                        \
      << ' '
// COMPARE_ABSOLUTE_ERROR {{{1
#define COMPARE_ABSOLUTE_ERROR(a_, b_, error_)                                           \
  vir::test::detail::Compare(a_, b_, #a_, #b_, __FILE__, __LINE__,                       \
                             vir::test::detail::Compare::AbsoluteError(), error_)        \
      << ' '
// COMPARE_RELATIVE_ERROR {{{1
#define COMPARE_RELATIVE_ERROR(a_, b_, error_)                                           \
  vir::test::detail::Compare(a_, b_, #a_, #b_, __FILE__, __LINE__,                       \
                             vir::test::detail::Compare::RelativeError(), error_)        \
      << ' '
// COMPARE {{{1
#define COMPARE(a, b) vir::test::detail::Compare(a, b, #a, #b, __FILE__, __LINE__) << ' '
// COMPARE_NOEQ {{{1
#define COMPARE_NOEQ(a, b)                                                               \
  vir::test::detail::Compare(a, b, #a, #b, __FILE__, __LINE__,                           \
                             vir::test::detail::Compare::NoEq())                         \
      << ' '
// MEMCOMPARE {{{1
#define MEMCOMPARE(a, b)                                                                 \
  vir::test::detail::Compare(a, b, #a, #b, __FILE__, __LINE__,                           \
                             vir::test::detail::Compare::Mem())                          \
      << ' '
// VERIFY {{{1
#define VERIFY(cond) vir::test::detail::Compare(cond, #cond, __FILE__, __LINE__) << ' '
// FAIL {{{1
#define FAIL() vir::test::detail::Compare(__FILE__, __LINE__) << ' '

// SKIP {{{1
class SKIP
{
  std::stringstream stream;

public:
  ~SKIP() noexcept(false) { throw detail::SkippedTest{stream.str()}; }
  template <typename T> SKIP &operator<<(T &&x)
  {
    stream << std::forward<T>(x);
    return *this;
  }
};

// ADD_PASS() << "text" {{{1
class ADD_PASS
{
public:
  ADD_PASS()
  {
    ++detail::global_unit_test_object_.passedTests;
    detail::printPass();
  }
  ~ADD_PASS() { std::cout << std::endl; }
  template <typename T> ADD_PASS &operator<<(const T &x)
  {
    std::cout << x;
    return *this;
  }
};

// EXPECT_FAILURE {{{1
void EXPECT_FAILURE() { detail::global_unit_test_object_.expect_failure = true; }

// expect_assert_failure {{{1
template <class F> inline void expect_assert_failure(F &&f)
{
  detail::global_unit_test_object_.expect_assert_failure = true;
  std::forward<F>(f)();
  detail::global_unit_test_object_.expect_assert_failure = false;
}
//}}}1
static void initTest(int argc, char **argv)  //{{{1
{
  for (int i = 1; i < argc; ++i) {
    if (0 == std::strcmp(argv[i], "--help") || 0 == std::strcmp(argv[i], "-h")) {
      std::cout << "Usage: " << argv[0] << " [-h|--help] [--only <testname>] [-v|--vim] "
                                           "[--maxdist] [--plotdist <plot.dat>]\n";
      exit(0);
    }
    if (0 == std::strcmp(argv[i], "--only") && i + 1 < argc) {
      detail::global_unit_test_object_.only_name = argv[i + 1];
    } else if (0 == std::strcmp(argv[i], "--maxdist")) {
      detail::global_unit_test_object_.findMaximumDistance = true;
    } else if (0 == std::strcmp(argv[i], "--plotdist") && i + 1 < argc) {
      detail::global_unit_test_object_.plotFile.open(argv[i + 1], std::ios_base::out);
      detail::global_unit_test_object_.plotFile << "# reference\tdistance\n";
    } else if (0 == std::strcmp(argv[i], "--vim") || 0 == std::strcmp(argv[i], "-v")) {
      detail::global_unit_test_object_.vim_lines = true;
    }
  }
}

static void runAll() //{{{1
{
  for (const auto &data : detail::allTests) {
    detail::global_unit_test_object_.runTestInt(data.f, data.name.c_str());
  }
}

static int finalize()  //{{{1
{
  return detail::global_unit_test_object_.finalize();
}

//}}}1
}  // namespace test
}  // namespace vir

// TEST_TYPES / TEST_CATCH / TEST macros {{{1
namespace Tests
{
using namespace vir::test;
}

#define REAL_TEST_TYPES(T_, name_, ...)                                                  \
  namespace Tests                                                                        \
  {                                                                                      \
  template <typename T_> struct name_##_ {                                               \
    static void run();                                                                   \
  };                                                                                     \
  static struct name_##_ctor {                                                           \
    name_##_ctor()                                                                       \
    {                                                                                    \
      using vir::Typelist;                                                               \
      using vir::concat;                                                                 \
      using vir::outer_product;                                                          \
      using list = vir::ensure_typelist_t<__VA_ARGS__>;                                  \
      vir::test::detail::addTestInstantiations<name_##_>(#name_, list{});                \
    }                                                                                    \
  } name_##_ctor_;                                                                       \
  }                                                                                      \
  template <typename T_> void Tests::name_##_<T_>::run()

#define FAKE_TEST_TYPES(V_, name_, ...)                                                  \
  namespace Tests                                                                        \
  {                                                                                      \
  template <typename V_> struct name_##_ {                                               \
    static void run();                                                                   \
  };                                                                                     \
  }                                                                                      \
  template <typename V_> void Tests::name_##_<V_>::run()

#define REAL_TEST(name_)                                                                 \
  namespace Tests                                                                        \
  {                                                                                      \
  struct name_##_ {                                                                      \
    static void run();                                                                   \
  };                                                                                     \
  vir::test::detail::Test<name_##_> test_##name_##_(#name_);                             \
  }                                                                                      \
  void Tests::name_##_::run()

#define FAKE_TEST(name_) template <typename UnitTest_T_> void name_##_()

#define REAL_TEST_CATCH(name_, exception_)                                               \
  struct Test##name_ {                                                                   \
    static void run();                                                                   \
  };                                                                                     \
  vir::test::detail::Test<Test##name_, exception_> test_##name_##_(#name_);              \
  void Test##name_::run()

#define FAKE_TEST_CATCH(name_, exception_) template <typename UnitTesT_T_> void name_()

#ifdef UNITTEST_ONLY_XTEST
#define TEST_TYPES(V_, name_, ...) FAKE_TEST_TYPES(V_, name_, __VA_ARGS__)
#define XTEST_TYPES(V_, name_, ...) REAL_TEST_TYPES(V_, name_, __VA_ARGS__)

#define TEST(name_) FAKE_TEST(name_)
#define XTEST(name_) REAL_TEST(name_)

#define TEST_CATCH(name_, exception_) FAKE_TEST_CATCH(name_, exception_)
#define XTEST_CATCH(name_, exception_) REAL_TEST_CATCH(name_, exception_)
#else
#define XTEST_TYPES(V_, name_, ...) FAKE_TEST_TYPES(V_, name_, __VA_ARGS__)
#define TEST_TYPES(V_, name_, ...) REAL_TEST_TYPES(V_, name_, __VA_ARGS__)

#define XTEST(name_) FAKE_TEST(name_)
#define TEST(name_) REAL_TEST(name_)

#define XTEST_CATCH(name_, exception_) FAKE_TEST_CATCH(name_, exception_)
#define TEST_CATCH(name_, exception_) REAL_TEST_CATCH(name_, exception_)
#endif

int
#ifdef _MSC_VER
__cdecl
#endif
main(int argc, char **argv)  //{{{1
{
  vir::test::initTest(argc, argv);
  vir::test::runAll();
  return vir::test::finalize();
}

//}}}1
#endif  // DOXYGEN
#endif  // VIR_TEST_H_
// vim: foldmethod=marker
