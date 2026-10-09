// Created by Chris Manlove

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <type_traits>
#include <vector>

#include "0_fundamentals/6_iterators_tests.hpp"
#include "0_fundamentals/6_iterators_challenge.hpp"
#include "0_fundamentals/6_iterators_solution.hpp"

namespace tests::iterators {

// Every test that uses an operator is guarded by a concept check, so an unfinished ArrayIterator is a failed test
// instead of a compile error. The "Concept" test checks one requirement at a time to point at the missing piece.
constexpr const char* NOT_RANDOM_ACCESS = "Iterator is not a std::random_access_iterator yet (see \"Concept\")";
constexpr const char* NOT_CONST_RANDOM_ACCESS = "ConstIterator is not a std::random_access_iterator yet";

struct Point {
    int32_t X = 0;
    int32_t Y = 0;
};

struct User {
    template <typename T, size_t N>
    using Array = challenges::iterators::Array<T, N>;

    template <typename T>
    using Iterator = challenges::iterators::ArrayIterator<T, false>;

    template <typename T>
    using ConstIterator = challenges::iterators::ArrayIterator<T, true>;

    template <typename It>
    static void Advance(It& it, std::iter_difference_t<It> n) {
        challenges::iterators::Advance(it, n);
    }

    template <typename It>
    static std::iter_difference_t<It> Distance(It first, It last) {
        return challenges::iterators::Distance(first, last);
    }
};

struct Solution {
    template <typename T, size_t N>
    using Array = solutions::iterators::Array<T, N>;

    template <typename T>
    using Iterator = solutions::iterators::ArrayIterator<T, false>;

    template <typename T>
    using ConstIterator = solutions::iterators::ArrayIterator<T, true>;

    template <typename It>
    static void Advance(It& it, std::iter_difference_t<It> n) {
        solutions::iterators::Advance(it, n);
    }

    template <typename It>
    static std::iter_difference_t<It> Distance(It first, It last) {
        return solutions::iterators::Distance(first, last);
    }
};

// std::array's iterator is contiguous (usually a plain T*), so it passes every stage.
struct Stl {
    template <typename T, size_t N>
    using Array = std::array<T, N>;

    template <typename T>
    using Iterator = typename std::array<T, 1>::iterator;

    template <typename T>
    using ConstIterator = typename std::array<T, 1>::const_iterator;

    template <typename It>
    static void Advance(It& it, std::iter_difference_t<It> n) {
        std::ranges::advance(it, n);
    }

    template <typename It>
    static std::iter_difference_t<It> Distance(It first, It last) {
        return std::ranges::distance(first, last);
    }
};

template <typename Impl, typename T, size_t N>
using ArrayOf = typename Impl::template Array<T, N>;

template <typename Impl, typename T>
using IteratorOf = typename Impl::template Iterator<T>;

template <typename Impl, typename T>
using ConstIteratorOf = typename Impl::template ConstIterator<T>;

template <typename It>
concept HasMemberTypes = requires {
    typename std::iterator_traits<It>::value_type;
    typename std::iterator_traits<It>::difference_type;
    typename std::iterator_traits<It>::reference;
    typename std::iterator_traits<It>::pointer;
    typename std::iterator_traits<It>::iterator_category;
};

template <typename It>
concept HasValueAndDifferenceTypes = requires {
    typename std::iter_value_t<It>;
    typename std::iter_difference_t<It>;
};

template <typename It>
concept CanDecrement = requires(It it) {
    { --it } -> std::same_as<It&>;
    { it-- } -> std::same_as<It>;
};

template <typename It>
concept CanJump = requires(It it, const It constIt, std::iter_difference_t<It> n) {
    { it += n } -> std::same_as<It&>;
    { it -= n } -> std::same_as<It&>;
    { constIt + n } -> std::same_as<It>;
    { n + constIt } -> std::same_as<It>;
    { constIt - n } -> std::same_as<It>;
};

template <typename It>
concept CanIndex = requires(const It constIt, std::iter_difference_t<It> n) {
    { constIt[n] } -> std::same_as<std::iter_reference_t<It>>;
};

template <typename It, typename ConstIt>
concept MixedComparable = requires(It it, ConstIt constIt) {
    { it == constIt } -> std::convertible_to<bool>;
    { constIt == it } -> std::convertible_to<bool>;
};

// Counts how often each operator runs, so the Advance and Distance tests can tell a loop from a jump.
struct Counts {
    static void Reset() {
        Increments = 0;
        Decrements = 0;
        Jumps = 0;
        Subtractions = 0;
    }

