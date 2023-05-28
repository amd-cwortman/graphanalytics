
#include "xilinx_apps_common.hpp"
#include <initializer_list>
#include <string>
#include <algorithm>  // std::min
#include <iostream>
#include <vector>
#include <utility>  // std::move
#include <sstream>

template <typename T>
using XVector = xilinx_apps::XVector<T>;

int testCount = 0;
int errorCount = 0;

template <typename T>
std::ostream &operator<<(std::ostream &os, const XVector<T> &vec) {
    os << '{';
    for (const typename XVector<T>::value_type &e : vec)
        os << e << ' ';
    os << '}';
    return os;
}


template <typename T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &vec) {
    os << '{';
    for (const typename std::vector<T>::value_type &e : vec)
        os << e << ' ';
    os << '}';
    return os;
}


template <typename T, typename U>
bool operator==(const XVector<T> &xvec, const std::vector<U> &svec) {
    if (xvec.size() != svec.size())
        return false;
    for (unsigned i = 0, end = std::min(xvec.size(), svec.size()); i < end; ++i) {
        if (xvec[i] != svec[i])
            return false;
    }
    return true;
}


template <typename T, typename U>
bool testEqualFn(const T &a, const U &b, const std::string &context, const std::string &astr, const std::string &bstr) {
    ++testCount;
    if (a == b)
        return true;
    std::cout << "ERROR: args not equal in context \"" << context << "\". "
        << astr << " = " << a << ", " << bstr << " = " << b << std::endl;
    ++errorCount;
    return false;
}

#define testEqual(a_, b_, context_) testEqualFn(a_, b_, context_, #a_, #b_)


template <int Instance>  // template param for separating stats between XVector and std::vector
struct IntObj {
    struct Stats {
        int ctorCount_ = 0;
        int dtorCount_ = 0;
        int copyCount_ = 0;
        int moveCount_ = 0;
    };
    
    static Stats s_stats;
    
    int i_ = 0;
    bool isConstructed_ = false;  // true if ctor called but not dtor
    bool hasValue_ = false;  // true if value assigned (ctor or =), false if dtor called or value moved out

    IntObj() : isConstructed_(true), hasValue_(false) {}
    IntObj(int i) : i_(i), isConstructed_(true), hasValue_(true) { ++s_stats.ctorCount_; }
    ~IntObj() { ++s_stats.dtorCount_; isConstructed_ = false; hasValue_ = false; }
    
    IntObj(const IntObj &other) : i_(other.i_), isConstructed_(true), hasValue_(other.hasValue_)
        { ++s_stats.ctorCount_; ++s_stats.copyCount_; }
    
    IntObj(IntObj &&other) : i_(other.i_), isConstructed_(true), hasValue_(other.hasValue_)
    {
        ++s_stats.ctorCount_;
        ++s_stats.moveCount_;
        other.hasValue_ = false;
        other.i_ = 0;
    }
    
    IntObj &operator=(const IntObj &other) {
        ++s_stats.copyCount_;
        i_ = other.i_;
        hasValue_ = other.hasValue_;
        return *this;
    }
    
    IntObj &operator=(IntObj &&other) {
        ++s_stats.moveCount_;
        i_ = other.i_;
        hasValue_ = other.hasValue_;
        other.hasValue_ = false;
        other.i_ = 0;
        return *this;
    }
    
    IntObj &operator=(int i) {
        i_ = i;
        hasValue_ = true;
        return *this;
    }
    
    operator int() const { return i_; }
    
    template <int I2>
    bool operator==(const IntObj<I2> &other) const {
        return
            i_ == other.i_
            && isConstructed_ == other.isConstructed_
            && hasValue_ == other.hasValue_
            && s_stats.ctorCount_ == other.s_stats.ctorCount_
            && s_stats.dtorCount_ == other.s_stats.dtorCount_
            && s_stats.copyCount_ == other.s_stats.copyCount_
            && s_stats.moveCount_ == other.s_stats.moveCount_
        ;
    }
    
    template <int I2>
    bool operator!=(const IntObj<I2> &other) const { return !(*this == other); }
    
    std::ostream &print(std::ostream &os) const {
        os << "{i:" << i_
            << ", isConstructed:" << isConstructed_
            << ", hasValue:" << hasValue_
            << ", ctorCount:" << s_stats.ctorCount_
            << ", dtorCount:" << s_stats.dtorCount_
            << ", copyCount:" << s_stats.copyCount_
            << ", moveCount:" << s_stats.moveCount_
            << '}';
        return os;
    }
    
    friend std::ostream &operator<<(std::ostream &os, const IntObj &obj) { return obj.print(os); }
};

template <int I>
typename IntObj<I>::Stats IntObj<I>::s_stats;


