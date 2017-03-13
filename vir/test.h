/*  This file is part of the Vc library.

    Copyright (C) 2009-2012 Matthias Kretz <kretz@kde.org>

    Vc is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    Vc is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with Vc.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef UNITTEST_H
#define UNITTEST_H

#include <Vc/Vc>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include "detail/ulp.h"
#include <typeinfo>

#define _expand(name) #name
#define runTest(name) _unit_test_global.runTestInt(&name, _expand(name))
#define testAllTypes(name) \
    _unit_test_global.runTestInt(&name<float_v>, #name "<float_v>"); \
    _unit_test_global.runTestInt(&name<short_v>, #name "<short_v>"); \
    _unit_test_global.runTestInt(&name<sfloat_v>, #name "<sfloat_v>"); \
    _unit_test_global.runTestInt(&name<ushort_v>, #name "<ushort_v>"); \
    _unit_test_global.runTestInt(&name<int_v>, #name "<int_v>"); \
    _unit_test_global.runTestInt(&name<double_v>, #name "<double_v>"); \
    _unit_test_global.runTestInt(&name<uint_v>, #name "<uint_v>")
#define testRealTypes(name) \
    _unit_test_global.runTestInt(&name<float_v>, #name "<float_v>"); \
    _unit_test_global.runTestInt(&name<double_v>, #name "<double_v>"); \
    _unit_test_global.runTestInt(&name<sfloat_v>, #name "<sfloat_v>");

template<typename A, typename B> struct isEqualType
{
    operator bool() const { return false; }
};

template<typename T> struct isEqualType<T, T>
{
    operator bool() const { return true; }
};

static inline void printPass()
{
    std::cout << AnsiColor::green << " PASS: " << AnsiColor::normal;
}

class _UnitTest_Failure
{
};

typedef void (*testFunction)();
class _UnitTest_Global_Object
{
    public:
        _UnitTest_Global_Object()
            : status(true),
            expect_failure(false),
            assert_failure(0),
            expect_assert_failure(false),
            float_fuzzyness( 1.f ),
            double_fuzzyness( 1. ),
            only_name(0),
            m_finalized(false),
            failedTests(0), passedTests(0),
            findMaximumDistance(false),
            maximumDistance(0),
            meanDistance(0),
            meanCount(0)
        {
        }

        ~_UnitTest_Global_Object()
        {
            if (m_finalized) {
                // on windows std::exit will call the dtor again, leading to infinite recursion
                return;
            }
            if (plotFile.is_open()) {
                plotFile.flush();
                plotFile.close();
            }
            std::cout << "\n Testing done. " << passedTests << " tests passed. " << failedTests << " tests failed." << std::endl;
            m_finalized = true;
            std::exit(failedTests);
        }

        void runTestInt(testFunction fun, const char *name);

        bool status;
        bool expect_failure;
        int assert_failure;
        bool expect_assert_failure;
        float float_fuzzyness;
        double double_fuzzyness;
        const char *only_name;
        std::fstream plotFile;
    private:
        bool m_finalized;
        int failedTests;
    public:
        int passedTests;
        bool findMaximumDistance;
        double maximumDistance;
        double meanDistance;
        int meanCount;
};

static _UnitTest_Global_Object _unit_test_global;

void EXPECT_FAILURE()
{
    _unit_test_global.expect_failure = true;
}

static const char *_unittest_fail()
{
    if (_unit_test_global.expect_failure) {
        return "XFAIL: ";
    }
    static const char *str = 0;
    if (str == 0) {
        if (mayUseColor(std::cout)) {
            static const char *fail = " \033[1;40;31mFAIL:\033[0m ";
            str = fail;
        } else {
            static const char *fail = " FAIL: ";
            str = fail;
        }
    }
    return str;
}

void initTest(int argc, char **argv)
{
    for (int i = 1; i < argc; ++i) {
        if (0 == std::strcmp(argv[i], "--help") || 0 == std::strcmp(argv[i], "-h")) {
            std::cout <<
                "Usage: " << argv[0] << " [-h|--help] [--only <testname>] [--maxdist] [--plotdist <plot.dat>]\n";
            exit(0);
        }
        if (0 == std::strcmp(argv[i], "--only") && i + 1 < argc) {
            _unit_test_global.only_name = argv[i + 1];
        } else if (0 == std::strcmp(argv[i], "--maxdist")) {
            _unit_test_global.findMaximumDistance = true;
        } else if (0 == std::strcmp(argv[i], "--plotdist") && i + 1 < argc) {
            _unit_test_global.plotFile.open(argv[i + 1], std::ios_base::out);
            _unit_test_global.plotFile << "# reference\tdistance\n";
        }
    }
}

template<typename T> static inline void setFuzzyness( T );
template<> inline void setFuzzyness<float>( float fuzz ) { _unit_test_global.float_fuzzyness = fuzz; }
template<> inline void setFuzzyness<double>( double fuzz ) { _unit_test_global.double_fuzzyness = fuzz; }

void _UnitTest_Global_Object::runTestInt(testFunction fun, const char *name)
{
    if (_unit_test_global.only_name && 0 != std::strcmp(name, _unit_test_global.only_name)) {
        return;
    }
    _unit_test_global.status = true;
    _unit_test_global.expect_failure = false;
    try {
        setFuzzyness<float>(1);
        setFuzzyness<double>(1);
        maximumDistance = 0.;
        meanDistance = 0.;
        meanCount = 0;
        fun();
    } catch(_UnitTest_Failure) {
    }
    if (_unit_test_global.expect_failure) {
        if (!_unit_test_global.status) {
            std::cout << "XFAIL: " << name << std::endl;
        } else {
            std::cout << "unexpected PASS: " << name <<
                "\n    This test should have failed but didn't. Check the code!" << std::endl;
            ++failedTests;
        }
    } else {
        if (!_unit_test_global.status) {
            if (findMaximumDistance) {
                std::cout << _unittest_fail() << "│ with a maximal distance of " << maximumDistance << " to the reference (mean: " << meanDistance / meanCount << ").\n";
            }
            std::cout << _unittest_fail() << "┕ " << name << std::endl;
            ++failedTests;
        } else {
            printPass();
            std::cout << name;
            if (findMaximumDistance) {
                if (maximumDistance > 0.) {
                    std::cout << " with a maximal distance of " << maximumDistance << " to the reference (mean: " << meanDistance / meanCount << ").";
                } else {
                    std::cout << " all values matched the reference precisely.";
                }
            }
            std::cout << std::endl;
            ++passedTests;
        }
    }
}

template<typename T1, typename T2> static inline bool unittest_compareHelper( const T1 &a, const T2 &b ) { return a == b; }
template<> inline bool unittest_compareHelper<Vc::int_v, Vc::int_v>( const Vc::int_v &a, const Vc::int_v &b ) { return (a == b).isFull(); }
template<> inline bool unittest_compareHelper<Vc::uint_v, Vc::uint_v>( const Vc::uint_v &a, const Vc::uint_v &b ) { return (a == b).isFull(); }
template<> inline bool unittest_compareHelper<Vc::float_v, Vc::float_v>( const Vc::float_v &a, const Vc::float_v &b ) { return (a == b).isFull(); }
template<> inline bool unittest_compareHelper<Vc::double_v, Vc::double_v>( const Vc::double_v &a, const Vc::double_v &b ) { return (a == b).isFull(); }
template<> inline bool unittest_compareHelper<Vc::ushort_v, Vc::ushort_v>( const Vc::ushort_v &a, const Vc::ushort_v &b ) { return (a == b).isFull(); }
template<> inline bool unittest_compareHelper<Vc::short_v, Vc::short_v>( const Vc::short_v &a, const Vc::short_v &b ) { return (a == b).isFull(); }
template<> inline bool unittest_compareHelper<std::type_info, std::type_info>(const std::type_info &a, const std::type_info &b ) { return &a == &b; }

template<typename T> T ulpDiffToReferenceWrapper(T a, T b) {
    const T diff = ulpDiffToReference(a, b);
    if (VC_IS_UNLIKELY(_unit_test_global.findMaximumDistance)) {
        _unit_test_global.maximumDistance = std::max<double>(std::abs(diff), _unit_test_global.maximumDistance);
        _unit_test_global.meanDistance += std::abs(diff);
        ++_unit_test_global.meanCount;
    }
    return diff;
}
template<typename T> Vc::Vector<T> ulpDiffToReferenceWrapper(Vc::Vector<T> a, Vc::Vector<T> b) {
    const Vc::Vector<T> diff = ulpDiffToReference(a, b);
    if (VC_IS_UNLIKELY(_unit_test_global.findMaximumDistance)) {
        _unit_test_global.maximumDistance = std::max<double>(Vc::abs(diff).max(), _unit_test_global.maximumDistance);
        _unit_test_global.meanDistance += Vc::abs(diff).sum();
        _unit_test_global.meanCount += Vc::Vector<T>::Size;
    }
    return diff;
}
template<typename T> static inline bool unittest_fuzzyCompareHelper( const T &a, const T &b ) { return a == b; }
template<> inline bool unittest_fuzzyCompareHelper<float>( const float &a, const float &b ) {
    return ulpDiffToReferenceWrapper(a, b) <= _unit_test_global.float_fuzzyness;
}
template<> inline bool unittest_fuzzyCompareHelper<Vc::float_v>( const Vc::float_v &a, const Vc::float_v &b ) {
    return ulpDiffToReferenceWrapper(a, b) <= _unit_test_global.float_fuzzyness;
}
template<> inline bool unittest_fuzzyCompareHelper<Vc::sfloat_v>( const Vc::sfloat_v &a, const Vc::sfloat_v &b ) {
    return ulpDiffToReferenceWrapper(a, b) <= _unit_test_global.float_fuzzyness;
}
template<> inline bool unittest_fuzzyCompareHelper<double>( const double &a, const double &b ) {
    return ulpDiffToReferenceWrapper(a, b) <= _unit_test_global.double_fuzzyness;
}
template<> inline bool unittest_fuzzyCompareHelper<Vc::double_v>( const Vc::double_v &a, const Vc::double_v &b ) {
    return ulpDiffToReferenceWrapper(a, b) <= _unit_test_global.double_fuzzyness;
}

template<typename T1, typename T2, typename M> inline void unitttest_comparePrintHelper(const T1 &a, const T2 &b, const M &m, const char *aa, const char *bb, const char *file, int line, double fuzzyness = 0.) {
    std::cout << "       " << aa << " (" << std::setprecision(10) << a << std::setprecision(6) << ") == " << bb << " (" << std::setprecision(10) << b << std::setprecision(6) << ") -> " << m;
    if (fuzzyness > 0.) {
        std::cout << " with fuzzyness " << fuzzyness;
    }
    std::cout << " at " << file << ":" << line << " failed.\n";
}

template<typename T> inline double unittest_fuzzynessHelper(const T &) { return 0.; }
template<> inline double unittest_fuzzynessHelper<float>(const float &) { return _unit_test_global.float_fuzzyness; }
template<> inline double unittest_fuzzynessHelper<Vc::float_v>(const Vc::float_v &) { return _unit_test_global.float_fuzzyness; }
template<> inline double unittest_fuzzynessHelper<double>(const double &) { return _unit_test_global.double_fuzzyness; }
template<> inline double unittest_fuzzynessHelper<Vc::double_v>(const Vc::double_v &) { return _unit_test_global.double_fuzzyness; }

#ifdef __GNUC__
#define Vc_ALWAYS_INLINE __attribute__((__always_inline__))
#else
#define Vc_ALWAYS_INLINE
#endif

class _UnitTest_Compare
{
    public:
        enum OptionFuzzy { Fuzzy };
        enum OptionNoEq { NoEq };

        template<typename T1, typename T2>
        inline Vc_ALWAYS_INLINE _UnitTest_Compare(const T1 &a, const T2 &b, const char *_a, const char *_b, const char *_file, int _line)
            : m_ip(getIp()), m_failed(!unittest_compareHelper(a, b))
        {
            if (VC_IS_UNLIKELY(m_failed)) {
                printFirst();
                printPosition(_file, _line); print(":\n");
                print(_a); print(" ("); print(std::setprecision(10)); print(a); print(") == ");
                print(_b); print(" ("); print(std::setprecision(10)); print(b); print(std::setprecision(6));
                print(") -> "); print(a == b);
            }
        }

        template<typename T1, typename T2>
        inline Vc_ALWAYS_INLINE _UnitTest_Compare(const T1 &a, const T2 &b, const char *_a, const char *_b, const char *_file, int _line, OptionNoEq)
            : m_ip(getIp()), m_failed(!unittest_compareHelper(a, b))
        {
            if (VC_IS_UNLIKELY(m_failed)) {
                printFirst();
                printPosition(_file, _line); print(":\n");
                print(_a); print(" ("); print(std::setprecision(10)); print(a); print(") == ");
                print(_b); print(" ("); print(std::setprecision(10)); print(b); print(std::setprecision(6));
                print(')');
            }
        }

        template<typename T>
        inline Vc_ALWAYS_INLINE _UnitTest_Compare(const T &a, const T &b, const char *_a, const char *_b, const char *_file, int _line, OptionFuzzy)
            : m_ip(getIp()), m_failed(!unittest_fuzzyCompareHelper(a, b))
        {
            if (VC_IS_UNLIKELY(m_failed)) {
                printFirst();
                printPosition(_file, _line); print(":\n");
                print(_a); print(" ("); print(std::setprecision(10)); print(a); print(") ≈ ");
                print(_b); print(" ("); print(std::setprecision(10)); print(b); print(std::setprecision(6));
                print(") -> "); print(a == b);
                printFuzzyInfo(a, b);
            }
            if (_unit_test_global.plotFile.is_open()) {
                writePlotData(_unit_test_global.plotFile, a, b);
            }
        }

        inline Vc_ALWAYS_INLINE _UnitTest_Compare(bool good, const char *cond, const char *_file, int _line)
            : m_ip(getIp()), m_failed(!good)
        {
            if (VC_IS_UNLIKELY(m_failed)) {
                printFirst();
                printPosition(_file, _line);
                print(": "); print(cond);
            }
        }

        inline Vc_ALWAYS_INLINE _UnitTest_Compare(const char *_file, int _line)
            : m_ip(getIp()), m_failed(true)
        {
            if (VC_IS_UNLIKELY(m_failed)) {
                printFirst();
                printPosition(_file, _line);
                print(' ');
            }
        }

        template<typename T> inline const _UnitTest_Compare &Vc_ALWAYS_INLINE operator<<(const T &x) const {
            if (m_failed) {
                print(x);
            }
            return *this;
        }

        inline const _UnitTest_Compare &Vc_ALWAYS_INLINE operator<<(const char *str) const {
            if (m_failed) {
                print(str);
            }
            return *this;
        }

        inline const _UnitTest_Compare &Vc_ALWAYS_INLINE operator<<(const char ch) const {
            if (m_failed) {
                print(ch);
            }
            return *this;
        }

        inline const _UnitTest_Compare &Vc_ALWAYS_INLINE operator<<(bool b) const {
            if (m_failed) {
                print(b);
            }
            return *this;
        }

        inline Vc_ALWAYS_INLINE ~_UnitTest_Compare()
        {
            if (m_failed) {
                printLast();
            }
        }

    private:
        static inline size_t Vc_ALWAYS_INLINE getIp() {
            size_t _ip;
#if defined(__x86_64__) && defined(VC_GNU_ASM)
            asm("lea 0(%%rip),%0" : "=r"(_ip));
#else
            _ip = 0;
#endif
            return _ip;
        }
        static void printFirst() { std::cout << _unittest_fail() << "┍ "; }
        template<typename T> static void print(const T &x) { std::cout << x; }
        static void print(const std::type_info &x) { std::cout << x.name(); }
        static void print(const char *str) {
            const char *pos = 0;
            if (0 != (pos = std::strchr(str, '\n'))) {
                if (pos == str) {
                    std::cout << '\n' << _unittest_fail() << "│ " << &str[1];
                } else {
                    char *left = strdup(str);
                    left[pos - str] = '\0';
                    std::cout << left << '\n' << _unittest_fail() << "│ " << &pos[1];
                    free(left);
                }
            } else {
                std::cout << str;
            }
        }
        static void print(const char ch) {
            if (ch == '\n') {
                std::cout << '\n' << _unittest_fail() << "│ ";
            } else {
                std::cout << ch;
            }
        }
        static void print(bool b) {
            std::cout << (b ? "true" : "false");
        }
        static void printLast() {
            std::cout << std::endl;
            _unit_test_global.status = false;
            //if (!_unit_test_global.plotFile.is_open()) {
            throw _UnitTest_Failure();
            //}
        }
        void printPosition(const char *_file, int _line) {
            std::cout << "at " << _file << ':' << _line << " (0x" << std::hex << m_ip << std::dec << ')';
        }
        template<typename T> static inline void writePlotData(std::fstream &file, T a, T b);
        template<typename T> static inline void printFuzzyInfo(T a, T b);
        template<typename T> static inline void printFuzzyInfoImpl(T a, T b, double fuzzyness) {
            print("\ndistance: ");
            print(ulpDiffToReferenceSigned(a, b));
            print(", allowed distance: ");
            print(fuzzyness);
        }
        const size_t m_ip;
        const bool m_failed;
};
template<typename T> inline void _UnitTest_Compare::printFuzzyInfo(T, T) {}
template<> inline void _UnitTest_Compare::printFuzzyInfo(float a, float b) {
    printFuzzyInfoImpl(a, b, _unit_test_global.float_fuzzyness);
}
template<> inline void _UnitTest_Compare::printFuzzyInfo(double a, double b) {
    printFuzzyInfoImpl(a, b, _unit_test_global.double_fuzzyness);
}
template<> inline void _UnitTest_Compare::printFuzzyInfo(Vc::float_v::AsArg a, Vc::float_v::AsArg b) {
    printFuzzyInfoImpl(a, b, _unit_test_global.float_fuzzyness);
}
template<> inline void _UnitTest_Compare::printFuzzyInfo(Vc::double_v::AsArg a, Vc::double_v::AsArg b) {
    printFuzzyInfoImpl(a, b, _unit_test_global.double_fuzzyness);
}
template<> inline void _UnitTest_Compare::printFuzzyInfo(Vc::sfloat_v::AsArg a, Vc::sfloat_v::AsArg b) {
    printFuzzyInfoImpl(a, b, _unit_test_global.float_fuzzyness);
}
template<typename T> inline void _UnitTest_Compare::writePlotData(std::fstream &, T, T) {}
template<> inline void _UnitTest_Compare::writePlotData(std::fstream &file, float a, float b) {
    file << std::setprecision(12) << b << "\t" << ulpDiffToReferenceSigned(a, b) << "\n";
}
template<> inline void _UnitTest_Compare::writePlotData(std::fstream &file, double a, double b) {
    file << std::setprecision(12) << b << "\t" << ulpDiffToReferenceSigned(a, b) << "\n";
}
template<> inline void _UnitTest_Compare::writePlotData(std::fstream &file, Vc::float_v::AsArg a, Vc::float_v::AsArg b) {
    const Vc::float_v ref = b;
    const Vc::float_v dist = ulpDiffToReferenceSigned(a, b);
    for (int i = 0; i < Vc::float_v::Size; ++i) {
        file << std::setprecision(12) << ref[i] << "\t" << dist[i] << "\n";
    }
}
template<> inline void _UnitTest_Compare::writePlotData(std::fstream &file, Vc::double_v::AsArg a, Vc::double_v::AsArg b) {
    const Vc::double_v ref = b;
    const Vc::double_v dist = ulpDiffToReferenceSigned(a, b);
    for (int i = 0; i < Vc::double_v::Size; ++i) {
        file << std::setprecision(12) << ref[i] << "\t" << dist[i] << "\n";
    }
}
template<> inline void _UnitTest_Compare::writePlotData(std::fstream &file, Vc::sfloat_v::AsArg a, Vc::sfloat_v::AsArg b) {
    const Vc::sfloat_v ref = b;
    const Vc::sfloat_v dist = ulpDiffToReferenceSigned(a, b);
    for (int i = 0; i < Vc::sfloat_v::Size; ++i) {
        file << std::setprecision(12) << ref[i] << "\t" << dist[i] << "\n";
    }
}
#undef Vc_ALWAYS_INLINE

// Workaround for clang: The "<< ' '" is only added to silence the warnings about unused return
// values.
#define FUZZY_COMPARE( a, b ) \
    _UnitTest_Compare(a, b, #a, #b, __FILE__, __LINE__, _UnitTest_Compare::Fuzzy) << ' '

#define COMPARE( a, b ) \
    _UnitTest_Compare(a, b, #a, #b, __FILE__, __LINE__) << ' '

#define COMPARE_NOEQ( a, b ) \
    _UnitTest_Compare(a, b, #a, #b, __FILE__, __LINE__, _UnitTest_Compare::NoEq) << ' '

#define VERIFY(cond) \
    _UnitTest_Compare(cond, #cond, __FILE__, __LINE__) << ' '

#define FAIL() \
    _UnitTest_Compare(__FILE__, __LINE__) << ' '

class ADD_PASS
{
    public:
        ADD_PASS() { ++_unit_test_global.passedTests; printPass(); }
        ~ADD_PASS() { std::cout << std::endl; }
        template<typename T> ADD_PASS &operator<<(const T &x) { std::cout << x; return *this; }
};

static void unittest_assert(bool cond, const char *code, const char *file, int line)
{
    if (!cond) {
        if (_unit_test_global.expect_assert_failure) {
            ++_unit_test_global.assert_failure;
        } else {
            std::cout << "       " << code << " at " << file << ":" << line << " failed.\n";
            std::abort();
        }
    }
}
#ifdef assert
#undef assert
#endif
#define assert(cond) unittest_assert(cond, #cond, __FILE__, __LINE__)

#define EXPECT_ASSERT_FAILURE(code) \
    _unit_test_global.expect_assert_failure = true; \
    _unit_test_global.assert_failure = 0; \
    code; \
    if (_unit_test_global.assert_failure == 0) { \
        /* failure expected but it didn't fail */ \
        std::cout << "       " << #code << " at " << __FILE__ << ":" << __LINE__ << \
            " did not fail as was expected.\n"; \
        _unit_test_global.status = false; \
        throw _UnitTest_Failure(); \
        return; \
    } \
    _unit_test_global.expect_assert_failure = false