    inline static int32_t Increments = 0;
    inline static int32_t Decrements = 0;
    inline static int32_t Jumps = 0;
    inline static int32_t Subtractions = 0;
};

class CountingIterator {
public:
    using iterator_concept = std::random_access_iterator_tag;
    using iterator_category = std::random_access_iterator_tag;
    using value_type = ptrdiff_t;
    using difference_type = ptrdiff_t;
    using reference = ptrdiff_t;
    using pointer = void;

    CountingIterator() = default;

    explicit CountingIterator(ptrdiff_t position)
        : position_(position)
    {}

    reference operator*() const {
        return position_;
    }

    CountingIterator& operator++() {
        ++Counts::Increments;
        ++position_;
        return *this;
    }

    CountingIterator operator++(int) {
        CountingIterator old = *this;
        ++*this;
        return old;
    }

    CountingIterator& operator--() {
        ++Counts::Decrements;
        --position_;
        return *this;
    }

    CountingIterator operator--(int) {
        CountingIterator old = *this;
        --*this;
        return old;
    }

    CountingIterator& operator+=(difference_type n) {
        ++Counts::Jumps;
        position_ += n;
        return *this;
    }

    CountingIterator& operator-=(difference_type n) {
        ++Counts::Jumps;
        position_ -= n;
        return *this;
    }

    CountingIterator operator+(difference_type n) const {
        CountingIterator result = *this;
        return result += n;
    }

    friend CountingIterator operator+(difference_type n, const CountingIterator& it) {
        return it + n;
    }

    CountingIterator operator-(difference_type n) const {
        CountingIterator result = *this;
        return result -= n;
    }

    difference_type operator-(const CountingIterator& other) const {
        ++Counts::Subtractions;
        return position_ - other.position_;
    }

    reference operator[](difference_type n) const {
        return position_ + n;
    }