template <typename T, typename U>
void runTest() {
    testEqual(XVector<T>(), std::vector<U>(), "empty XVector");
    testEqual(XVector<T>(5), std::vector<U>(5), "XVector(count)");
    testEqual(XVector<T>(5, 10), std::vector<U>(5,10), "XVector(count, value)");
    
    XVector<T> xvList({1, 2, 3});
    std::vector<U> svList({1, 2, 3});
    testEqual(xvList, svList, "XVector({list})");

    XVector<T> xvCopy(xvList);
    std::vector<U> svCopy(svList);
    testEqual(xvCopy, svCopy, "XVector(const XVector &)");

    XVector<T> xvMove(std::move(xvCopy));
    std::vector<U> svMove(std::move(svCopy));
    testEqual(xvMove, svMove, "XVector(XVector &&)");
    testEqual(xvCopy, svCopy, "original vectors after move");

    xvCopy = xvMove;
    svCopy = svMove;
    testEqual(xvCopy, svCopy, "copy-assign");
    testEqual(xvMove, svMove, "original vectors after copy-assign");
    
    XVector<T> xvMove2;
    std::vector<U> svMove2;
    xvMove2 = std::move(xvCopy);
    svMove2 = std::move(svCopy);
    testEqual(xvMove2, svMove2, "move-assign");
    testEqual(xvCopy, svCopy, "original vectors after move-assign");
    
    testEqual(xvList[0], svList[0], "v[0]");
    testEqual(xvList[1], svList[1], "v[1]");
    testEqual(xvList[2], svList[2], "v[2]");
    xvList[2] = 10;
    svList[2] = 10;
    testEqual(xvList[2], svList[2], "after v[2] = 10");
    
    auto xvIt = xvList.begin();
    auto svIt = svList.begin();
    int count = 0;
    for (auto endIt = xvList.end(); xvIt != endIt; ++svIt, ++xvIt) {
        std::ostringstream oss;
        *xvIt = *xvIt * 2;
        *svIt = *svIt * 2;
        oss << "iterator[" << count << "]";
        testEqual(*xvIt, *svIt, oss.str());
        ++count;
    }

    auto xvCit = xvList.cbegin();
    auto svCit = svList.cbegin();
    count = 0;
    for (auto endCit = xvList.cend(); xvCit != endCit; ++svCit, ++xvCit) {
        std::ostringstream oss;
        oss << "const iterator[" << count << "]";
        testEqual(*xvCit, *svCit, oss.str());
        ++count;
    }

    auto xvRit = xvList.rbegin();
    auto svRit = svList.rbegin();
    count = 0;
    for (auto endRit = xvList.rend(); xvRit != endRit; ++svRit, ++xvRit) {
        std::ostringstream oss;
        *xvRit = count;
        *svRit = count;
        oss << "reverse iterator[" << count << "]";
        testEqual(*xvRit, *svRit, oss.str());
        ++count;
    }

    auto xvCrit = xvList.crbegin();
    auto svCrit = svList.crbegin();
    count = 0;
    for (auto endCrit = xvList.crend(); xvCrit != endCrit; ++svCrit, ++xvCrit) {
        std::ostringstream oss;
        oss << "const reverse iterator[" << count << "]";
        testEqual(*xvCrit, *svCrit, oss.str());
        ++count;
    }

    testEqual(xvList.empty(), svList.empty(), "empty for non-empty vec");
    testEqual(XVector<T>().empty(), std::vector<T>().empty(), "empty for empty vec");
    testEqual(xvList.size(), svList.size(), "size for non-empty vec");
    testEqual(XVector<T>().size(), std::vector<T>().size(), "size for empty vec");
    
    xvList.clear();
    svList.clear();
    testEqual(xvList, svList, "after clear");
    
    for (int i = 0; i < 10; ++i) {
        xvList.push_back(i);
        svList.push_back(i);
    }
    testEqual(xvList, svList, "push_back 0 to 9");
    
    xvList.reserve(5);
    svList.reserve(5);
    testEqual(xvList, svList, "reserve 5");
    
    xvList.reserve(15);
    svList.reserve(15);
    testEqual(xvList, svList, "reserve 15");

    for (int i = 10; i < 15; ++i) {
        xvList.push_back(i);
        svList.push_back(i);
    }
    testEqual(xvList, svList, "after push_back 10 to 14");
    
    for (int i =15; i < 20; ++i) {
        xvList.push_back(i);
        svList.push_back(i);
    }
    testEqual(xvList, svList, "after push_back 15 to 19");
    
    xvList.resize(30);
    svList.resize(30);
    testEqual(xvList, svList, "after resize(30)");
    
    xvList.resize(40, 3);
    svList.resize(40, 3);
    testEqual(xvList, svList, "after resize(40, 3)");
    
    xvList.resize(20, 4);
    svList.resize(20, 4);
    testEqual(xvList, svList, "after resize(20, 4)");
    
    xvList.resize(15);
    svList.resize(15);
    testEqual(xvList, svList, "after resize(15)");
    
    XVector<T> xvPushBack;
    std::vector<U> svPushBack;
    for (int i = 0; i < 5; ++i) {
        xvPushBack.push_back(i);
        svPushBack.push_back(i);
    }
    testEqual(xvPushBack, svPushBack, "after push_back 0 to 4 on empty vector");

    for (int i = 5; i < 10; ++i) {
        xvPushBack.push_back(i);
        svPushBack.push_back(i);
    }
    testEqual(xvPushBack, svPushBack, "after push_back 5 to 9 on empty vector");
    
    for (int i = 10; i < 15; ++i) {
        xvPushBack.push_back(i);
        svPushBack.push_back(i);
    }
    testEqual(xvPushBack, svPushBack, "after push_back 10 to 14 on empty vector");
}


int main(int, char *[]) {
    runTest<int, int>();
    runTest<IntObj<1>, IntObj<2>>();
    
    std::cout << errorCount << " tests failed out of a total of " << testCount << " tests." << std::endl;
    if (errorCount > 0)
        return 1;
    return 0;
}