template<typename Vec> static typename Vec::Mask allMasks(int i)
{
    typedef typename Vec::IndexType I;
    typedef typename Vec::Mask M;

    if (i == 0) {
        return M(true);
    }
    --i;
    if (i < Vec::Size) {
        return M (I(Vc::IndexesFromZero) == i);
    }
    i -= Vec::Size;
    if (Vec::Size < 3) {
        return M(false);
    }
    for (int a = 0; a < Vec::Size - 1; ++a) {
        for (int b = a + 1; b < Vec::Size; ++b) {
            if (i == 0) {
                I indexes(Vc::IndexesFromZero);
                return M(indexes == a || indexes == b);
            }
            --i;
        }
    }
    if (Vec::Size < 4) {
        return M(false);
    }
    for (int a = 0; a < Vec::Size - 1; ++a) {
        for (int b = a + 1; b < Vec::Size; ++b) {
            for (int c = b + 1; c < Vec::Size; ++c) {
                if (i == 0) {
                    I indexes(Vc::IndexesFromZero);
                    return M(indexes == a || indexes == b || indexes == c);
                }
                --i;
            }
        }
    }
    if (Vec::Size < 5) {
        return M(false);
    }
    for (int a = 0; a < Vec::Size - 1; ++a) {
        for (int b = a + 1; b < Vec::Size; ++b) {
            for (int c = b + 1; c < Vec::Size; ++c) {
                for (int d = c + 1; d < Vec::Size; ++d) {
                    if (i == 0) {
                        I indexes(Vc::IndexesFromZero);
                        return M(indexes == a || indexes == b || indexes == c || indexes == d);
                    }
                    --i;
                }
            }
        }
    }
    if (Vec::Size < 6) {
        return M(false);
    }
    for (int a = 0; a < Vec::Size - 1; ++a) {
        for (int b = a + 1; b < Vec::Size; ++b) {
            for (int c = b + 1; c < Vec::Size; ++c) {
                for (int d = c + 1; d < Vec::Size; ++d) {
                    for (int e = d + 1; e < Vec::Size; ++e) {
                        if (i == 0) {
                            I indexes(Vc::IndexesFromZero);
                            return M(indexes == a || indexes == b || indexes == c || indexes == d || indexes == e);
                        }
                        --i;
                    }
                }
            }
        }
    }
    if (Vec::Size < 7) {
        return M(false);
    }
    for (int a = 0; a < Vec::Size - 1; ++a) {
        for (int b = a + 1; b < Vec::Size; ++b) {
            for (int c = b + 1; c < Vec::Size; ++c) {
                for (int d = c + 1; d < Vec::Size; ++d) {
                    for (int e = d + 1; e < Vec::Size; ++e) {
                        for (int f = e + 1; f < Vec::Size; ++f) {
                            if (i == 0) {
                                I indexes(Vc::IndexesFromZero);
                                return M(indexes == a || indexes == b || indexes == c || indexes == d || indexes == e || indexes == f);
                            }
                            --i;
                        }
                    }
                }
            }
        }
    }
    if (Vec::Size < 8) {
        return M(false);
    }
    for (int a = 0; a < Vec::Size - 1; ++a) {
        for (int b = a + 1; b < Vec::Size; ++b) {
            for (int c = b + 1; c < Vec::Size; ++c) {
                for (int d = c + 1; d < Vec::Size; ++d) {
                    for (int e = d + 1; e < Vec::Size; ++e) {
                        for (int f = e + 1; f < Vec::Size; ++f) {
                            for (int g = f + 1; g < Vec::Size; ++g) {
                                if (i == 0) {
                                    I indexes(Vc::IndexesFromZero);
                                    return M(indexes == a || indexes == b || indexes == c || indexes == d
                                            || indexes == e || indexes == f || indexes == g);
                                }
                                --i;
                            }
                        }
                    }
                }
            }
        }
    }
    return M(false);
}

#define for_all_masks(VecType, _mask_) \
    for (int _Vc_for_all_masks_i = 0; _Vc_for_all_masks_i == 0; ++_Vc_for_all_masks_i) \
        for (typename VecType::Mask _mask_ = allMasks<VecType>(_Vc_for_all_masks_i++); !_mask_.isEmpty(); _mask_ = allMasks<VecType>(_Vc_for_all_masks_i++))

#endif // UNITTEST_H