    auto operator<=>(const CountingIterator&) const = default;

private:
    ptrdiff_t position_ = 0;
};

static_assert(std::random_access_iterator<CountingIterator>);

// An ArrayIterator with no data members yet means the challenge hasn't been started, so its tests report TODO instead
// of FAIL. Once a data member is added, missing pieces fail with a message that names them.
template <typename Impl>
void TodoUnlessStarted() {
    using It = IteratorOf<Impl, int32_t>;
    if constexpr (std::is_empty_v<It>) {
        Test::Todo();
    }
}

// The iterator

template <typename Impl>
void ForwardMemberTypes() {
    using It = IteratorOf<Impl, int32_t>;
    TodoUnlessStarted<Impl>();
    Test::Check(
        HasMemberTypes<It>,
        "Iterator needs all five member types: value_type, difference_type, reference, pointer, iterator_category");
    if constexpr (HasMemberTypes<It>) {
        using Traits = std::iterator_traits<It>;
        Test::Check(std::is_same_v<typename Traits::value_type, int32_t>, "value_type should be int32_t");
        Test::Check(std::is_same_v<typename Traits::difference_type, ptrdiff_t>, "difference_type should be ptrdiff_t");
        Test::Check(std::is_same_v<typename Traits::reference, int32_t&>, "reference should be int32_t&");
        Test::Check(std::is_same_v<typename Traits::pointer, int32_t*>, "pointer should be int32_t*");
        Test::Check(
            std::derived_from<typename Traits::iterator_category, std::forward_iterator_tag>,
            "iterator_category should be std::forward_iterator_tag or stronger");
    }
}

template <typename Impl>
void Concept() {
    using It = IteratorOf<Impl, int32_t>;
    TodoUnlessStarted<Impl>();
    Test::Check(std::default_initializable<It>, "Iterator needs a default constructor (forward iterators must have one)");
    Test::Check(
        std::copyable<It>,
        "Iterator must be copyable (if you've started stage D, see the converting constructor trap in the description)");
    Test::Check(HasValueAndDifferenceTypes<It>, "Iterator needs the member types value_type and difference_type");
    Test::Check(
        std::indirectly_readable<It>,
        "*it must work on a const Iterator: operator* should be a const member function returning reference");
    Test::Check(std::weakly_incrementable<It>, "++it must return Iterator&, and it++ must exist");
    Test::Check(std::incrementable<It>, "it++ must return a copy of the old Iterator by value");
    Test::Check(std::equality_comparable<It>, "it == other must work: add operator==");
    Test::Check(CanDecrement<It>, "--it must return Iterator&, and it-- must return a copy of the old Iterator");
    Test::Check(std::totally_ordered<It>, "it < other, <=, >, >= must work: try defaulting operator<=>");
    Test::Check(std::sized_sentinel_for<It, It>, "it2 - it1 must work and return difference_type");
    Test::Check(
        CanJump<It>,
        "it += n and it -= n must return Iterator&; it + n, n + it, and it - n must return Iterator");
    Test::Check(CanIndex<It>, "it[n] must work on a const Iterator and return reference");
    Test::Check(
        std::random_access_iterator<It>,
        "Iterator should satisfy std::random_access_iterator: iterator_concept should be std::random_access_iterator_tag");
}

template <typename Impl>
void ForwardBeginEnd() {
    using It = IteratorOf<Impl, int32_t>;
    TodoUnlessStarted<Impl>();
    if constexpr (!std::random_access_iterator<It>) {
        Test::Check(false, NOT_RANDOM_ACCESS);
    } else {
        ArrayOf<Impl, int32_t, 5> a = {1, 2, 3, 4, 5};
        It it = a.begin();
        Test::Check(*it == 1, "*begin() should be the first element");
        int32_t steps = 0;
        while (it != a.end() && steps < 5) {
            ++it;
            ++steps;
        }
        Test::Check(it == a.end(), "Five ++ from begin() should reach end() (one past the last element)");
    }
}

template <typename Impl>
void ForwardIncrement() {
    using It = IteratorOf<Impl, int32_t>;
    TodoUnlessStarted<Impl>();
    if constexpr (!std::random_access_iterator<It>) {
        Test::Check(false, NOT_RANDOM_ACCESS);
    } else {
        ArrayOf<Impl, int32_t, 5> a = {1, 2, 3, 4, 5};
        It it = a.begin();
        It& same = ++it;
        Test::Check(&same == &it, "++it should return a reference to it");
        Test::Check(*it == 2, "++it should move to the next element");
        It old = it++;
        Test::Check(*old == 2, "it++ should return the old position");
        Test::Check(*it == 3, "it++ should still move it forward");
    }
}

template <typename Impl>
void ForwardArrow() {
    using It = IteratorOf<Impl, Point>;
    TodoUnlessStarted<Impl>();
    if constexpr (!std::random_access_iterator<It>) {
        Test::Check(false, NOT_RANDOM_ACCESS);
    } else {
        ArrayOf<Impl, Point, 2> a = {Point{1, 2}, Point{3, 4}};
        It it = a.begin();
        Test::Check(it->X == 1 && it->Y == 2, "it->X should reach the first element's members");
        ++it;
        it->X = 30;
        Test::Check((*it).X == 30, "Writing it->X should change the element");
    }
}

template <typename Impl>
void ForwardWrite() {
    using It = IteratorOf<Impl, int32_t>;
    TodoUnlessStarted<Impl>();
    if constexpr (!std::random_access_iterator<It>) {
        Test::Check(false, NOT_RANDOM_ACCESS);
    } else {
        ArrayOf<Impl, int32_t, 3> a = {1, 2, 3};
        const It it = a.begin();
        *it = 10;
        Test::Check(*a.begin() == 10, "*it = 10 should change the element, even through a const Iterator object");
    }
}

template <typename Impl>
void ForwardRangeFor() {
    using It = IteratorOf<Impl, int32_t>;
    TodoUnlessStarted<Impl>();
    if constexpr (!std::random_access_iterator<It>) {
        Test::Check(false, NOT_RANDOM_ACCESS);
    } else {
        ArrayOf<Impl, int32_t, 4> a = {1, 2, 3, 4};
        int32_t sum = 0;
        for (int32_t value : a) {
            sum += value;
        }
        Test::Check(sum == 10, "range-for should visit every element once");
        for (int32_t& value : a) {
            value *= 2;
        }
        Test::Check(*a.begin() == 2, "range-for with int32_t& should be able to change the elements");
        Test::Check(std::ranges::forward_range<ArrayOf<Impl, int32_t, 4>>, "Array should be a std::ranges::forward_range");
    }
}

template <typename Impl>
void ForwardDefaultConstructed() {
    using It = IteratorOf<Impl, int32_t>;
    TodoUnlessStarted<Impl>();
    if constexpr (!std::random_access_iterator<It>) {
        Test::Check(false, NOT_RANDOM_ACCESS);
    } else {
        It x = It();
        It y = It();
        Test::Check(x == y, "Two default-constructed iterators should compare equal");
    }
}

template <typename Impl>
void ForwardMultiPass() {
    using It = IteratorOf<Impl, int32_t>;
    TodoUnlessStarted<Impl>();
    if constexpr (!std::random_access_iterator<It>) {
        Test::Check(false, NOT_RANDOM_ACCESS);
    } else {
        ArrayOf<Impl, int32_t, 4> a = {1, 2, 3, 4};
        It first = a.begin();
        It copy = first;
        ++first;
        ++first;
        Test::Check(*copy == 1, "Moving an iterator should not move its copies");
        Test::Check(*first == 3, "The moved iterator should be two elements ahead");
        ++copy;
        ++copy;
        Test::Check(copy == first, "Two iterators that took the same steps should compare equal");
    }
}

template <typename Impl>
void ForwardAlgorithms() {
    using It = IteratorOf<Impl, int32_t>;
    TodoUnlessStarted<Impl>();
    if constexpr (!std::random_access_iterator<It>) {
        Test::Check(false, NOT_RANDOM_ACCESS);
    } else {
        ArrayOf<Impl, int32_t, 5> a = {1, 2, 3, 4, 5};
        It found = std::ranges::find(a, 3);
        Test::Check(found != a.end() && *found == 3, "std::ranges::find(a, 3) should find 3");
        Test::Check(std::ranges::find(a, 9) == a.end(), "std::ranges::find(a, 9) should return end()");
        It legacy = std::find(a.begin(), a.end(), 4);
        Test::Check(
            legacy != a.end() && *legacy == 4,
            "std::find(begin, end, 4) should find 4 (older algorithms read iterator_category)");
        int64_t evens = std::ranges::count_if(a, [](int32_t x) { return x % 2 == 0; });
        Test::Check(evens == 2, "std::ranges::count_if should count 2 even elements");
    }
}

template <typename Impl>
void BidirectionalDecrement() {
    using It = IteratorOf<Impl, int32_t>;
    TodoUnlessStarted<Impl>();
    if constexpr (!std::random_access_iterator<It>) {
        Test::Check(false, NOT_RANDOM_ACCESS);
    } else {
        ArrayOf<Impl, int32_t, 5> a = {1, 2, 3, 4, 5};
        It it = a.end();
        --it;
        Test::Check(*it == 5, "--end() should be the last element");
        It& same = --it;
        Test::Check(&same == &it, "--it should return a reference to it");
        Test::Check(*it == 4, "--it should move to the previous element");
        It old = it--;
        Test::Check(*old == 4, "it-- should return the old position");
        Test::Check(*it == 3, "it-- should still move it backward");
    }
}

template <typename Impl>
void BidirectionalAlgorithms() {
    using It = IteratorOf<Impl, int32_t>;
    TodoUnlessStarted<Impl>();
    if constexpr (!std::random_access_iterator<It>) {
        Test::Check(false, NOT_RANDOM_ACCESS);
    } else {
        ArrayOf<Impl, int32_t, 5> a = {1, 2, 3, 4, 5};
        std::vector<int32_t> backward(std::make_reverse_iterator(a.end()), std::make_reverse_iterator(a.begin()));
        Test::Check(
            backward == std::vector<int32_t>{5, 4, 3, 2, 1},
            "std::reverse_iterator over your iterator should walk the array backward");
        std::ranges::reverse(a);
        Test::Check(
            std::ranges::equal(a, std::array<int32_t, 5>{5, 4, 3, 2, 1}),
            "std::ranges::reverse(a) should reverse the array");
        Test::Check(
            std::ranges::bidirectional_range<ArrayOf<Impl, int32_t, 5>>,
            "Array should be a std::ranges::bidirectional_range");
    }
}

template <typename Impl>
void RandomAccessJumps() {
    using It = IteratorOf<Impl, int32_t>;
    TodoUnlessStarted<Impl>();
    if constexpr (!std::random_access_iterator<It>) {
        Test::Check(false, NOT_RANDOM_ACCESS);
    } else {
        ArrayOf<Impl, int32_t, 5> a = {1, 2, 3, 4, 5};
        It it = a.begin();
        It& same = (it += 3);
        Test::Check(&same == &it, "it += n should return a reference to it");
        Test::Check(*it == 4, "begin() += 3 should reach the fourth element");
        it -= 2;
        Test::Check(*it == 2, "it -= 2 should move back two elements");
        it += -1;
        Test::Check(*it == 1, "it += -1 should move back one element");
        Test::Check(*(it + 4) == 5, "it + 4 should be four elements ahead");
        Test::Check(*(4 + it) == 5, "4 + it should be the same as it + 4");
        Test::Check(*it == 1, "it + n should not move it");
        Test::Check(*(a.end() - 1) == 5, "end() - 1 should be the last element");
    }
}

template <typename Impl>
void RandomAccessDifference() {
    using It = IteratorOf<Impl, int32_t>;
    TodoUnlessStarted<Impl>();
    if constexpr (!std::random_access_iterator<It>) {
        Test::Check(false, NOT_RANDOM_ACCESS);
    } else {
        ArrayOf<Impl, int32_t, 5> a = {1, 2, 3, 4, 5};
        Test::Check(a.end() - a.begin() == 5, "end() - begin() should be the number of elements");
        Test::Check(a.begin() - a.end() == -5, "begin() - end() should be negative");
        Test::Check(
            std::is_same_v<decltype(a.end() - a.begin()), ptrdiff_t>,
            "it2 - it1 should return difference_type (ptrdiff_t)");
    }
}

template <typename Impl>
void RandomAccessIndexAndOrder() {
    using It = IteratorOf<Impl, int32_t>;
    TodoUnlessStarted<Impl>();
    if constexpr (!std::random_access_iterator<It>) {
        Test::Check(false, NOT_RANDOM_ACCESS);
    } else {
        ArrayOf<Impl, int32_t, 5> a = {1, 2, 3, 4, 5};
        It it = a.begin();
        Test::Check(it[0] == 1 && it[4] == 5, "it[n] should be the element n steps ahead");
        it[1] = 20;
        Test::Check(*(it + 1) == 20, "Writing it[1] should change the element");
        It later = it + 2;
        Test::Check(it < later && it <= later && later > it && later >= it, "begin() should be less than begin() + 2");
        Test::Check(!(later < it) && it <= it && it >= it, "<, <=, >= should agree with positions");
    }
}

template <typename Impl>
void RandomAccessAlgorithms() {
    using It = IteratorOf<Impl, int32_t>;
    TodoUnlessStarted<Impl>();
    if constexpr (!std::random_access_iterator<It>) {
        Test::Check(false, NOT_RANDOM_ACCESS);
    } else {
        ArrayOf<Impl, int32_t, 5> a = {5, 3, 1, 4, 2};
        std::ranges::sort(a);
        Test::Check(std::ranges::equal(a, std::array<int32_t, 5>{1, 2, 3, 4, 5}), "std::ranges::sort(a) should sort the array");
        Test::Check(std::ranges::binary_search(a, 4), "std::ranges::binary_search(a, 4) should find 4");
        std::sort(a.begin(), a.end(), [](int32_t x, int32_t y) { return x > y; });
        Test::Check(
            std::ranges::equal(a, std::array<int32_t, 5>{5, 4, 3, 2, 1}),
            "std::sort(begin, end, greater) should sort descending (older algorithms read iterator_category)");
        Test::Check(
            std::ranges::random_access_range<ArrayOf<Impl, int32_t, 5>>,
            "Array should be a std::ranges::random_access_range");
    }
}

// Const

template <typename Impl>
void ConstMemberTypes() {
    using ConstIt = ConstIteratorOf<Impl, int32_t>;
    TodoUnlessStarted<Impl>();
    Test::Check(HasMemberTypes<ConstIt>, "ConstIterator needs all five member types");
    if constexpr (HasMemberTypes<ConstIt>) {
        using Traits = std::iterator_traits<ConstIt>;
        Test::Check(
            std::is_same_v<typename Traits::value_type, int32_t>,
            "ConstIterator's value_type should still be int32_t, without const");
        Test::Check(std::is_same_v<typename Traits::reference, const int32_t&>, "ConstIterator's reference should be const int32_t&");
        Test::Check(std::is_same_v<typename Traits::pointer, const int32_t*>, "ConstIterator's pointer should be const int32_t*");
    }
}

template <typename Impl>
void ConstReadOnly() {
    using It = IteratorOf<Impl, int32_t>;
    TodoUnlessStarted<Impl>();
    using ConstIt = ConstIteratorOf<Impl, int32_t>;
    Test::Check(std::random_access_iterator<ConstIt>, NOT_CONST_RANDOM_ACCESS);
    Test::Check(std::indirectly_writable<It, int32_t>, "*it = value should work through an Iterator");
    Test::Check(!std::indirectly_writable<ConstIt, int32_t>, "*it = value should not compile through a ConstIterator");
}

template <typename Impl>
void ConstConversion() {
    using It = IteratorOf<Impl, int32_t>;
    TodoUnlessStarted<Impl>();
    using ConstIt = ConstIteratorOf<Impl, int32_t>;
    Test::Check(std::is_convertible_v<It, ConstIt>, "Iterator should convert to ConstIterator");
    Test::Check(!std::is_convertible_v<ConstIt, It>, "ConstIterator should not convert to Iterator");
    Test::Check(!std::is_constructible_v<It, ConstIt>, "Iterator(constIterator) should not compile");
    Test::Check(MixedComparable<It, ConstIt>, "it == constIt and constIt == it should compile");
    if constexpr (std::random_access_iterator<ConstIt> && std::is_convertible_v<It, ConstIt> && MixedComparable<It, ConstIt>) {
        ArrayOf<Impl, int32_t, 3> a = {1, 2, 3};
        ConstIt it = a.begin();
        Test::Check(*it == 1, "A ConstIterator made from begin() should read the first element");
        Test::Check(a.begin() == it && it == a.begin(), "An Iterator and a ConstIterator at the same position should be equal");
        Test::Check(it != a.end(), "begin() and end() should not be equal");
    }
}

template <typename Impl>
void ConstBeginEnd() {
    using ConstIt = ConstIteratorOf<Impl, int32_t>;
    TodoUnlessStarted<Impl>();
    if constexpr (!std::random_access_iterator<ConstIt>) {
        Test::Check(false, NOT_CONST_RANDOM_ACCESS);
    } else {
        const ArrayOf<Impl, int32_t, 4> a = {1, 2, 3, 4};
        Test::Check(std::is_same_v<decltype(a.begin()), ConstIt>, "begin() on a const Array should return ConstIterator");
        Test::Check(std::is_same_v<decltype(a.end()), ConstIt>, "end() on a const Array should return ConstIterator");
        int32_t sum = 0;
        for (int32_t value : a) {
            sum += value;
        }
        Test::Check(sum == 10, "range-for over a const Array should visit every element");
        Test::Check(a.end() - a.begin() == 4, "end() - begin() on a const Array should be 4");
    }
}

// Advance and Distance

template <typename Impl>
void AdvanceJumps() {
    CountingIterator it = CountingIterator(0);
    Counts::Reset();
    Impl::Advance(it, 1000);
    Test::Check(*it == 1000, "Advance(it, 1000) should move 1000 steps");
    Impl::Advance(it, -400);
    Test::Check(*it == 600, "Advance(it, -400) should move back 400 steps");
    Test::Check(
        Counts::Increments == 0 && Counts::Decrements == 0,
        "Advance on a random access iterator should jump with +=, not loop with ++ or --");
}

template <typename Impl>
void DistanceSubtracts() {
    Counts::Reset();
    ptrdiff_t forward = Impl::Distance(CountingIterator(3), CountingIterator(1003));
    ptrdiff_t backward = Impl::Distance(CountingIterator(10), CountingIterator(4));
    Test::Check(forward == 1000, "Distance(3, 1003) should be 1000");
    Test::Check(backward == -6, "Distance(10, 4) on a random access iterator should be -6");
    Test::Check(Counts::Increments == 0, "Distance on a random access iterator should subtract, not count ++ steps");
}

template <typename Impl>
void AdvanceDistanceStdContainers() {
    std::vector<int32_t> v = {1, 2, 3, 4, 5};
    std::array<int32_t, 5> a = {1, 2, 3, 4, 5};
    auto vectorIt = v.begin();
    Impl::Advance(vectorIt, 3);
    Test::Check(*vectorIt == 4, "Advance(vector.begin(), 3) should reach 4");
    Impl::Advance(vectorIt, -2);
    Test::Check(*vectorIt == 2, "Advance(it, -2) should reach 2");
    auto arrayIt = a.end();
    Impl::Advance(arrayIt, -1);
    Test::Check(*arrayIt == 5, "Advance(array.end(), -1) should reach 5");
    Test::Check(Impl::Distance(v.begin(), v.end()) == 5, "Distance over a 5-element vector should be 5");
    Test::Check(Impl::Distance(a.end(), a.begin()) == -5, "Distance(end, begin) over a 5-element array should be -5");
}

}  // namespace tests::iterators

