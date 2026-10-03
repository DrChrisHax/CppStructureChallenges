// Created by Chris Manlove

#pragma once

#include <array>
#include <cstddef>
#include <type_traits>

namespace solutions::span {

inline constexpr size_t DynamicLength = static_cast<size_t>(-1);

template <typename T, size_t Length>
inline constexpr size_t BytesLength = Length == DynamicLength ? DynamicLength : Length * sizeof(T);

template <size_t Length>
struct LengthStorage {
    constexpr LengthStorage(size_t) noexcept {}

    constexpr size_t Size() const noexcept {
        return Length;
    }
};

template <>
struct LengthStorage<DynamicLength> {
    constexpr LengthStorage(size_t length) noexcept
        : Length(length)
    {}

    constexpr size_t Size() const noexcept {
        return Length;
    }

    size_t Length;
};

template <typename T, size_t Length = DynamicLength>
struct Span {
    using ElementType = T;
    using ValueType = std::remove_cv_t<T>;
    using SizeType = size_t;
    using Iterator = T*;

    constexpr explicit(Length != DynamicLength) Span(T* ptr, SizeType length)
        : Pointer(ptr)
        , StoredLength(length)
    {}

    template <size_t N>
        requires(Length == DynamicLength || Length == N)
    constexpr Span(std::type_identity_t<T> (&arr)[N]) noexcept
        : Pointer(arr)
        , StoredLength(N)
    {}

    template <typename U, size_t N>
        requires(Length == DynamicLength || Length == N) && std::is_convertible_v<U (*)[], T (*)[]>
    constexpr Span(std::array<U, N>& arr) noexcept
        : Pointer(arr.data())
        , StoredLength(N)
    {}

    template <typename U, size_t OtherLength>
        requires(Length == DynamicLength || OtherLength == DynamicLength || Length == OtherLength) &&
                std::is_convertible_v<U (*)[], T (*)[]>
    constexpr explicit(Length != DynamicLength && OtherLength == DynamicLength)
        Span(const Span<U, OtherLength>& other) noexcept
        : Pointer(other.Data())
        , StoredLength(other.Size())
    {}

    constexpr T& operator[](SizeType index) const {
        return Pointer[index];
    }

    constexpr T* Data() const noexcept {
        return Pointer;
    }

    constexpr SizeType Size() const noexcept {
        return StoredLength.Size();
    }

    constexpr SizeType SizeBytes() const noexcept {
        return StoredLength.Size() * sizeof(T);
    }

    constexpr Iterator begin() const noexcept {
        return Pointer;
    }

    constexpr Iterator end() const noexcept {
        return Pointer + StoredLength.Size();
    }

    constexpr Span<T> First(SizeType length) const {
        return Span<T>(Pointer, length);
    }

    template <size_t N>
    constexpr Span<T, N> First() const {
        return Span<T, N>(Pointer, N);
    }

    constexpr Span<T> Last(SizeType length) const {
        return Span<T>(Pointer + (Size() - length), length);
    }

    constexpr Span<T> Subspan(SizeType offset, SizeType length = DynamicLength) const {
        return Span<T>(Pointer + offset, length == DynamicLength ? Size() - offset : length);
    }

    T* Pointer;
    [[no_unique_address]] LengthStorage<Length> StoredLength;
};

template <typename T, size_t N>
Span(T (&)[N]) -> Span<T, N>;

template <typename T, size_t N>
Span(std::array<T, N>&) -> Span<T, N>;

template <typename T, size_t Length>
Span<const std::byte, BytesLength<T, Length>> AsBytes(Span<T, Length> s) noexcept {
    return Span<const std::byte, BytesLength<T, Length>>(reinterpret_cast<const std::byte*>(s.Data()), s.SizeBytes());
}

template <typename T, size_t Length>
    requires(!std::is_const_v<T>)
Span<std::byte, BytesLength<T, Length>> AsWritableBytes(Span<T, Length> s) noexcept {
    return Span<std::byte, BytesLength<T, Length>>(reinterpret_cast<std::byte*>(s.Data()), s.SizeBytes());
}

}  // namespace solutions::span