Test6::Test6()
    : Test(6, "iterators")
{}

void Test6::RunTests() {
    using namespace tests::iterators;

    Run("Member types", 1, ForwardMemberTypes<User>, ForwardMemberTypes<Solution>, ForwardMemberTypes<Stl>);
    Run("Concept", 1, Concept<User>, Concept<Solution>, Concept<Stl>);
    Run("begin() to end()", 1, ForwardBeginEnd<User>, ForwardBeginEnd<Solution>, ForwardBeginEnd<Stl>);
    Run("++it and it++", 1, ForwardIncrement<User>, ForwardIncrement<Solution>, ForwardIncrement<Stl>);
    Run("it->member", 1, ForwardArrow<User>, ForwardArrow<Solution>, ForwardArrow<Stl>);
    Run("Writing through *it", 1, ForwardWrite<User>, ForwardWrite<Solution>, ForwardWrite<Stl>);
    Run("Range-for", 1, ForwardRangeFor<User>, ForwardRangeFor<Solution>, ForwardRangeFor<Stl>);
    Run("Default-constructed iterators", 1, ForwardDefaultConstructed<User>, ForwardDefaultConstructed<Solution>, ForwardDefaultConstructed<Stl>);
    Run("Multi-pass", 1, ForwardMultiPass<User>, ForwardMultiPass<Solution>, ForwardMultiPass<Stl>);
    Run("find and count_if", 1, ForwardAlgorithms<User>, ForwardAlgorithms<Solution>, ForwardAlgorithms<Stl>);
    Run("--it and it--", 1, BidirectionalDecrement<User>, BidirectionalDecrement<Solution>, BidirectionalDecrement<Stl>);
    Run("reverse_iterator and reverse", 1, BidirectionalAlgorithms<User>, BidirectionalAlgorithms<Solution>, BidirectionalAlgorithms<Stl>);
    Run("+= -= + -", 1, RandomAccessJumps<User>, RandomAccessJumps<Solution>, RandomAccessJumps<Stl>);
    Run("it2 - it1", 1, RandomAccessDifference<User>, RandomAccessDifference<Solution>, RandomAccessDifference<Stl>);
    Run("it[n] and ordering", 1, RandomAccessIndexAndOrder<User>, RandomAccessIndexAndOrder<Solution>, RandomAccessIndexAndOrder<Stl>);
    Run("sort and binary_search", 1, RandomAccessAlgorithms<User>, RandomAccessAlgorithms<Solution>, RandomAccessAlgorithms<Stl>);
    Run("Const: member types", 1, ConstMemberTypes<User>, ConstMemberTypes<Solution>, ConstMemberTypes<Stl>);
    Run("Const: read-only", 1, ConstReadOnly<User>, ConstReadOnly<Solution>, ConstReadOnly<Stl>);
    Run("Const: Iterator -> ConstIterator only", 1, ConstConversion<User>, ConstConversion<Solution>, ConstConversion<Stl>);
    Run("Const: begin() and end() on a const Array", 1, ConstBeginEnd<User>, ConstBeginEnd<Solution>, ConstBeginEnd<Stl>);
    Run("Advance: jumps with +=", 1, AdvanceJumps<User>, AdvanceJumps<Solution>, AdvanceJumps<Stl>);
    Run("Distance: subtracts", 1, DistanceSubtracts<User>, DistanceSubtracts<Solution>, DistanceSubtracts<Stl>);
    Run("Advance and Distance: vector and std::array", 1, AdvanceDistanceStdContainers<User>, AdvanceDistanceStdContainers<Solution>, AdvanceDistanceStdContainers<Stl>);
}
